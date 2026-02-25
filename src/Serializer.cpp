#pragma once
#include "OutputMemoryStream.cpp"

class Serializer {
    public:
        virtual void Serialize(OutputMemoryStream& inStream) const = 0;
};