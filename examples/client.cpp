// client.cpp - Simple UDP client using NetworkManagerClient
//
// Build: g++ -std=c++14 -o client.o examples/client.cpp
// Usage: ./client.o [server_ip] [server_port] [player_name]

#include <cstdio>
#include "../src/Client/NetworkManagerClient.cpp"

static const char*    DEFAULT_SERVER_IP   = "127.0.0.1";
static const uint16_t DEFAULT_SERVER_PORT = 8080;
static const uint16_t CLIENT_PORT         = 0; // OS assigns port
static const int      BUFFER_SIZE         = 4096;

int main(int argc, char* argv[]) {
    const char* serverIp   = (argc >= 2) ? argv[1] : DEFAULT_SERVER_IP;
    uint16_t    serverPort = (argc >= 3) ? static_cast<uint16_t>(atoi(argv[2])) : DEFAULT_SERVER_PORT;
    const char* playerName = (argc >= 4) ? argv[3] : "Player1";

    SocketAddress serverAddress = SocketAddress(inet_addr(serverIp), serverPort);

    NetworkManagerClient client(serverAddress);
    if (!client.Initialize(CLIENT_PORT, playerName)) {
        wprintf(L"Failed to initialize client\n");
        return 1;
    }

    wprintf(L"Connecting to %hs:%d as %hs\n", serverIp, serverPort, playerName);

    // Send sync packet to server
    client.Send();

    // Receive loop: wait for ACK from server
    char buffer[BUFFER_SIZE];
    while (true) {
        SocketAddress fromAddress(INADDR_ANY, 0);
        int bytesReceived = client.ReceiveFrom(buffer, BUFFER_SIZE, fromAddress);
        if (bytesReceived > 0) {
            InputMemoryBitStream inStream(buffer, bytesReceived * 8);
            client.Read(inStream);
        }
    }

    return 0;
}
