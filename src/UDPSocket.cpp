#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include "SocketAddress.cpp"

using namespace std;

class UDPSocket {
public:
    ~UDPSocket();
    int Bind(const SocketAddress& bindAddress);
    int SendTo(const void* buffer, int length, const SocketAddress& toAddress);
    int ReceiveFrom(void* buffer, int length, SocketAddress& fromAddress);
private:
    friend class SocketUtil;
    UDPSocket(int socket) : mSocket(socket) {}
    int mSocket;
};

using UDPSocketPtr = shared_ptr<UDPSocket>;
