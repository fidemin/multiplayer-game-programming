#include <cassert>
#include <unordered_map>
#include "GameObject.cpp"
#include "RoboCat.cpp"

using GameObjectCreationFunction = GameObject* (*)();

class ObjectCreationRegistry {
    public:
        static ObjectCreationRegistry& GetInstance() {
            static ObjectCreationRegistry instance;
            return instance;
        }

        template<class T>
        void RegisterCreationFunction() {
            // check if the class ID is already registered
            assert(mCreationFunctionMap.find(T::kClassId) == mCreationFunctionMap.end() && "Class ID already registered");
            mCreationFunctionMap[T::kClassId] = T::CreateInstance;
        }

        GameObject* CreateGameObject(uint32_t inClassId) const {
            auto it = mCreationFunctionMap.find(inClassId);

            if (it == mCreationFunctionMap.end()) {
                throw std::runtime_error("Class ID not found in registry");
            }
            return it->second();
        }

    private:
        ObjectCreationRegistry() {}
        // NOTE: uint32_t for kClassId, which is four-characters literal, which can be represented as a 32-bit integer
        std::unordered_map<uint32_t, GameObjectCreationFunction> mCreationFunctionMap;
};

void RegisterGameObjectCreationFunctions() {
    ObjectCreationRegistry::GetInstance().RegisterCreationFunction<GameObject>();
    ObjectCreationRegistry::GetInstance().RegisterCreationFunction<RoboCat>();
}