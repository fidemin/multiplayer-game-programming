
#pragma once
#include <cstdint>

#include "ErrorUtil.cpp"
#include "UDPSocket.cpp"
#include "OutputMemoryBitStream.cpp"
#include "SocketAddress.cpp"



class NetworkManager {
    public:
        static const uint32_t kSyncCC = 'SYNC';
        static const uint32_t kAckedCC = 'ACKC';
        static const uint32_t kStateCC = 'STAT';

        bool Initialize(uint16_t inPort);
        void SendPacket(OutputMemoryBitStream& inStream, const SocketAddress& toAddress);
        int ReceiveFrom(void* buffer, int length, SocketAddress& fromAddress);

    private:
        UDPSocketPtr mSocket;
};

bool NetworkManager::Initialize(uint16_t inPort) {
    if (mSocket) {
        ErrorUtil::ReportError(L"NetworkManager::Initialize - Socket already initialized\n");
        return false;
    }

    mSocket = UDPSocket::Create();
    if (!mSocket) {
        ErrorUtil::ReportError(L"NetworkManager::Initialize - Failed to create UDP socket\n");
        return false;
    }
    const SocketAddress bindAddress = SocketAddress(INADDR_ANY, inPort);

    if (mSocket->Bind(bindAddress) < 0) {
        ErrorUtil::ReportError(L"NetworkManager::Initialize - Failed to bind UDP socket\n");
        return false;
    }

    return true;
}

int NetworkManager::ReceiveFrom(void* buffer, int length, SocketAddress& fromAddress) {
    return mSocket->ReceiveFrom(buffer, length, fromAddress);
}

void NetworkManager::SendPacket(OutputMemoryBitStream& inStream, const SocketAddress& toAddress) {
    uint32_t remainingBytes = inStream.GetByteLength();
    const char* buffer = inStream.GetBufferPtr();

    while (remainingBytes > 0) {
        int bytesSent = mSocket->SendTo(buffer, remainingBytes, toAddress);
        if (bytesSent < 0) {
            ErrorUtil::ReportError(L"NetworkManager::SendPacket - Failed to send packet\n");
            throw std::runtime_error("Failed to send packet");
        }
        remainingBytes -= bytesSent;
        buffer += bytesSent;
    }
};

const uint32_t NetworkManager::kSyncCC;
const uint32_t NetworkManager::kAckedCC;
const uint32_t NetworkManager::kStateCC;


