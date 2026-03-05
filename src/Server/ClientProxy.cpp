
#pragma once
#include <string> 
#include "../SocketAddress.cpp"
#include "ReplicationManagerServer.cpp"
#include "../DeliveryNotificationManager.cpp"

class ClientProxy
{
public:
    ClientProxy(
        const SocketAddress& inSocketAddress, 
        const std::string& inPlayerName, 
        uint32_t inPlayerId, 
        LinkingContext* inLinkingContext): 
            mSocketAddress(inSocketAddress), 
            mPlayerName(inPlayerName), 
            mPlayerId(inPlayerId), 
            mReplicationManagerServer(inLinkingContext) {
        }

    uint32_t GetPlayerId() const { return mPlayerId; }

    void QueueCreate(uint32_t networkId) {
        mReplicationManagerServer.AddReplicationCommand(networkId, RA_Create);
    }

    void WriteReplicationData(OutputMemoryBitStream& outStream) {
        mDeliveryNotificationManager.WriteSequenceNumber(outStream);
        mDeliveryNotificationManager.WritePendingAcks(outStream);
        mReplicationManagerServer.WritePendingCommands(outStream);
    }

private:
    SocketAddress mSocketAddress;
    std::string mPlayerName;
    uint32_t mPlayerId;
    ReplicationManagerServer mReplicationManagerServer;
    DeliveryNotificationManager mDeliveryNotificationManager;
};


