
#pragma once
#include <string> 
#include "../SocketAddress.cpp"

class ClientProxy
{
public:
    ClientProxy(const SocketAddress& inSocketAddress, const std::string& inPlayerName, uint32_t inPlayerId)
        : mSocketAddress(inSocketAddress), mPlayerName(inPlayerName), mPlayerId(inPlayerId) {}

    uint32_t GetPlayerId() const { return mPlayerId; }
private:
    SocketAddress mSocketAddress;
    std::string mPlayerName;
    uint32_t mPlayerId;
};


