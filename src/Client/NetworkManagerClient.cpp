
#pragma once
#include "../SocketAddress.cpp"
#include "../OutputMemoryBitStream.cpp"
#include "../InputMemoryBitStream.cpp"
#include "../NetworkManager.cpp"
#include "../ReplicationManager.cpp"
#include "../DeliveryNotificationManager.cpp"


class NetworkManagerClient: public NetworkManager {
    public:
        NetworkManagerClient(const SocketAddress& inServerAddress) : mServerAddress(inServerAddress) {}
        bool Initialize(const uint16_t inPort, const std::string& inName);

        void Send();
        void Read(InputMemoryBitStream& inStream);
        void SetPlayerName(const std::string& inName) { mPlayerName = inName; }

        enum NetworkClientState {
            NCS_Uninitialized,
            NCS_InSync,
            NCS_Connected,
        };

    private:
        SocketAddress mServerAddress;
        NetworkClientState mState = NCS_Uninitialized;
        uint32_t mPlayerId;
        std::string mPlayerName;
        LinkingContext mLinkingContext;
        DeliveryNotificationManager mDeliveryNotificationManager;

        void SendSync();
        void HandleAckedPacket(InputMemoryBitStream& inStream);
        void HandleReplicationData(InputMemoryBitStream& inStream);
};

bool NetworkManagerClient::Initialize(uint16_t inPort, const std::string& inName )
{
	bool result = NetworkManager::Initialize( 0 );
    if (!result) {
        ErrorUtil::ReportError(L"NetworkManagerClient::Initialize - Failed to initialize network manager\n");
        return false;
    }
	mState = NCS_InSync;
    mPlayerName = inName;
    return true;
}


void NetworkManagerClient::Send() {
    printf("NetworkManagerClient::Send - Current state: %d\n", mState);
    if (mState == NCS_InSync) {
        SendSync();
    }
    // TODO: handle other states if necessary
}

void NetworkManagerClient::Read(InputMemoryBitStream& inStream) {
    uint32_t packetType;
    inStream.Read(packetType);

    printf("NetworkManagerClient::Read - Received packet of type: %d\n", packetType);

    switch (packetType) {
        case kAckedCC: {
            HandleAckedPacket(inStream);
            break;
        }
        case kStateCC: {
            // TODO: handle state update from server
            printf("NetworkManagerClient::Read - Received state update packet from server\n");
            HandleReplicationData(inStream);
            break;
        }
        default: {
            ErrorUtil::ReportError((L"NetworkManagerClient::Read - Received unknown packet type: " + std::to_wstring(packetType)).c_str());
            break;
        }
    }
    printf("NetworkManagerClient::Read - Finished processing packet of type: %d\n", packetType);
}

void NetworkManagerClient::SendSync() {
    OutputMemoryBitStream outStream;
    outStream.Write(kSyncCC);
    outStream.Write(mPlayerName);

    SendPacket(outStream, mServerAddress);
}


void NetworkManagerClient::HandleAckedPacket(InputMemoryBitStream& inStream) {
    if (mState == NCS_InSync) {
        uint32_t playerId;
        inStream.Read(playerId);

        mPlayerId = playerId;
        mState = NCS_Connected;
        printf("Received ACK from server. Assigned player ID: %u\n", mPlayerId);
    }
};


void NetworkManagerClient::HandleReplicationData(InputMemoryBitStream& inStream) {
    uint32_t packetType;
    printf("NetworkManagerClient::HandleReplicationData - Received replication data packet with type: %d\n", packetType);

    std::unordered_set<GameObject*> receivedObjects;

    // TODO: process acks and send pending acks back to server if necessary
    mDeliveryNotificationManager.ProcessSequenceNumber(inStream);
    mDeliveryNotificationManager.ProcessAcks(inStream);
    uint32_t commandCount;
    inStream.Read(commandCount);
    for (uint32_t i = 0; i < commandCount; ++i) {
        ReplicationHeader header;
        header.Read(inStream);
        if (header.GetAction() == RA_Create) {
            printf("Received create for object with network ID: %u, class ID: %u\n", header.GetNetworkId(), header.GetClassId());
            GameObject* gameObject = ObjectCreationRegistry::GetInstance().CreateGameObject(header.GetClassId());
            gameObject->Deserialize(inStream);
            receivedObjects.insert(gameObject);
        } else if (header.GetAction() == RA_Update) {
            printf("Received update for object with network ID: %u\n", header.GetNetworkId());
            GameObject* existingObject = mLinkingContext.GetGameObject(header.GetNetworkId());
            if (existingObject) {
                existingObject->Deserialize(inStream);
                receivedObjects.insert(existingObject);
            } else {
                ErrorUtil::ReportError((L"NetworkManagerClient::HandleReplicationData - Received update for non-existing object with network ID: " + std::to_wstring(header.GetNetworkId())).c_str());
            }
        } else if (header.GetAction() == RA_Destroy) {
            GameObject* existingObject = mLinkingContext.GetGameObject(header.GetNetworkId());
            if (existingObject) {
                printf("Received destroy for object with network ID: %u\n", header.GetNetworkId());
                mLinkingContext.RemoveGameObject(existingObject);
                existingObject->Destroy();
            } else {
                ErrorUtil::ReportError((L"NetworkManagerClient::HandleReplicationData - Received destroy for non-existing object with network ID: " + std::to_wstring(header.GetNetworkId())).c_str());
            }
        } else {
            ErrorUtil::ReportError((L"NetworkManagerClient::HandleReplicationData - Received unknown replication action: " + std::to_wstring(header.GetAction())).c_str());
        }
    }
} 
