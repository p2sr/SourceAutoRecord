#pragma once

#define OFFSET_DEFAULT(name, win, linux) extern int name;
#define OFFSET_EMPTY(name) extern int name;
#define OFFSET_WINDOWS(name, off)
#define OFFSET_LINUX(name, off)

#define SIGSCAN_DEFAULT(name, win, linux) extern const char *name;
#define SIGSCAN_EMPTY(name) extern const char *name;
#define SIGSCAN_WINDOWS(name, sig)
#define SIGSCAN_LINUX(name, sig)

namespace Offsets {
    // All offset types should be defined here
    #include "Offsets/Default.hpp"
}

#undef OFFSET_DEFAULT
#undef OFFSET_EMPTY
#undef OFFSET_WINDOWS
#undef OFFSET_LINUX

#undef SIGSCAN_DEFAULT
#undef SIGSCAN_EMPTY
#undef SIGSCAN_WINDOWS
#undef SIGSCAN_LINUX

#ifdef _WIN32
#define OFFSET_DEFAULT(name, win, linux) name = win;
#define OFFSET_EMPTY(name) name = 0;
#define OFFSET_WINDOWS(name, off) name = off;
#define OFFSET_LINUX(name, off);

#define SIGSCAN_DEFAULT(name, win, linux) name = win;
#define SIGSCAN_EMPTY(name) name = "";
#define SIGSCAN_WINDOWS(name, sig) name = sig;
#define SIGSCAN_LINUX(name, sig)
#else
#define OFFSET_DEFAULT(name, win, linux) name = linux;
#define OFFSET_EMPTY(name) name = 0;
#define OFFSET_WINDOWS(name, off);
#define OFFSET_LINUX(name, off) name = off;

#define SIGSCAN_DEFAULT(name, win, linux) name = linux;
#define SIGSCAN_EMPTY(name) name = "";
#define SIGSCAN_WINDOWS(name, sig);
#define SIGSCAN_LINUX(name, sig) name = sig;
#endif
