// client.cpp - Simple UDP client using NetworkManagerClient
//
// Build: g++ -std=c++14 -o client.o examples/client.cpp
// Usage: ./client.o [server_ip] [server_port] [player_name]

#include <cstdio>
#include "../src/SocketAddressFactory.cpp"
#include "../src/Client/NetworkManagerClient.cpp"

static const char*    DEFAULT_SERVER_IP   = "127.0.0.1";
static const uint16_t DEFAULT_SERVER_PORT = 8080;
static const uint16_t CLIENT_PORT         = 0; // OS assigns port
static const int      BUFFER_SIZE         = 4096;

int main(int argc, char* argv[]) {
    const char* serverIp   = (argc >= 2) ? argv[1] : DEFAULT_SERVER_IP;
    uint16_t    serverPort = (argc >= 3) ? static_cast<uint16_t>(atoi(argv[2])) : DEFAULT_SERVER_PORT;
    const char* playerName = (argc >= 4) ? argv[3] : "Player1";

    SocketAddressPtr serverAddressPtr = SocketAddressFactory::CreateIPv4FromString(
        string(serverIp) + ":" + to_string(serverPort));

    if (!serverAddressPtr) {
        printf("Failed to resolve server address: %s:%d\n", serverIp, serverPort);
        return 1;
    }
    SocketAddress serverAddress = *serverAddressPtr;

    NetworkManagerClient client(serverAddress);
    if (!client.Initialize(CLIENT_PORT, playerName)) {
        printf("Failed to initialize client\n");
        return 1;
    }

    printf("Connecting to %s:%d as %s\n", serverIp, serverPort, playerName);

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
        } else {
            printf("Failed to receive packet\n");
        }
    }

    return 0;
}
