#pragma once
#include "OutputMemoryBitStream.cpp"

class Serializer {
    public:
        virtual void Serialize(OutputMemoryBitStream& inStream) const = 0;
};