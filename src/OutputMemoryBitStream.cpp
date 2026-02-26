
#pragma once
#include <cstdlib>
#include <memory>
#include <string>

class OutputMemoryBitStream {
    public:
        OutputMemoryBitStream() : mBuffer(nullptr), mBitHead(0), mBitCapacity(0) {
            ReallocBuffer(32 * 8); // start with 32 bytes (256 bits)
        }
        ~OutputMemoryBitStream() {
            free(mBuffer);
        }

        void WriteBits(const void* inData, size_t inBitCount);
        void WriteBits(const uint8_t inData, size_t inBitCount);
        void Write(const bool& inData) { WriteBits(inData ? 1 : 0, 1); }
        void Write(const std::string& inString) {
            uint32_t stringLength = static_cast<uint32_t>(inString.size());
            Write(stringLength);
            WriteBits(inString.c_str(), stringLength * 8);
        }
        template<typename T> void Write(const T& inData, size_t inBitCount = sizeof(T) * 8) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "OutputMemoryBitStream::WriteBits<T> requires an arithmetic or enum type");
            WriteBits(&inData, inBitCount);
        }

        const char* GetBufferPtr() const { return mBuffer; }
        uint32_t GetBitLength() const { return mBitHead; }
        uint32_t GetByteLength() const { return (mBitHead + 7) >> 3; }

    private:
        char* mBuffer;
        uint32_t mBitHead;
        uint32_t mBitCapacity;

        void ReallocBuffer(uint32_t inNewBitCapacity);
};

void OutputMemoryBitStream::WriteBits(const void* inData, size_t inBitCount) {
    const uint8_t* byteData = static_cast<const uint8_t*>(inData);
    while (inBitCount > 8) {
        WriteBits(*byteData, 8);
        byteData++;
        inBitCount -= 8;
    }

    if (inBitCount > 0) {
        WriteBits(*byteData, inBitCount);
    }
}


void OutputMemoryBitStream::WriteBits(const uint8_t inData, size_t inBitCount) {
    // Write bits within a single byte
    uint32_t requiredBitCapacity = mBitHead + static_cast<uint32_t>(inBitCount);
    uint32_t newBitCapacity = mBitCapacity;

    while (newBitCapacity < requiredBitCapacity) {
        newBitCapacity *= 2;
    }

    if (newBitCapacity != mBitCapacity) {
        ReallocBuffer(newBitCapacity);
    }

    // byte offset
    uint32_t byteOffset = mBitHead >> 3;
    // bit offset
    uint32_t bitOffset = mBitHead & 0x7;

    // clear the bits at the target position and set the new bits
    uint8_t currentMask = ~(0xFF << bitOffset);
    mBuffer[byteOffset] = (mBuffer[byteOffset] & currentMask) | (inData << bitOffset);

    uint32_t bitsFreeInCurrentByte = 8 - bitOffset;
    if (bitsFreeInCurrentByte < inBitCount) {
        mBuffer[byteOffset + 1] = inData >> bitsFreeInCurrentByte;
    }
    mBitHead = requiredBitCapacity;
}

void OutputMemoryBitStream::ReallocBuffer(uint32_t inNewBitCapacity) {
    char* backup = mBuffer;

    mBuffer = static_cast<char*>(realloc(mBuffer, (inNewBitCapacity + 7) >> 3));
    if (mBuffer == nullptr) {
        free(backup);
        throw std::bad_alloc();
    }
    mBitCapacity = inNewBitCapacity;
}