// LinuxProcessHandler.h
#include "../ProcessHandler.h"
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <poll.h>

void ProcessHandler_VerifyStringContained(const char *string, size_t string_bytes, const char *msg);

#include <string>
#include <sstream>


class LinuxProcessHandler : public ProcessHandler {
private:
    int in_pipe[2]={};
    int out_pipe[2]={};
    int err_pipe[2]={};
    pid_t pid={};

public:

    LinuxProcessHandler() : ProcessHandler()
    {
        shell = "/bin/sh -c ";
    }

    ~LinuxProcessHandler() override {
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(err_pipe[0]);
        waitpid(pid, NULL, 0);
    }

    void StartProcess(utf8_string_handle command,  E_PIPE pipes) override {
        command.verify_contained();

        if (pipes & E_PIPE_STDIN && pipe(in_pipe) == -1) throw std::runtime_error("Pipe creation failed");
        if (pipes & E_PIPE_STDOUT && pipe(out_pipe) == -1) throw std::runtime_error("Pipe creation failed");
        if (pipes & E_PIPE_STDERR && pipe(err_pipe) == -1) throw std::runtime_error("Pipe creation failed");

        pid = fork();
        if (pid == -1)
            throw std::runtime_error("Fork failed");

        if (pid == 0) {  // Child process
            if (pipes & E_PIPE_STDIN)
            {
                close(in_pipe[1]);
                dup2(in_pipe[0], STDIN_FILENO);
                close(in_pipe[0]);
            }

            if (pipes & E_PIPE_STDOUT)
            {
                close(out_pipe[0]);
                dup2(out_pipe[1], STDOUT_FILENO);
                close(out_pipe[1]);
            }

            if (pipes & E_PIPE_STDERR)
            {
                close(err_pipe[0]);
                dup2(err_pipe[1], STDERR_FILENO);
                close(err_pipe[1]);
            }


            execl("/bin/sh", "sh", "-c", escapeStringForCommandLine(command), (char *) 0);
            _exit(EXIT_FAILURE);
        } else {  // Parent process
            if (pipes & E_PIPE_STDIN)
            {
                close(in_pipe[0]);
            }

            if (pipes & E_PIPE_STDOUT)
            {
                close(out_pipe[1]);
            }

            if (pipes & E_PIPE_STDERR)
            {
                close(err_pipe[1]);
            }

            close(in_pipe[0]);

        }
    }

    bool HasData(E_PIPE targ) override {
        char ch;

        int pipe;

        if (targ == E_PIPE_STDOUT) pipe = out_pipe[0];
        else if (targ == E_PIPE_STDERR) pipe = err_pipe[0];
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

        if (targ == E_PIPE_STDOUT) pipe = out_pipe[0];
        else if (targ == E_PIPE_STDERR) pipe = err_pipe[0];
        else throw std::runtime_error("Invalid output stream selector");

        ssize_t bytesRead = read(pipe, &ch, 1);
        if (bytesRead <= 0) {
            END[targ] = true;
            return EOF;
        }
        return ch;
    }

    void WriteByte(char byte) override {
        if (write(in_pipe[1], &byte, 1) == -1)
            throw std::runtime_error("WriteByte failed");
    }

};

ProcessHandler *ProcessHandler_Create() {
    return new LinuxProcessHandler();
}