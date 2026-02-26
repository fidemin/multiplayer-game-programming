#pragma once
#include <cstdint>
#include <string>

#define CLASS_IDENIFICATION(inCode, inClass) \
    enum { kClassId = inCode }; \
    virtual uint32_t GetClassId() const { return kClassId; } \
    static GameObject* CreateInstance() { return new inClass(); }

class GameObject {
    public:
        CLASS_IDENIFICATION('GOBJ', GameObject)
        virtual std::string GetDescription() const { return "GameObject(" + std::to_string(GetClassId()) + ")"; }
};