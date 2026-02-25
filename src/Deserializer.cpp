
#include "InputMemoryStream.cpp"

class Deserializer {
    public:
        virtual void Deserialize(InputMemoryStream& inStream) = 0;
};