#include <cstdint>
#include <cstring>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>

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
    private:
        sockaddr mSockAddr;

        sockaddr_in* GetAsSockAddrIn() {
            return reinterpret_cast<sockaddr_in*>(&mSockAddr);
        }
};

using SocketAddressPtr = shared_ptr<SocketAddress>;