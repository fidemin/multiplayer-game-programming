// echo_server.cpp - Sample demonstrating SocketSelectUtil for a multi-client echo server
//
// Build: g++ -std=c++17 -o echo_server.o examples/echo_server.cpp
// Usage:
//   Server: ./echo_server.o [buffer_size]
//   Client: nc localhost 8080
//
// The server uses select() via SocketSelectUtil to handle multiple TCP clients
// concurrently in a single thread. Any message sent by a client is echoed back.

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <netinet/in.h>
#include "../src/SocketSelectUtil.cpp"

static const uint16_t SERVER_PORT = 8080;
static const int      DEFAULT_BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    int bufferSize = (argc >= 2) ? atoi(argv[1]) : DEFAULT_BUFFER_SIZE;
    if (bufferSize <= 0) {
        wprintf(L"Invalid buffer size: %hs\n", argv[1]);
        return 1;
    }
    wprintf(L"Buffer size: %d\n", bufferSize);

    // Create listening socket
    TCPSocketPtr listenSocket = TCPSocket::Create();
    if (!listenSocket) return 1;

    SocketAddress bindAddr(INADDR_ANY, SERVER_PORT);
    if (listenSocket->Bind(bindAddr) < 0) return 1;
    if (listenSocket->Listen()       < 0) return 1;

    wprintf(L"Echo server listening on port %d\n", SERVER_PORT);

    // readSet tracks: the listening socket + all accepted client sockets
    vector<TCPSocketPtr> readSet  = { listenSocket };
    vector<TCPSocketPtr> writeSet;   // unused — pass empty sets
    vector<TCPSocketPtr> exceptSet;  // unused — pass empty sets

    while (true) {
        vector<TCPSocketPtr> readReady, writeReady, exceptReady;

        int ready = SocketSelectUtil::Select(
            &readSet,   &readReady,
            &writeSet,  &writeReady,
            &exceptSet, &exceptReady);

        if (ready < 0) {
            wprintf(L"Select error, exiting.\n");
            break;
        }

        for (const TCPSocketPtr& sock : readReady) {
            if (sock == listenSocket) {
                // New incoming connection
                SocketAddress clientAddr(INADDR_ANY, 0);
                TCPSocketPtr client = listenSocket->Accept(clientAddr);
                if (client) {
                    wprintf(L"New client connected.\n");
                    readSet.push_back(client);
                }
            } else {
                // Existing client has data
                vector<char> buf(bufferSize);
                int bytesReceived = sock->Receive(buf.data(), bufferSize);

                if (bytesReceived <= 0) {
                    // Client disconnected or error
                    wprintf(L"Client disconnected.\n");

                    // Remove the socket from readSet
                    readSet.erase(
                        remove(readSet.begin(), readSet.end(), sock),
                        readSet.end());
                } else {
                    // print received message
                    wprintf(L"Received %d bytes: %.*hs\n", bytesReceived, bytesReceived, buf.data());
                    // Echo the received message back to the client
                    sock->Send(buf.data(), bytesReceived);
                }
            }
        }
    }

    return 0;
}
