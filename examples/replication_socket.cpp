// replication_socket.cpp - Demonstrates ReplicationManager send and receive over a socket pair
//
// Build: g++ -std=c++14 -o replication_socket.o examples/replication_socket.cpp
// Usage: ./replication_socket.o
//
// Uses socketpair() to create a connected pair of file descriptors. The sender
// replicates a world of RoboCat objects via ReplicationManager, sends the raw
// bit-stream over the socket, and the receiver reconstructs the world on the
// other end using a separate LinkingContext.

#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include "../src/ReplicationManager.cpp"
#include "../src/ErrorUtil.cpp"

int main() {
    RegisterGameObjectCreationFunctions();

    // Create a connected socket pair: fds[0] = sender, fds[1] = receiver
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        ErrorUtil::ReportError(L"socketpair");
        return 1;
    }

    // --- Sender side ---
    LinkingContext senderContext;
    ReplicationManager senderRM(&senderContext);

    RoboCat cat1, cat2;
    cat1.SetHealth(42);
    cat1.SetMeowCount(7);
    cat1.SetName("Whiskers");

    cat2.SetHealth(5);
    cat2.SetMeowCount(2);
    cat2.SetName("Shadow");

    std::vector<GameObject*> world = { &cat1, &cat2 };

    wprintf(L"[Sender] %hs\n", cat1.GetDescription().c_str());
    wprintf(L"[Sender] %hs\n", cat2.GetDescription().c_str());

    OutputMemoryBitStream outStream;
    senderRM.ReplicateWorld(outStream, world);

    uint32_t byteLength = outStream.GetByteLength();
    if (send(fds[0], outStream.GetBufferPtr(), byteLength, 0) < 0) {
        ErrorUtil::ReportError(L"send");
        return 1;
    }
    wprintf(L"[Sender] Sent %u bytes\n", byteLength);

    // --- Receiver side ---
    LinkingContext receiverContext;
    ReplicationManager receiverRM(&receiverContext);

    const uint32_t bufferSize = 1470;
    char buffer[bufferSize];
    int bytesReceived = recv(fds[1], buffer, sizeof(buffer), 0);
    if (bytesReceived < 0) {
        ErrorUtil::ReportError(L"recv");
        return 1;
    }
    wprintf(L"[Receiver] Received %d bytes\n", bytesReceived);

    InputMemoryBitStream inStream(buffer, bytesReceived * 8);
    receiverRM.ReceiveWorld(inStream);

    for (GameObject* obj : receiverRM.GetObjectsInWorld()) {
        wprintf(L"[Receiver] %hs\n", obj->GetDescription().c_str());
    }

    close(fds[0]);
    close(fds[1]);
    return 0;
}
