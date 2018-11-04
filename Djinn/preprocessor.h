#pragma once

// lay down some constants
#define DJINN_PLATFORM_WINDOWS 1
#define DJINN_PLATFORM_LINUX 2

// try to autodetect the platform
#ifdef _WIN32
    #define DJINN_PLATFORM DJINN_PLATFORM_WINDOWS
#else
    #define DJINN_PLATFORM DJINN_PLATFORM_LINUX
#endif

// perform platform-specific fixes
#if DJINN_PLATFORM == DJINN_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NO_MINMAX

    #include <windows.h>
    #include <winsdkver.h> // this defines the _WIN32_WINNT value (relevant for boost::asio)
    //#include <DbgHelp.h> // [TODO] -- implement stack tracing
    #include <intrin.h>

    #define VK_USE_PLATFORM_WIN32_KHR
#endif

// with MSVC we can figure out wheter this is a debug build
#ifdef _DEBUG
    #define DJINN_DEBUG
#endif

// NO_MINMAX should mean that these are not defined, but just in case
#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif
