
constexpr int GetRequiredBits(int value, int bits = 0) {
    while (value > 0) {
        value >>= 1;
        bits++;
    }
    return bits;
}