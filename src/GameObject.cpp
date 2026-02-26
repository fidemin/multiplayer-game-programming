#pragma once
#include "Serializer.cpp"
#include "Deserializer.cpp"
#include <cstdint>
#include <string>


#define CLASS_IDENIFICATION(inCode, inClass) \
    enum { kClassId = inCode }; \
    virtual uint32_t GetClassId() const { return kClassId; } \
    virtual void Destroy() { delete this; } \
    static GameObject* CreateInstance() { return new inClass(); }


class GameObject: public Serializer, public Deserializer {
    public:
        CLASS_IDENIFICATION('GOBJ', GameObject)
        virtual std::string GetDescription() const { return "GameObject(" + std::to_string(GetClassId()) + ")"; }
        virtual void Serialize(OutputMemoryBitStream& inStream) const override {
            // TODO: implement serialization of GameObject's state, if any
        }
        virtual void Deserialize(InputMemoryBitStream& inStream) override {
            // TODO: implement deserialization of GameObject's state, if any
        }
};