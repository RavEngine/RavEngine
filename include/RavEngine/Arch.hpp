#pragma once
#include <cstdint>

// 32-bit detection. if __WORDSIZE does not exist, then we'll just assume 64 bit.
#if __WORDSIZE == 32 || __EMSCRIPTEN__
    #define RVE_32_BIT 1
#else
    #define RVE_64_BIT 1
#endif
