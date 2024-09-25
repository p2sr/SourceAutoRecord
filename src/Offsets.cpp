#include "Offsets.hpp"

#undef OFFSET_DEFAULT
#undef OFFSET_EMPTY
#undef OFFSET_WINDOWS
#undef OFFSET_LINUX

#define OFFSET_DEFAULT(name, win, linux) int name;
#define OFFSET_EMPTY(name) int name;
#define OFFSET_WINDOWS(name, off)
#define OFFSET_LINUX(name, off)

namespace Offsets {
    #include "Offsets/Default.hpp"
}
