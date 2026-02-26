#pragma once
#include "InputMemoryBitStream.cpp"

class Deserializer {
    public:
        virtual void Deserialize(InputMemoryBitStream& inStream) = 0;
};