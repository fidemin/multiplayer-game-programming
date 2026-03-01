#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

class SocketAddress {
    public:
        SocketAddress(uint32_t inAddress, uint16_t inPort) {
            memset(&mSockAddr, 0, sizeof(mSockAddr)); // initialize
            GetAsSockAddrIn()->sin_family = AF_INET;
            GetAsSockAddrIn()->sin_addr.s_addr = htonl(inAddress);
            GetAsSockAddrIn()->sin_port = htons(inPort);
        }

        SocketAddress(const sockaddr& inSockAddr) {
            memcpy(&mSockAddr, &inSockAddr, sizeof(mSockAddr));
        }

        size_t GetSize() const {
            return sizeof(mSockAddr);
        }

        string ToString() const {
            char buffer[INET_ADDRSTRLEN];
            const sockaddr_in* addr = reinterpret_cast<const sockaddr_in*>(&mSockAddr);
            inet_ntop(AF_INET, &addr->sin_addr, buffer, sizeof(buffer));
            uint16_t port = ntohs(addr->sin_port);
            return string(buffer) + ":" + to_string(port);
        }

    private:
        friend class UDPSocket;
        friend class TCPSocket;
        sockaddr mSockAddr;

        sockaddr_in* GetAsSockAddrIn() {
            return reinterpret_cast<sockaddr_in*>(&mSockAddr);
        }

};

using SocketAddressPtr = shared_ptr<SocketAddress>;