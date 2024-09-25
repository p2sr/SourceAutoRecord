#pragma once

#define OFFSET_DEFAULT(name, win, linux) extern int name;
#define OFFSET_EMPTY(name) extern int name;
#define OFFSET_WINDOWS(name, off)
#define OFFSET_LINUX(name, off)

namespace Offsets {
    // All offset types should be defined here
    #include "Offsets/Default.hpp"
}

#undef OFFSET_DEFAULT
#undef OFFSET_WINDOWS
#undef OFFSET_LINUX

#ifdef _WIN32
#define OFFSET_DEFAULT(name, win, linux) name = win;
#define OFFSET_WINDOWS(name, off) name = off;
#define OFFSET_LINUX(name, off);
#else
#define OFFSET_DEFAULT(name, win, linux) name = linux;
#define OFFSET_WINDOWS(name, off);
#define OFFSET_LINUX(name, off) name = off;
#endif
