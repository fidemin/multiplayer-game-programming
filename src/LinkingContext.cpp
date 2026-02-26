#include <cstdint>
#include <unordered_map>
#include "GameObject.cpp"

class LinkingContext {
    public:
        LinkingContext(): mNextId(1) {}
        uint32_t GetId(GameObject* inGameObject, bool shouldCreateIfNotFound) {
            auto it = mGameObjectToIdMap.find(inGameObject);
            if (it != mGameObjectToIdMap.end()) {
                return it->second;
            }
            if (shouldCreateIfNotFound) {
                uint32_t newId = GetNextId();
                mGameObjectToIdMap[inGameObject] = newId;
                mIdToGameObjectMap[newId] = inGameObject;
                return newId;
            }
            return 0; // 0 can be reserved for "not found"
        }

        GameObject* GetGameObject(uint32_t inId) const {
            auto it = mIdToGameObjectMap.find(inId);
            if (it != mIdToGameObjectMap.end()) {
                return it->second;
            }
            return nullptr;
        }

        void AddGameObject(uint32_t inId, GameObject* inGameObject) {
            if (inId == 0) {
                throw std::invalid_argument("ID 0 is reserved for 'not found'");
            }

            if (mIdToGameObjectMap.find(inId) != mIdToGameObjectMap.end()) {
                throw std::runtime_error("ID already exists in the linking context");
            }

            if (mGameObjectToIdMap.find(inGameObject) != mGameObjectToIdMap.end()) {
                throw std::runtime_error("GameObject already exists in the linking context");
            }

            mIdToGameObjectMap[inId] = inGameObject;
            mGameObjectToIdMap[inGameObject] = inId;
        }

        void RemoveGameObject(GameObject* inGameObject) {
            auto it = mGameObjectToIdMap.find(inGameObject);
            if (it != mGameObjectToIdMap.end()) {
                uint32_t id = it->second;
                mGameObjectToIdMap.erase(it);
                mIdToGameObjectMap.erase(id);
            }
        }
    private:
        uint32_t mNextId;
        uint32_t GetNextId() { return mNextId++; }
        std::unordered_map<GameObject*, uint32_t> mGameObjectToIdMap;
        std::unordered_map<uint32_t, GameObject*> mIdToGameObjectMap;
};

