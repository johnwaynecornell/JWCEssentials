// LinuxCommandSpawn.h
#include <cstring>

#include "JWCCommandSpawn/CommandSpawn.h"
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <poll.h>
#include <vector>

#include "Args.h"
#include <string>
#include <sstream>


using namespace JWCEssentials;
namespace JWCCommandSpawn {
    void CommandSpawn_VerifyStringContained(const char *string, size_t string_bytes, const char *msg);

    utf8_string_struct resolveExecutablePath(const std::string &executable) {
        // Get the PATH environment variable
        const char *pathEnv = std::getenv("PATH");
        if (!pathEnv) {
            return "";
        }

        std::string pathStr(pathEnv);
        std::istringstream pathStream(pathStr);
        std::string pathDir;
        std::vector<std::string> paths;

        // Split the PATH environment variable into individual directories
        while (std::getline(pathStream, pathDir, ':')) {
            paths.push_back(pathDir);
        }

        // Search for the executable in each directory
        for (const auto &dir : paths) {
            std::string fullPath = dir + "/" + executable;
            if (access(fullPath.c_str(), X_OK) == 0) {
                return fullPath.c_str();
            }
        }

        // Executable not found
        return nullptr;
    }

    class CommandSpawn_Linux : public CommandSpawn {
    private:
        struct {
            int in_pipe[2]={};
            int out_pipe[2]={};
            int err_pipe[2]={};
            pid_t pid={};
        } state;

    public:

        CommandSpawn_Linux() : CommandSpawn()
        {
        }

        ~CommandSpawn_Linux() override {
            CommandSpawn_Linux::Join();
        }

        Shell GetShell_Defaultl() override {
            return { "/bin/sh", "-c" };
        }

        Shell GetShell_Bash() override {
            return { "/bin/bash", "-c" };
        }

        static utf8_string_struct resolve(utf8_string_struct path) {
            bool qualified = false;
            bool escape = false;

            for (int i=0; i < path.length; i++) {
                if (!escape) {
                    if (path[i] == '\\') escape = true;
                    if (path[i] == '/') qualified = true;
                } else escape = false;
            }

            if (!qualified) {
                utf8_string_struct qpath = resolveExecutablePath(path.c_str);
                return qpath;
            } else if (access(path.c_str, X_OK) == 0) {
                return path;
            }

            return nullptr;
        }

        void Close() override {
            close(state.in_pipe[1]);
            close(state.out_pipe[0]);
            close(state.err_pipe[0]);

            memset(&state, 1, sizeof(state));

        }

        long Join() override {
            if (state.pid != 0) {
                int status;

                waitpid(state.pid, &status, 0);

                int exit_status = -1;

                if ( WIFEXITED(status) )
                {
                    exit_status = WEXITSTATUS(status);
                    printf("Exit status of the child was %d\n",
                                                 exit_status);
                }

                last_return = exit_status;

                Close();
            }

            return last_return;
        }


        bool HasShell(Shell shell) override {
            return resolve(shell.shell);
        }

        bool Command(utf8_string_struct command,  E_PIPE pipes) override {
            if (state.pid != 0) {
                std::cerr << "Join must be called before reusing CommandSpawn" << std::endl;
                return false;
            }


            command.verify_contained();

            bool rc = true;

            if (pipes & E_PIPE_STDIN && pipe(state.in_pipe) == -1) throw std::runtime_error("Pipe creation failed");
            if (pipes & E_PIPE_STDOUT && pipe(state.out_pipe) == -1) throw std::runtime_error("Pipe creation failed");
            if (pipes & E_PIPE_STDERR && pipe(state.err_pipe) == -1) throw std::runtime_error("Pipe creation failed");

            utf8_string_struct command_line = ToString(command);
            utf8_string_struct path;
            char **args =execvArgs_convertTo(command_line);

            path = resolve(args[0]);

            if (!path) {
                std::cerr << "Can not find executable " << args[0];
                return false;
            }

            state.pid = fork();
            if (state.pid == -1)
                throw std::runtime_error("Fork failed");

            if (state.pid == 0) {  // Child process
                if (execv(path, args) == -1) {
                    std::cerr << "ERROR FINDc " << command_line << std::endl;
                }

                if (pipes & E_PIPE_STDIN)
                {
                    close(state.in_pipe[1]);
                    dup2(state.in_pipe[0], STDIN_FILENO);
                    close(state.in_pipe[0]);
                }

                if (pipes & E_PIPE_STDOUT)
                {
                    close(state.out_pipe[0]);
                    dup2(state.out_pipe[1], STDOUT_FILENO);
                    close(state.out_pipe[1]);
                }

                if (pipes & E_PIPE_STDERR)
                {
                    close(state.err_pipe[0]);
                    dup2(state.err_pipe[1], STDERR_FILENO);
                    close(state.err_pipe[1]);
                }

                //execl("/bin/sh", "sh", "-c", escapeStringForCommandLine(command), (char *) 0);
                _exit(EXIT_FAILURE);
            } else {  // Parent process
                execvArgs_free(args);

                if (pipes & E_PIPE_STDIN)
                {
                    close(state.in_pipe[0]);
                }

                if (pipes & E_PIPE_STDOUT)
                {
                    close(state.out_pipe[1]);
                }

                if (pipes & E_PIPE_STDERR)
                {
                    close(state.err_pipe[1]);
                }

                return rc;
            }
        }

        bool HasData(E_PIPE targ) override {
            char ch;

            int pipe;

            if (targ == E_PIPE_STDOUT) pipe = state.out_pipe[0];
            else if (targ == E_PIPE_STDERR) pipe = state.err_pipe[0];
            else  throw std::runtime_error("Invalid output stream selector");

            //file descriptor struct to check if POLLIN bit will be set
            //fd is the file descriptor of the pipe
            struct pollfd fds{ .fd = pipe, .events = POLLIN };
            //poll with no wait time
            int res = poll(&fds, 1, 0);

            //if res < 0 then an error occurred with poll
            //POLLERR is set for some other errors
            //POLLNVAL is set if the pipe is closed
            if(res < 0 || fds.revents & (POLLERR | POLLNVAL))
            {
                throw std::runtime_error("Error occured on child process " + (std::string) ((targ == E_PIPE_STDOUT) ? "STDOUT" : "STDERR"));

            }
            return fds.revents & POLLIN;
        }

        int ReadByte(E_PIPE targ) override {
            char ch;

            int pipe;

            if (targ == E_PIPE_STDOUT) pipe = state.out_pipe[0];
            else if (targ == E_PIPE_STDERR) pipe = state.err_pipe[0];
            else throw std::runtime_error("Invalid output stream selector");

            ssize_t bytesRead = read(pipe, &ch, 1);
            if (bytesRead <= 0) {
                END[targ] = true;
                return EOF;
            }
            return ch;
        }

        void WriteByte(char byte) override {
            if (write(state.in_pipe[1], &byte, 1) == -1)
                throw std::runtime_error("WriteByte failed");
        }

    };

    CommandSpawn *CommandSpawn_Create() {
        return new CommandSpawn_Linux();
    }
}