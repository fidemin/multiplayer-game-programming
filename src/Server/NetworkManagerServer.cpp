#pragma once

#include <unordered_map>

#include "../SocketAddress.cpp"
#include "../NetworkManager.cpp"
#include "../InputMemoryBitStream.cpp"

class NetworkManagerServer : public NetworkManager {
    public:
        void ProcessPacket(InputMemoryBitStream& inStream, const SocketAddress& fromAddress);
    private:
        std::unordered_map<const SocketAddress*, uint32_t> mClientAddressToIdMap;
        std::unordered_map<uint32_t, const SocketAddress*> mClientIdToAddressMap;

        void HandlePacketFromNewClient(InputMemoryBitStream& inStream, const SocketAddress& fromAddress);
        void HandlePacketFromExistingClient(InputMemoryBitStream& inStream, uint32_t clientId);
        // TODO: implement SendAckPacket
        void SendAckPacket(const SocketAddress& toAddress);
};

void NetworkManagerServer::ProcessPacket(InputMemoryBitStream& inStream, const SocketAddress& fromAddress) {
    printf("NetworkManagerServer::ProcessPacket - Received packet from %s\n", fromAddress.ToString().c_str());
    auto it = mClientAddressToIdMap.find(&fromAddress);
    if (it == mClientAddressToIdMap.end()) {
        HandlePacketFromNewClient(inStream, fromAddress);
    } else {
        uint32_t clientId = it->second;
        HandlePacketFromExistingClient(inStream, clientId);
    };
}

void NetworkManagerServer::HandlePacketFromNewClient(InputMemoryBitStream& inStream, const SocketAddress& fromAddress) {
    uint32_t packetType;
    inStream.Read(packetType);

    printf("NetworkManagerServer::HandlePacketFromNewClient - Received packet type %d from new client with address: %s\n", packetType, fromAddress.ToString().c_str());

    if (packetType == kSyncCC) {
        std::string playerName;
        inStream.Read(playerName);
        SendAckPacket(fromAddress);
    } else {
        std::string errorMessage = "Received unexpected packet type from new client with address: " + fromAddress.ToString();
        std::wstring waddr(errorMessage.begin(), errorMessage.end());
        ErrorUtil::ReportError(waddr.c_str());
    }
}

void NetworkManagerServer::HandlePacketFromExistingClient(InputMemoryBitStream& inStream, uint32_t clientId) {
    // implement this
};


void NetworkManagerServer::SendAckPacket(const SocketAddress& toAddress) {
    OutputMemoryBitStream outStream;
    outStream.Write(kAckedCC);
    // TODO: return actual client id
    outStream.Write(static_cast<uint32_t>(0)); // placeholder client ID
    SendPacket(outStream, toAddress);
}