#pragma once

#include <unordered_map>

#include "../SocketAddress.cpp"
#include "../NetworkManager.cpp"
#include "../InputMemoryBitStream.cpp"
#include "ClientProxy.cpp"

class NetworkManagerServer : public NetworkManager {
    public:
        void ProcessPacket(InputMemoryBitStream& inStream, SocketAddress& fromAddress);
    private:
        std::unordered_map<SocketAddress, ClientProxy*> mClientAddressToProxyMap;
        uint32_t mNextPlayerId = 1;
        LinkingContext mLinkingContext = LinkingContext();

        void HandlePacketFromNewClient(InputMemoryBitStream& inStream, const SocketAddress& fromAddress);
        void HandlePacketFromExistingClient(InputMemoryBitStream& inStream, ClientProxy* clientProxy);
        void SendAckPacket(const SocketAddress& toAddress, ClientProxy* clientProxy);

        uint32_t GenerateNewPlayerId() {
            return mNextPlayerId++;
        }
};

void NetworkManagerServer::ProcessPacket(InputMemoryBitStream& inStream, SocketAddress& fromAddress) {
    printf("NetworkManagerServer::ProcessPacket - Received packet from %s\n", fromAddress.ToString().c_str());
    auto it = mClientAddressToProxyMap.find(fromAddress);
    if (it == mClientAddressToProxyMap.end()) {
        HandlePacketFromNewClient(inStream, fromAddress);
    } else {
        ClientProxy* clientProxy = it->second;
        HandlePacketFromExistingClient(inStream, clientProxy);
    };
}

void NetworkManagerServer::HandlePacketFromNewClient(InputMemoryBitStream& inStream, const SocketAddress& fromAddress) {
    uint32_t packetType;
    inStream.Read(packetType);

    printf("NetworkManagerServer::HandlePacketFromNewClient - Received packet type %d from new client with address: %s\n", packetType, fromAddress.ToString().c_str());

    if (packetType == kSyncCC) {
        std::string playerName;
        inStream.Read(playerName);

        if (mClientAddressToProxyMap.find(fromAddress) == mClientAddressToProxyMap.end()) {
            uint32_t newPlayerId = GenerateNewPlayerId();
            ClientProxy* newClient = new ClientProxy(fromAddress, playerName, newPlayerId, &mLinkingContext);
            mClientAddressToProxyMap[fromAddress] = newClient;
        } else {
            printf("Client with address %s already exists, ignoring sync packet\n", fromAddress.ToString().c_str());
        }

        ClientProxy* clientProxy = mClientAddressToProxyMap[fromAddress];

        SendAckPacket(fromAddress, clientProxy);

    } else {
        std::string errorMessage = "Received unexpected packet type from new client with address: " + fromAddress.ToString();
        std::wstring waddr(errorMessage.begin(), errorMessage.end());
        ErrorUtil::ReportError(waddr.c_str());
    }
}

void NetworkManagerServer::HandlePacketFromExistingClient(InputMemoryBitStream& inStream, ClientProxy* clientProxy) {
    // implement this
};


void NetworkManagerServer::SendAckPacket(const SocketAddress& toAddress, ClientProxy* clientProxy) {
    OutputMemoryBitStream outStream;
    outStream.Write(kAckedCC);
    outStream.Write(clientProxy->GetPlayerId());
    SendPacket(outStream, toAddress);
}