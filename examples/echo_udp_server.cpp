// echo_udp_server.cpp - Simple UDP echo server
//
// Build: g++ -std=c++17 -o echo_udp_server.o examples/echo_udp_server.cpp
// Usage:
//   Server: ./echo_udp_server.o [port]
//   Client: nc -u localhost 8080

#include <cstdio>
#include <netinet/in.h>
#include "../src/UDPSocket.cpp"

static const uint16_t DEFAULT_PORT  = 8080;
static const int      BUFFER_SIZE   = 1024;

int main(int argc, char* argv[]) {
    uint16_t port = (argc >= 2) ? static_cast<uint16_t>(atoi(argv[1])) : DEFAULT_PORT;

    UDPSocketPtr socket = UDPSocket::Create();
    if (!socket) {
        wprintf(L"Failed to create UDP socket\n");
        return 1;
    }

    SocketAddress bindAddress(INADDR_ANY, port);
    if (socket->Bind(bindAddress) != 0) {
        wprintf(L"Failed to bind to port %d\n", port);
        return 1;
    }

    wprintf(L"UDP echo server listening on port %d\n", port);

    char buffer[BUFFER_SIZE];
    while (true) {
        SocketAddress fromAddress(INADDR_ANY, 0);
        int bytesReceived = socket->ReceiveFrom(buffer, BUFFER_SIZE, fromAddress);

        if (bytesReceived > 0) {
            printf("Received %d bytes from %s - echoing back\n",
                   bytesReceived, fromAddress.ToString().c_str());
            socket->SendTo(buffer, bytesReceived, fromAddress);
        }
    }

    return 0;
}
