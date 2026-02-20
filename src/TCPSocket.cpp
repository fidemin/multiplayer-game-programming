
#include <memory>
#include <unistd.h>
#include "SocketAddress.cpp"
#include "SocketUtil.cpp"



class TCPSocket;

using TCPSocketPtr = std::shared_ptr<TCPSocket>;


class TCPSocket {
    public:
        ~TCPSocket();
        int Connect(const SocketAddress& inAddress);
        int Bind(const SocketAddress& inBindAddress);
        int Listen(int inBackLog = 32);
        TCPSocketPtr Accept(SocketAddress& fromAddress);
        int Send(const void* inData, int inLen);
        int Receive(void* inBuffer, int inLen);
    private:
        int mSocket; // socket for connect and listen. accepted sockets are created as new TCPSocket instances.
        TCPSocket(int inSocket) : mSocket(inSocket) {}
};

int TCPSocket::Connect(const SocketAddress& inAddress) {
    int err = connect(mSocket, &inAddress.mSockAddr, inAddress.GetSize());
    if (err < 0) {
        SocketUtil::ReportError(L"TCPSocket::Connect");
        return -SocketUtil::GetLastError();
    }

    return 0;
}

int TCPSocket::Bind(const SocketAddress& inBindAddress) {
    int err = bind(mSocket, &inBindAddress.mSockAddr, inBindAddress.GetSize());
    if (err < 0) {
        SocketUtil::ReportError(L"TCPSocket::Bind");
        return -SocketUtil::GetLastError();
    }

    return 0;
}

int TCPSocket::Listen(int inBackLog) {
    int err = listen(mSocket, inBackLog);
    if (err < 0) {
        SocketUtil::ReportError(L"TCPSocket::Listen");
        return -SocketUtil::GetLastError();
    }

    return 0;
}

TCPSocketPtr TCPSocket::Accept(SocketAddress& fromAddress) {
    socklen_t fromLength = fromAddress.GetSize();
    int sock = accept(mSocket, &fromAddress.mSockAddr, &fromLength);
    if (sock < 0) {
        SocketUtil::ReportError(L"TCPSocket::Accept");
        return nullptr;
    }

    return TCPSocketPtr(new TCPSocket(sock));
}

int TCPSocket::Send(const void* inData, int inLen) {
    int bytesSent = send(mSocket, inData, inLen, 0);
    if (bytesSent < 0) {
        SocketUtil::ReportError(L"TCPSocket::Send");
        return -SocketUtil::GetLastError();
    }

    return bytesSent;
}

int TCPSocket::Receive(void* inBuffer, int inLen) {
    int bytesReceived = recv(mSocket, inBuffer, inLen, 0);
    if (bytesReceived < 0) {
        SocketUtil::ReportError(L"TCPSocket::Receive");
        return -SocketUtil::GetLastError();
    }

    return bytesReceived;
}

TCPSocket::~TCPSocket() {
    close(mSocket);
}