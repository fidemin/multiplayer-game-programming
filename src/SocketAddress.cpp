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

        bool operator==( const SocketAddress& inOther ) const
        {
            // Two SocketAddress instances are considered equal if they have the same address family, IP address, and port number.
            return ( mSockAddr.sa_family == AF_INET &&
                    ( GetIP4Ref() == inOther.GetIP4Ref() ) &&
                    GetAsSockAddrIn()->sin_port == inOther.GetAsSockAddrIn()->sin_port );
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
        friend struct std::hash<SocketAddress>;
        sockaddr mSockAddr;

        sockaddr_in* GetAsSockAddrIn() {
            return reinterpret_cast<sockaddr_in*>(&mSockAddr);
        }

        const sockaddr_in* GetAsSockAddrIn() const {
            return reinterpret_cast<const sockaddr_in*>(&mSockAddr);
        }

        uint32_t GetIP4Ref() const {
            return GetAsSockAddrIn()->sin_addr.s_addr;
        }

};

using SocketAddressPtr = shared_ptr<SocketAddress>;

namespace std {
    template<>
    struct hash<SocketAddress> {
        size_t operator()(const SocketAddress& addr) const {
            const sockaddr_in* in = reinterpret_cast<const sockaddr_in*>(&addr.mSockAddr);
            size_t h1 = hash<uint32_t>()(in->sin_addr.s_addr);
            size_t h2 = hash<uint16_t>()(in->sin_port);
            return h1 ^ (h2 << 16);
        }
    };
}