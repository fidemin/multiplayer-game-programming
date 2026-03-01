#include "../SocketAddress.cpp"
#include "../OutputMemoryBitStream.cpp"
#include "../InputMemoryBitStream.cpp"
#include "../NetworkManager.cpp"


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

        void SendSync();
        void HandleAckedPacket(InputMemoryBitStream& inStream);
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
    wprintf(L"NetworkManagerClient::Send - Current state: %d\n", mState);
    if (mState == NCS_InSync) {
        SendSync();
    }
    // TODO: handle other states if necessary
}

void NetworkManagerClient::Read(InputMemoryBitStream& inStream) {
    uint32_t packetType;
    inStream.Read(packetType);

    wprintf(L"NetworkManagerClient::Read - Received packet type %d\n", packetType);

    switch (packetType) {
        case kAckedCC: {
            HandleAckedPacket(inStream);
            break;
        }
        case kStateCC: {
            // TODO: handle state update from server
            break;
        }
    }
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
        wprintf(L"Received ACK from server. Assigned player ID: %u\n", mPlayerId);
    }
};



