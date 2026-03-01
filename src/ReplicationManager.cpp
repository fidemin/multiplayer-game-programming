#pragma once
#include <unordered_set>
#include "LinkingContext.cpp"
#include "BitsHelper.cpp"
#include "PacketType.cpp"
#include "ObjectCreationRegistry.cpp"
#include "ReplicationHeader.cpp"

class ReplicationManager {
    public:
        ReplicationManager(LinkingContext* inLinkingContext) : mLinkingContext(inLinkingContext) {};
        void ReplicateWorld(OutputMemoryBitStream& inStream, std::vector<GameObject*>& gameObjects);
        void ReceiveWorld(InputMemoryBitStream& inStream);

        void ReplicateCreate(OutputMemoryBitStream& inStream, GameObject* inGameObject);        
        void ReplicateUpdate(OutputMemoryBitStream& inStream, GameObject* inGameObject);
        void ReplicateDestroy(OutputMemoryBitStream& inStream, GameObject* inGameObject);

        void ProcessReplicationAction(InputMemoryBitStream& inStream);

        std::unordered_set<GameObject*> GetObjectsInWorld() const {
            return mReplicatedObjects;
        }

    protected:
        LinkingContext* mLinkingContext;
    
    private:
        void ReplicateGameObject(OutputMemoryBitStream& inStream, GameObject* inGameObject);
        GameObject* RecieveGameObject(InputMemoryBitStream& inStream);
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

void ReplicationManager::ReplicateCreate(OutputMemoryBitStream& inStream, GameObject* inGameObject) {
    ReplicationHeader header(RA_Create, mLinkingContext->GetId(inGameObject, false), inGameObject->GetClassId());
    header.Write(inStream);
    inGameObject->Serialize(inStream);
}

void ReplicationManager::ReplicateUpdate(OutputMemoryBitStream& inStream, GameObject* inGameObject) {
    ReplicationHeader header(RA_Update, mLinkingContext->GetId(inGameObject, false), inGameObject->GetClassId());
    header.Write(inStream);
    inGameObject->Serialize(inStream);
}

void ReplicationManager::ReplicateDestroy(OutputMemoryBitStream& inStream, GameObject* inGameObject) {
    // For destroy action, we only need to send the network ID to identify which object to destroy on the receiving end. The receiving end can call GameObject's Destroy() method to clean up the object.
    // NOTE: remove object from the linking context should be handled by the caller after calling ReplicateDestroy.
    ReplicationHeader header(RA_Destroy, mLinkingContext->GetId(inGameObject, false));
    header.Write(inStream);
}

void ReplicationManager::ProcessReplicationAction(InputMemoryBitStream& inStream) {
    ReplicationHeader header;
    header.Read(inStream);

    switch (header.GetAction()) {
        case RA_Create: {
            GameObject* gameObject = ObjectCreationRegistry::GetInstance().CreateGameObject(header.GetClassId());
            gameObject->Deserialize(inStream);
            mLinkingContext->AddGameObject(header.GetNetworkId(), gameObject);
            break;
        }
        case RA_Update: {
            GameObject* gameObject = mLinkingContext->GetGameObject(header.GetNetworkId());
            if (gameObject) {
                gameObject->Deserialize(inStream);
            } else {
                // Handle the case where the object to update does not exist (e.g., log an error)
                wprintf(L"Error: Received update for non-existent object with network ID %u\n", header.GetNetworkId());
            }
            break;
        }
        case RA_Destroy: {
            GameObject* gameObject = mLinkingContext->GetGameObject(header.GetNetworkId());
            if (gameObject) {
                mLinkingContext->RemoveGameObject(gameObject);
                gameObject->Destroy();
            } else {
                // Handle the case where the object to destroy does not exist (e.g., log an error)
                wprintf(L"Error: Received destroy for non-existent object with network ID %u\n", header.GetNetworkId());
            }
            break;
        }
        default:
            throw std::runtime_error("Unknown replication action received");
    }
}
