#include "Offsets.hpp"

#undef OFFSET_DEFAULT
#undef OFFSET_EMPTY
#undef OFFSET_WINDOWS
#undef OFFSET_LINUX

#undef SIGSCAN_DEFAULT
#undef SIGSCAN_EMPTY
#undef SIGSCAN_WINDOWS
#undef SIGSCAN_LINUX

#define OFFSET_DEFAULT(name, win, linux) int name;
#define OFFSET_EMPTY(name) int name;
#define OFFSET_WINDOWS(name, off)
#define OFFSET_LINUX(name, off)

#define SIGSCAN_DEFAULT(name, win, linux) const char *name;
#define SIGSCAN_EMPTY(name) const char *name;
#define SIGSCAN_WINDOWS(name, sig)
#define SIGSCAN_LINUX(name, sig)

namespace Offsets {
    #include "Offsets/Default.hpp"
}
