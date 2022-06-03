#pragma once

#define OFFSET_DEFAULT(name, win, linux) extern int name;
#define OFFSET_EMPTY(name) extern int name;

namespace Offsets {
    #include "OffsetsData.hpp"
}

#undef OFFSET_DEFAULT
#undef OFFSET_EMPTY
