#include <cstdlib>
#include <memory>
#include <cstring>
#include <stdexcept>
#include <string>

class InputMemoryStream {
    public:
        InputMemoryStream(const char* inBuffer, size_t inByteCount) : mHead(0), mCapacity(static_cast<uint32_t>(inByteCount)) {
            mBuffer = static_cast<char*>(std::malloc(inByteCount));
            if (mBuffer == nullptr) {
                throw std::bad_alloc();
            }
            std::memcpy(mBuffer, inBuffer, inByteCount);
        }
        ~InputMemoryStream() {
            std::free(mBuffer);
        };

        void Read(void* outData, size_t inByteCount);
        void Read(std::string& outString) {
            uint32_t stringLength;
            Read(stringLength);
            outString.resize(stringLength);
            Read(&outString[0], stringLength);
        }
        
        template<typename T> void Read(T& outData) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "InputMemoryStream::Read<T> requires an arithmetic or enum type");
            Read(&outData, sizeof(outData));
        }
    private:
        char* mBuffer;
        uint32_t mHead;
        uint32_t mCapacity;

        
};

void InputMemoryStream::Read(void* outData, size_t inByteCount) {
    uint32_t thresholdCapacity = mHead + static_cast<uint32_t>(inByteCount);
    if (thresholdCapacity > mCapacity) {
        throw std::runtime_error("InputMemoryStream::Read - not enough data to read");
    }

    std::memcpy(outData, mBuffer + mHead, inByteCount);
    mHead = thresholdCapacity;
}