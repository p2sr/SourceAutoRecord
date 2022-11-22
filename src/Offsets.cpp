#include "Offsets.hpp"

#define OFFSET_DEFAULT(name, win, linux) int name;
#define OFFSET_EMPTY(name) int name;
#define OFFSET_LINMOD(name, off)

namespace Offsets {
    #include "OffsetsData.hpp"
}
