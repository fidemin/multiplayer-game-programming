// delivery_notification_socket.cpp - Demonstrates DeliveryNotificationManager over a socket pair
//
// Build: g++ -std=c++17 -o delivery_notification_socket.o examples/delivery_notification_socket.cpp
// Usage: ./delivery_notification_socket.o
//
// Two sides (A and B) exchange packets over a socketpair. Each packet carries:
//   - The sender's sequence number (for the receiver to track and ack)
//   - Pending acks for previously received packets (piggybacked)
//   - A simple string payload
//
// DeliveryNotificationManager handles bookkeeping on each side so that
// delivery success/failure callbacks fire when acks arrive or packets go missing.

#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include "../src/DeliveryNotificationManager.cpp"
#include "../src/ErrorUtil.cpp"

class LogTransmissionData : public TransmissionData {
public:
    LogTransmissionData(PacketSequenceNumber inSeqNum, const char* inSide)
        : mSeqNum(inSeqNum), mSide(inSide) {}

    void HandleDeliverySuccess(DeliveryNotificationManager*) override {
        printf("[%s] Packet #%u: DELIVERED\n", mSide, mSeqNum);
    }
    void HandleDeliveryFailure(DeliveryNotificationManager*) override {
        printf("[%s] Packet #%u: DROPPED\n", mSide, mSeqNum);
    }
private:
    PacketSequenceNumber mSeqNum;
    const char* mSide;
};

// Send a packet: sequence number + payload + piggybacked acks
static void SendPacket(int fd, DeliveryNotificationManager& dnm, const char* side, const char* payload) {
    OutputMemoryBitStream outStream;

    InFlightPacket* packet = dnm.WriteSequenceNumber(outStream);
    packet->AddTransmissionData(
        std::make_shared<LogTransmissionData>(packet->GetSequenceNumber(), side));

    std::string msg(payload);
    outStream.Write(msg);
    dnm.WritePendingAcks(outStream);

    uint32_t bytes = outStream.GetByteLength();
    send(fd, outStream.GetBufferPtr(), bytes, 0);
    printf("[%s] Sent seq #%u (%u bytes): \"%s\"\n", side, packet->GetSequenceNumber(), bytes, payload);
}

// Receive a packet: process seq number + read payload + process acks
static bool ReceivePacket(int fd, DeliveryNotificationManager& dnm, const char* side) {
    char buffer[1470];
    int bytes = recv(fd, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return false;

    InputMemoryBitStream inStream(buffer, static_cast<size_t>(bytes) * 8);

    if (!dnm.ProcessSequenceNumber(inStream)) {
        printf("[%s] Received old/duplicate packet — dropping\n", side);
        return false;
    }

    std::string msg;
    inStream.Read(msg);
    printf("[%s] Received: \"%s\"\n", side, msg.c_str());

    dnm.ProcessAcks(inStream);
    return true;
}

int main() {
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        perror("socketpair");
        return 1;
    }

    // Each endpoint manages its own outgoing sequence numbers and incoming acks independently
    DeliveryNotificationManager aDNM;
    DeliveryNotificationManager bDNM;

    printf("=== Round 1: A sends to B ===\n");
    SendPacket(fds[0], aDNM, "A", "Hello from A!");
    ReceivePacket(fds[1], bDNM, "B");

    // B's reply carries piggybacked acks for A's seq #1 — A's delivery callback fires here
    printf("\n=== Round 2: B replies, acking A's packet ===\n");
    SendPacket(fds[1], bDNM, "B", "Hello from B!");
    ReceivePacket(fds[0], aDNM, "A");

    // A sends two more packets before B replies
    printf("\n=== Round 3: A sends two more packets ===\n");
    SendPacket(fds[0], aDNM, "A", "Second message from A");
    SendPacket(fds[0], aDNM, "A", "Third message from A");
    ReceivePacket(fds[1], bDNM, "B");
    ReceivePacket(fds[1], bDNM, "B");

    // B acks both in a single reply — A's callbacks fire for seq #2 and #3
    printf("\n=== Round 4: B acks A's last two packets ===\n");
    SendPacket(fds[1], bDNM, "B", "Acking your last two");
    ReceivePacket(fds[0], aDNM, "A");

    close(fds[0]);
    close(fds[1]);
    return 0;
}
