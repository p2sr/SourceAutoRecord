#pragma once

#define OFFSET_DEFAULT(name, win, linux) extern int name;
#define OFFSET_EMPTY(name) extern int name;
#define OFFSET_LINMOD(name, off)

namespace Offsets {
    // All offset types should be defined here
    #include "Offsets/Default.hpp"
}

#undef OFFSET_DEFAULT
#undef OFFSET_LINMOD

#ifdef _WIN32
#define OFFSET_DEFAULT(name, win, linux) name = win;
#define OFFSET_LINMOD(name, off);
#else
#define OFFSET_DEFAULT(name, win, linux) name = linux;
#define OFFSET_LINMOD(name, off) name = off;
#endif
