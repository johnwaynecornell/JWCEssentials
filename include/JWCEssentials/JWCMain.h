#ifndef JWC_MAIN_H
#define JWC_MAIN_H

#include <string>
#include <vector>

int JWCMain(int argc, char** argv);

#ifdef _WIN32
#include <windows.h>

namespace JWCEssentials
{
    inline std::string WideToUtf8(const wchar_t* value)
    {
        if (!value) return {};

        int size = WideCharToMultiByte(
            CP_UTF8,
            0,
            value,
            -1,
            nullptr,
            0,
            nullptr,
            nullptr
        );

        if (size <= 0) return {};

        std::string result(size - 1, '\0');

        WideCharToMultiByte(
            CP_UTF8,
            0,
            value,
            -1,
            result.data(),
            size,
            nullptr,
            nullptr
        );

        return result;
    }

    inline int JWCMain_WideEntry(int argc, wchar_t** wargv)
    {
        std::vector<std::string> args;
        args.reserve(argc);

        std::vector<char*> argv;
        argv.reserve(argc + 1);

        for (int i = 0; i < argc; i++)
            args.push_back(WideToUtf8(wargv[i]));

        for (auto& arg : args)
            argv.push_back(arg.data());

        argv.push_back(nullptr);

        return JWCMain(argc, argv.data());
    }
}


int wmain(int argc, wchar_t** argv)
{
    return JWCEssentials::JWCMain_WideEntry(argc, argv);
}

#else

int main(int argc, char** argv)
{
    return JWCMain(argc, argv);
}

#endif

#endif