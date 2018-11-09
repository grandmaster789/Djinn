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
	// disable as much of the windows header as possible

	#define WIN32_LEAN_AND_MEAN // disables many additional headers

	#define NOGDICAPMASKS     // CC_ * , LC_*, PC_*, CP_*, TC_*, RC_
	//#define NOVIRTUALKEYCODES // VK_ *
	//#define NOWINMESSAGES     // WM_ * , EM_*, LB_*, CB_*
	//#define NOWINSTYLES       // WS_ * , CS_*, ES_*, LBS_*, SBS_*, CBS_*
	//#define NOSYSMETRICS      // SM_ * 
	#define NOMENUS           // - MF_ *
	//#define NOICONS           // - IDI_ *
	#define NOKEYSTATES       // - MK_ *
	#define NOSYSCOMMANDS     // - SC_ *
	#define NORASTEROPS       // - Binary and Tertiary raster ops
	//#define NOSHOWWINDOW      // - SW_ *
	#define OEMRESOURCE       // - OEM Resource values
	#define NOATOM            // - Atom Manager routines
	#define NOCLIPBOARD       // - Clipboard routines
	#define NOCOLOR           // - Screen colors
	#define NOCTLMGR          // - Control and Dialog routines
	#define NODRAWTEXT        // - DrawText() and DT_*
	//#define NOGDI             // - All GDI defines and routines
	#define NOKERNEL          // - All KERNEL defines and routines
	//#define NOUSER            // - All USER defines and routines
	#define NONLS             // - All NLS defines and routines
	#define NOMB              // - MB_ * and MessageBox()
	#define NOMEMMGR          // - GMEM_ * , LMEM_*, GHND, LHND, associated routines
	#define NOMETAFILE        // - typedef METAFILEPICT
	#define NOMINMAX          // - Macros min(a, b) and max(a, b)
	//#define NOMSG             // - typedef MSG and associated routines
	#define NOOPENFILE        // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
	#define NOSCROLL          // - SB_ * and scrolling routines
	#define NOSERVICE         // - All Service Controller routines, SERVICE_ equates, etc.
	#define NOSOUND           // - Sound driver routines
	#define NOTEXTMETRIC      // - typedef TEXTMETRIC and associated routines
	#define NOWH              // - SetWindowsHook and WH_*
	//#define NOWINOFFSETS      // - GWL_ * , GCL_*, associated routines
	#define NOCOMM            // - COMM driver routines
	#define NOKANJI           // - Kanji support stuff.
	#define NOHELP            // - Help engine interface.
	#define NOPROFILER        // - Profiler interface.
	#define NODEFERWINDOWPOS  // - DeferWindowPos routines
	#define NOMCX             // - Modem Configuration Extensions

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
