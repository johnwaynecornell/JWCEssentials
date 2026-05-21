#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstring>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

enum class LineEndingMode {
    Auto, // Use #ifdef _WIN32 behavior
    ForceLF,
    ForceCRLF
};

void process_file(LineEndingMode mode, const fs::path& path) {
    if (!fs::exists(path) || !fs::is_regular_file(path)) return;

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "Could not open file for reading: " << path << std::endl;
        return;
    }

    if (mode == LineEndingMode::Auto)
    {
        #ifdef _WIN32
            mode = LineEndingMode::ForceCRLF;
        #else
            mode = LineEndingMode::ForceLF;
        #endif
    }

    // Read the full first line to reliably catch shebangs
    std::string first_line;
    std::getline(in, first_line);

    // Quick validation: must start with '#!'
    if (first_line.size() < 2 || first_line[0] != '#' || first_line[1] != '!') {
        return;
    }

    // Reset stream state and rewind to read the whole file
    in.clear();
    in.seekg(0);
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    bool has_cr = (content.find('\r') != std::string::npos);

    if (mode == LineEndingMode::ForceCRLF)
    {
        // Windows logic: Normalize to CRLF (\r\n)
        std::string windows_content;
        windows_content.reserve(content.size() * 1.1); // Allocating a bit of padding for extra \r bytes

        for (size_t i = 0; i < content.size(); ++i) {
            // If it's already a \r\n sequence, preserve it
            if (content[i] == '\r') {
                windows_content.push_back('\r');
                continue;
            }
            // If it's a standalone \n, inject \r right before it
            if (content[i] == '\n' && (i == 0 || content[i - 1] != '\r')) {
                windows_content.push_back('\r');
            }
            windows_content.push_back(content[i]);
        }
        
        // Only write if the file actually needs to be updated to CRLF
        if (content != windows_content) {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            out.write(windows_content.data(), static_cast<std::streamsize>(windows_content.size()));
            out.close();
            std::cout << "Normalized to CRLF (Windows format): " << path << std::endl;
        }
    }  else if (mode == LineEndingMode::ForceLF)
    {
        // Linux logic: Use a clean LF (\n) stripping mechanism
        if (has_cr) {
            std::string new_content;
            new_content.reserve(content.size());
            for (char c : content) {
                if (c != '\r') new_content.push_back(c);
            }
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            out.write(new_content.data(), static_cast<std::streamsize>(new_content.size()));
            out.close();
            std::cout << "Normalized to LF (Linux format): " << path << std::endl;
        }
    }

    // Modern C++ cross-platform permission adjustment
    try {
        fs::perms current_perms = fs::status(path).permissions();
        fs::perms exec_perms = fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec;

        // Only modify if executable bits are missing
        if ((current_perms & exec_perms) != exec_perms) {
            fs::permissions(path, exec_perms, fs::perm_options::add);
            std::cout << "Marked as executable: " << path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to alter permissions: " << e.what() << std::endl;
    }
}

/**
 * Recursive search for files in a directory.
 */
void search_and_process(LineEndingMode mode, const fs::path& root) {
    if (!fs::exists(root)) {
        std::cerr << "Search path does not exist: " << root << std::endl;
        return;
    }
    if (fs::is_regular_file(root)) {
        process_file(mode, root);
        return;
    }
    if (fs::is_directory(root)) {
        try {
            for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied);
                 it != fs::recursive_directory_iterator();
                 ++it) {
                if (it->is_directory() && it->path().filename() == ".git") {
                    it.disable_recursion_pending();
                    continue;
                }
                if (it->is_regular_file()) {
                    process_file(mode, it->path());
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error during search: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: newage_shell_touch [[-search <path>]|[file]]*" << std::endl;
        std::cout << "  Files from recursive search and arguments that are identified as beginning with '#!/' are"  << std::endl;;
        std::cout << "      washed of '\\r' and on Linux. On Windows, '\\n' is normalized to '\\r\\n'. Both are given exec" << std::endl;
        std::cout << "      use -lf and -crlf To override this default behaviour" << std::endl;
        std::cout << "      This recovers from Windows line endings on Linux and Linux line endings on Windows." << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  find $NewAge/bin -name '*\\.sh'  -type f -exec newage_shell_touch {} \\;" << std::endl;
        std::cout << "  find $NewAge/bin -type f -exec newage_shell_touch {} +" << std::endl;
        std::cout << "  newage_repo_list.sh | xargs -I {} newage_shell_touch -search \"\\$NewAge/{}/Bash\"" << std::endl;
        std::cout << "  newage_shell_touch -search \"\\$NewAge/bin\"" << std::endl; // Updated example matches new constraints
        std::cout << std::endl;
        std::cout << "Note: This utility is designed to ensure consistent shell scripts across different operating systems by normalizing line endings and applying executable permissions where necessary." << std::endl;
        std::cout << "      It can be a constrictive or inclusive as necessary." << std::endl;
        return 0;
    }

    std::vector<fs::path> explicit_files;
    std::vector<fs::path> explicit_paths;
    LineEndingMode mode = LineEndingMode::Auto;
    bool mode_flag_set = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-search") == 0 && i + 1 < argc) {
            explicit_paths.push_back(argv[++i]);
        } else if (std::strcmp(argv[i], "-lf") == 0) {
            if (mode_flag_set && mode != LineEndingMode::ForceLF) {
                std::cerr << "Error: Multiple conflicting line-ending modes specified. "
                          << "The mode flag applies globally to the entire command run line." << std::endl;
                return 1;
            }
            mode = LineEndingMode::ForceLF;
            mode_flag_set = true;
        } else if (std::strcmp(argv[i], "-crlf") == 0) {
            if (mode_flag_set && mode != LineEndingMode::ForceCRLF) {
                std::cerr << "Error: Multiple conflicting line-ending modes specified. "
                          << "The mode flag applies globally to the entire command run line." << std::endl;
                return 1;
            }
            mode = LineEndingMode::ForceCRLF;
            mode_flag_set = true;
        } else {
            // Because you decided against plain path target arguments, raw inputs fall directly into files.
            explicit_files.push_back(argv[i]);
        }
    }

    // Process our explicitly target-searched trees
    for (const auto& p : explicit_paths) {
        search_and_process(mode, p);
    }

    // Process explicit file arguments
    for (const auto& p : explicit_files) {
        process_file(mode, p);
    }

    return 0;
}