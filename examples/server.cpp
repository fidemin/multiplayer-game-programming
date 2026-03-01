// server.cpp - Simple UDP server using NetworkManagerServer
//
// Build: g++ -std=c++14 -o server.o examples/server.cpp
// Usage: ./server.o [port]

#include <cstdio>
#include "../src/Server/NetworkManagerServer.cpp"

static const uint16_t DEFAULT_PORT = 8080;
static const int      BUFFER_SIZE  = 4096;

int main(int argc, char* argv[]) {
    uint16_t port = (argc >= 2) ? static_cast<uint16_t>(atoi(argv[1])) : DEFAULT_PORT;

    NetworkManagerServer server;
    if (!server.Initialize(port)) {
        wprintf(L"Failed to initialize server on port %d\n", port);
        return 1;
    }
    wprintf(L"Server listening on port %d\n", port);

    char buffer[BUFFER_SIZE];
    while (true) {
        SocketAddress fromAddress(INADDR_ANY, 0);
        int bytesReceived = server.ReceiveFrom(buffer, BUFFER_SIZE, fromAddress);
        if (bytesReceived > 0) {
            InputMemoryBitStream inStream(buffer, bytesReceived * 8);
            server.ProcessPacket(inStream, fromAddress);
        }
    }

    return 0;
}
