#pragma once
#include <cstdint>
#include <ctime>
#include <chrono>

class Timing {
    public:
        static Timing sInstance;
        uint64_t GetTimeMS() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        }
};

Timing Timing::sInstance;
