#ifndef COMMON_HPP
#define COMMON_HPP

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
    #define PIPE_READ_END 0
    #define PIPE_WRITE_END 1
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <fcntl.h>
    #include <cerrno>
    #include <cstring>
    #define PIPE_READ_END 0
    #define PIPE_WRITE_END 1
#endif

#include <iostream>
#include <string>
#include <cstdlib>

#ifdef _WIN32
    #define ERR_EXIT(msg) do { \
        std::cerr << "[PID " << GetCurrentProcessId() << "] Error in " << msg \
                  << ": " << GetLastError() << std::endl; \
        std::exit(EXIT_FAILURE); \
    } while(0)
#else
    #define ERR_EXIT(msg) do { \
        std::cerr << "[PID " << getpid() << "] Error in " << msg \
                  << ": " << std::strerror(errno) << std::endl; \
        std::exit(EXIT_FAILURE); \
    } while(0)
#endif

#endif