#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include "SocketAddress.cpp"
#include "SocketUtil.cpp"

using namespace std;

class UDPSocket {
public:
    static UDPSocketPtr Create() {
        int socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (socket < 0) {
            SocketUtil::ReportError(L"UDPSocket::Create");
            return nullptr;
        }

        return make_shared<UDPSocket>(socket);
    }
    ~UDPSocket();
    int Bind(const SocketAddress& bindAddress);
    int SendTo(const void* buffer, int length, const SocketAddress& toAddress);
    int ReceiveFrom(void* buffer, int length, SocketAddress& fromAddress);
    int SetNonBlockingMode(bool shouldBeNonBlocking);
private:
    UDPSocket(int socket) : mSocket(socket) {}
    int mSocket;
};

using UDPSocketPtr = shared_ptr<UDPSocket>;

int UDPSocket::Bind(const SocketAddress& bindAddress) {
    int err = bind(mSocket, &bindAddress.mSockAddr, bindAddress.GetSize());

    if (err == 0) {
        return 0;
    }

    SocketUtil::ReportError(L"UDPSocket::Bind");
    return SocketUtil::GetLastError();
}

int UDPSocket::SendTo(const void* buffer, int length, const SocketAddress& toAddress) {
    int bytesSent = sendto(
        mSocket, 
        buffer, 
        length, 
        0, 
        &toAddress.mSockAddr, 
        toAddress.GetSize()
    );

    if (bytesSent >= 0) {
        return bytesSent;
    }

    SocketUtil::ReportError(L"UDPSocket::SendTo");
    // return negative value to indicate failure, and the value is the error code
    return -SocketUtil::GetLastError();
}

int UDPSocket::ReceiveFrom(void* buffer, int length, SocketAddress& fromAddress) {
    socklen_t fromLength = fromAddress.GetSize();
    int bytesReceived = recvfrom(mSocket, buffer, length, 0, &fromAddress.mSockAddr, &fromLength);

    if (bytesReceived >= 0) {
        return bytesReceived;
    }

    SocketUtil::ReportError(L"UDPSocket::ReceiveFrom");
    // return negative value to indicate failure, and the value is the error code
    return -SocketUtil::GetLastError();
}

int UDPSocket::SetNonBlockingMode(bool shouldBeNonBlocking) {
    int nonBlockingFlag = shouldBeNonBlocking ? 1 : 0;
    int flags = fcntl(mSocket, F_GETFL, 0);

    if (flags < 0) {
        SocketUtil::ReportError(L"UDPSocket::SetNonBlockingMode - F_GETFL");
        return SocketUtil::GetLastError();
    }

    flags = shouldBeNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    int err = fcntl(mSocket, F_SETFL, flags);

    if (err == 0) {
        return 0;
    }

    SocketUtil::ReportError(L"UDPSocket::SetNonBlockingMode");
    return SocketUtil::GetLastError();
}

UDPSocket::~UDPSocket() {
    close(mSocket);
}

