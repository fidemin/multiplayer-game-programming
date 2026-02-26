
#pragma once
#include <cstdlib>
#include <memory>
#include <string>

class InputMemoryBitStream {
    public:
        InputMemoryBitStream(const char* inBuffer, size_t inBitCount) : mBitHead(0), mBitCapacity(static_cast<uint32_t>(inBitCount)) {
            mBuffer = static_cast<char*>(std::malloc((inBitCount + 7) >> 3));
            if (mBuffer == nullptr) {
                // Handle memory allocation failure
                throw std::bad_alloc();
            }
            std::memcpy(mBuffer, inBuffer, (inBitCount + 7) >> 3);
        }
        ~InputMemoryBitStream() {
            std::free(mBuffer);
        };

        void ReadBits(void* outData, size_t inBitCount);
        void ReadBits(uint8_t& outData, size_t inBitCount);
        void Read(bool& outData) {
            uint8_t byteData;
            ReadBits(byteData, 1);
            outData = (byteData != 0);
        }
        void Read(std::string& outString) {
            uint32_t stringLength;
            Read(stringLength);
            outString.resize(stringLength);
            ReadBits(&outString[0], stringLength * 8);
        }

        template<typename T> void Read(T& outData, size_t inBitCount = sizeof(T) * 8) {
            static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "InputMemoryBitStream::ReadBits<T> requires an arithmetic or enum type");
            ReadBits(&outData, inBitCount);
        }

        uint32_t GetRemainingBitCount() const { return mBitCapacity - mBitHead;}

    private:
        char* mBuffer;
        uint32_t mBitHead;
        uint32_t mBitCapacity;
};

void InputMemoryBitStream::ReadBits(uint8_t& outData, size_t inBitCount) {
    // uint32_t byteOffset = mBitHead >> 3;
	// uint32_t bitOffset = mBitHead & 0x7;
	
	// outData = static_cast< uint8_t >( mBuffer[ byteOffset ] ) >> bitOffset;
	
	// uint32_t bitsFreeThisByte = 8 - bitOffset;

    // // if the bits we need to read span across the current byte and the next byte, we need to read from the next byte as well
	// if( bitsFreeThisByte < inBitCount )
	// {
	// 	//we need another byte
	// 	outData |= static_cast< uint8_t >( mBuffer[ byteOffset + 1 ] ) << bitsFreeThisByte;
	// }
	
	// //don't forget a mask so that we only read the bit we wanted...
	// outData &= ( ~( 0x00ff << inBitCount ) );
	
	// mBitHead += inBitCount;
    uint32_t thresholdBitCapacity = mBitHead + static_cast<uint32_t>(inBitCount);
    if (thresholdBitCapacity > mBitCapacity) {
        throw std::runtime_error("InputMemoryBitStream::Read - not enough data to read");
    }


    uint32_t byteOffset = mBitHead >> 3;
    uint32_t bitOffset = mBitHead & 0x07;
    uint32_t leftBitsInCurrentByte = 8 - bitOffset;
    
    // mBuffer is char* which is shifted with keeping signedness on some platforms, so we need to cast it to uint8_t to avoid sign extension when shifting.
    // <Example>
    // Unsigned shift: 0b10101000 (-88 in signed char) >> 2 = 0b00101010 (+42 in unsigned char)
    // Signed shift: 0b10101000 >> 2 (-88 in signed char) = 0b11101010 (-22 in signed char, which is not what we want)
    outData = static_cast<uint8_t>(mBuffer[byteOffset]) >> bitOffset;

    if (leftBitsInCurrentByte < inBitCount) {
        outData |= static_cast<uint8_t>(mBuffer[byteOffset + 1]) << leftBitsInCurrentByte;
    }

    // Mask out the bits we don't care about (for inBitCount less than 8)
	outData &= ( ~( 0x00ff << inBitCount ) );
    mBitHead = thresholdBitCapacity;
}


void InputMemoryBitStream::ReadBits(void* outData, size_t inBitCount) {
    uint32_t thresholdBitCapacity = mBitHead + static_cast<uint32_t>(inBitCount);
    if (thresholdBitCapacity > mBitCapacity) {
        throw std::runtime_error("InputMemoryBitStream::Read - not enough data to read");
    }

    uint8_t* byteData = static_cast<uint8_t*>(outData);

    while (inBitCount > 8) {
        ReadBits(*byteData, 8);
        byteData++;
        inBitCount -= 8;
    }

    if (inBitCount > 0) {
        ReadBits(*byteData, inBitCount);
    }
}
