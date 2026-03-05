#pragma once

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define IS_LITTLE_ENDIAN 1
#else
    #define IS_LITTLE_ENDIAN 0
#endif

constexpr int GetRequiredBits(int value, int bits = 0) {
    while (value > 0) {
        value >>= 1;
        bits++;
    }
    return bits;
}