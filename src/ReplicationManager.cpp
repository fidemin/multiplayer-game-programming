
#include <unordered_set>
#include "LinkingContext.cpp"
#include "BitsHelper.cpp"
#include "PacketType.cpp"
#include "ObjectCreationRegistry.cpp"

class ReplicationManager {
    public:
        ReplicationManager(LinkingContext* inLinkingContext) : mLinkingContext(inLinkingContext) {};
        void ReplicateWorld(OutputMemoryBitStream& inStream, std::vector<GameObject*>& gameObjects);
        void ReceiveWorld(InputMemoryBitStream& inStream);

        std::unordered_set<GameObject*> GetObjectsInWorld() const {
            return mReplicatedObjects;
        }
    
    private:
        void ReplicateGameObject(OutputMemoryBitStream& inStream, GameObject* inGameObject);
        GameObject* RecieveGameObject(InputMemoryBitStream& inStream);
        LinkingContext* mLinkingContext;
        std::unordered_set<GameObject*> mReplicatedObjects;
};

void ReplicationManager::ReplicateWorld(OutputMemoryBitStream& inStream, std::vector<GameObject*>& gameObjects) {
    inStream.WriteBits(PT_ReplicationData, GetRequiredBits(PT_MAX));
    inStream.Write(static_cast<uint32_t>(gameObjects.size())); // Write the number of objects being replicated

    for (GameObject* gameObject : gameObjects) {
        ReplicateGameObject(inStream, gameObject);
    }
}

void ReplicationManager::ReceiveWorld(InputMemoryBitStream& inStream) {
    uint32_t packetType;
    inStream.ReadBits(&packetType, GetRequiredBits(PT_MAX));
    uint32_t objectCount;
    inStream.Read(objectCount);

    std::unordered_set<GameObject*> receivedObjects;

    if (packetType == PT_ReplicationData) {
        for (uint32_t i = 0; i < objectCount; ++i) {
             GameObject* thisGameObject = RecieveGameObject(inStream);
             receivedObjects.insert(thisGameObject);
        }

        for (GameObject* gameObject : mReplicatedObjects) {
            if (receivedObjects.find(gameObject) == receivedObjects.end()) {
                // Handle objects that are no longer present
                mLinkingContext->RemoveGameObject(gameObject);
                gameObject->Destroy();
            }
        }
        mReplicatedObjects = receivedObjects;

    } else if (packetType == PT_Hello) {
        // TODO: Handle other packet types as needed

    } else if (packetType == PT_Disconnect) {
        // TODO: Handle disconnect logic as needed
    } else {
        throw std::runtime_error("Unknown packet type received");
    }
}

void ReplicationManager::ReplicateGameObject(OutputMemoryBitStream& inStream, GameObject* inGameObject) {
    uint32_t networkId = mLinkingContext->GetId(inGameObject, true);
    inStream.Write(networkId);

    inStream.Write(inGameObject->GetClassId());

    inGameObject->Serialize(inStream);
}

GameObject* ReplicationManager::RecieveGameObject(InputMemoryBitStream& inStream) {
    uint32_t networkId;
    inStream.Read(networkId);

    uint32_t classId;
    inStream.Read(classId);

    GameObject* existingObject = mLinkingContext->GetGameObject(networkId); 
    if (existingObject) {
        existingObject->Deserialize(inStream);
        return existingObject;
    }

    // new object: create and deserialize
    GameObject* gameObject = ObjectCreationRegistry::GetInstance().CreateGameObject(classId);
    gameObject->Deserialize(inStream);
    mLinkingContext->AddGameObject(networkId, gameObject);

    return gameObject;
}