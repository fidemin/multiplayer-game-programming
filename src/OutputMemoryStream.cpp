#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include "ErrorUtil.cpp"

class OutputMemoryBitStream {
    public:
        OutputMemoryBitStream(): mBuffer(nullptr), mHead(0), mCapacity(0) {
            ReallocBuffer(32); // start with 32 bytes
        }
        ~OutputMemoryBitStream() {
            free(mBuffer);
        }

        void Write(const void* inData, size_t inByteCount);
        template<typename T> void Write(const T& inData) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "OutputMemoryStream::Write<T> requires an arithmetic or enum type");
            Write(&inData, sizeof(inData));
        }

        void Write(const std::string& inString) {
            uint32_t stringLength = static_cast<uint32_t>(inString.size());
            Write(stringLength);
            Write(inString.c_str(), stringLength);
        }
        
        char* GetBufferPtr() const { return mBuffer; }
        uint32_t GetLength() const { return mHead; }
    private:
        char* mBuffer;
        uint32_t mHead;
        uint32_t mCapacity;

        void ReallocBuffer(uint32_t inNewLength);
};

void OutputMemoryBitStream::ReallocBuffer(uint32_t inNewLength) {
    char* backup = mBuffer;

    mBuffer = static_cast<char*>(realloc(mBuffer, inNewLength));
    if (mBuffer == nullptr) {
        ErrorUtil::ReportError(L"OutputMemoryStream::ReallocBuffer");
        throw std::bad_alloc();
    }
    mCapacity = inNewLength;
}

void OutputMemoryBitStream::Write(const void* inData, size_t inByteCount) {
    uint32_t requiredCapacity = mHead + static_cast<uint32_t>(inByteCount);
    uint32_t newCapacity = mCapacity;

    while (newCapacity < requiredCapacity) {
        newCapacity *= 2;
    }

    if (newCapacity != mCapacity) {
        ReallocBuffer(newCapacity);
    }

    // mBuffer: start pointer of the buffer
    // mHead: offset of the buffer where the new data should be written
    memcpy(mBuffer + mHead, inData, inByteCount);
    mHead = requiredCapacity;
}

