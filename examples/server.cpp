// server.cpp - Simple UDP server using NetworkManagerServer
//
// Build: g++ -std=c++14 -o server.o examples/server.cpp
// Usage: ./server.o [port]

#include <cstdio>
#include "../src/Server/NetworkManagerServer.cpp"
#include "../src/RoboCat.cpp"

static const uint16_t DEFAULT_PORT = 8080;
static const int      BUFFER_SIZE  = 4096;

int main(int argc, char* argv[]) {
    uint16_t port = (argc >= 2) ? static_cast<uint16_t>(atoi(argv[1])) : DEFAULT_PORT;

    NetworkManagerServer networkManager;
    if (!networkManager.Initialize(port)) {
        wprintf(L"Failed to initialize server on port %d\n", port);
        return 1;
    }
    wprintf(L"Server listening on port %d\n", port);

    RoboCat* cat1 = new RoboCat();
    cat1->SetName("Whiskers");
    cat1->SetHealth(10);
    networkManager.AddWorldObject(cat1);

    RoboCat* cat2 = new RoboCat();
    cat2->SetName("Mittens");
    cat2->SetHealth(5);
    networkManager.AddWorldObject(cat2);

    char buffer[BUFFER_SIZE];
    while (true) {
        SocketAddress fromAddress(INADDR_ANY, 0);
        int bytesReceived = networkManager.ReceiveFrom(buffer, BUFFER_SIZE, fromAddress);
        if (bytesReceived > 0) {
            printf("Received %d bytes from %s\n", bytesReceived, fromAddress.ToString().c_str());
            InputMemoryBitStream inStream(buffer, bytesReceived * 8);
            networkManager.ProcessPacket(inStream, fromAddress);
            networkManager.SendReplicationDataToAllClients();
        }
    }

    return 0;
}
