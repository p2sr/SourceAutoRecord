#include "Offsets.hpp"

#undef OFFSET_DEFAULT
#undef OFFSET_EMPTY
#undef OFFSET_LINMOD

#define OFFSET_DEFAULT(name, win, linux) int name;
#define OFFSET_EMPTY(name) int name;
#define OFFSET_LINMOD(name, off)

namespace Offsets {
    #include "Offsets/Default.hpp"
}
