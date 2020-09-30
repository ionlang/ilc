#pragma once

// Determine OS.
#if defined(WIN32) || defined (_WIN32) || defined(_WIN64) || defined(__NT__)
    #define OS_WINDOWS
#elif defined(__unix__) || defined(__unix) || defined(__linux__)
    #define OS_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
    #define OS_MAC
#else
    #error "Unknown or unsupported platform"
#endif

// Include appropriate file depending on OS.
#if defined(OS_LINUX) || defined(OS_MAC)
    #include <unistd.h>
#elif defined(OS_WINDOWS)
    #if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0600)
        #error "Please include this file before any Windows system headers or set _WIN32_WINNT at least to _WIN32_WINNT_VISTA"
    #elif !defined(_WIN32_WINNT)
        #define _WIN32_WINNT _WIN32_WINNT_VISTA
    #endif

    #include <windows.h>
    #include <io.h>
    #include <memory>

    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#endif
