// delivery_notification_socket.cpp - Demonstrates DeliveryNotificationManager over UDP sockets
//
// Build: g++ -std=c++17 -o delivery_notification_socket.o examples/delivery_notification_socket.cpp
// Usage: ./delivery_notification_socket.o
//
// Two UDPSockets (A on port 9001, B on port 9000) exchange packets on localhost.
// Each packet carries:
//   - The sender's sequence number (for the receiver to track and ack)
//   - Pending acks for previously received packets (piggybacked)
//   - A simple string payload
//
// DeliveryNotificationManager handles bookkeeping on each side so that
// delivery success/failure callbacks fire when acks arrive or packets time out.

#include <cstdio>
#include <thread>
#include <chrono>
#include "../src/DeliveryNotificationManager.cpp"
#include "../src/UDPSocket.cpp"
#include "../src/SocketAddressFactory.cpp"
#include "../src/ErrorUtil.cpp"

class LogTransmissionData : public TransmissionData {
public:
    LogTransmissionData(PacketSequenceNumber inSeqNum, const char* inSide)
        : mSeqNum(inSeqNum), mSide(inSide) {}

    void HandleDeliverySuccess(DeliveryNotificationManager*) override {
        printf("[%s] Packet #%u: DELIVERED\n", mSide, mSeqNum);
    }
    void HandleDeliveryFailure(DeliveryNotificationManager*) override {
        printf("[%s] Packet #%u: TIMED OUT\n", mSide, mSeqNum);
    }
private:
    PacketSequenceNumber mSeqNum;
    const char* mSide;
};

// Send a packet: sequence number + payload + piggybacked acks
static void SendPacket(UDPSocketPtr& sock, const SocketAddress& toAddr,
                       DeliveryNotificationManager& dnm, const char* side, const char* payload) {
    OutputMemoryBitStream outStream;

    InFlightPacket* packet = dnm.WriteSequenceNumber(outStream);
    packet->AddTransmissionData(
        std::make_shared<LogTransmissionData>(packet->GetSequenceNumber(), side));

    std::string msg(payload);
    outStream.Write(msg);
    dnm.WritePendingAcks(outStream);

    sock->SendTo(outStream.GetBufferPtr(), outStream.GetByteLength(), toAddr);
    printf("[%s] Sent seq #%u (%u bytes): \"%s\"\n",
           side, packet->GetSequenceNumber(), outStream.GetByteLength(), payload);
}

// Receive a packet: process seq number + read payload + process acks
static bool ReceivePacket(UDPSocketPtr& sock, DeliveryNotificationManager& dnm, const char* side) {
    char buffer[1470];
    SocketAddress fromAddr(INADDR_ANY, 0);
    int bytes = sock->ReceiveFrom(buffer, sizeof(buffer), fromAddr);
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
    auto addrA = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:9001");
    auto addrB = SocketAddressFactory::CreateIPv4FromString("127.0.0.1:9000");

    auto sockA = UDPSocket::Create();
    auto sockB = UDPSocket::Create();

    sockA->Bind(*addrA);
    sockB->Bind(*addrB);

    DeliveryNotificationManager aDNM;
    DeliveryNotificationManager bDNM;

    printf("=== Round 1: A sends to B ===\n");
    SendPacket(sockA, *addrB, aDNM, "A", "Hello from A!");
    ReceivePacket(sockB, bDNM, "B");

    // B's reply piggybacks an ack for A's seq #1 — A's delivery callback fires here
    printf("\n=== Round 2: B replies, acking A's packet ===\n");
    SendPacket(sockB, *addrA, bDNM, "B", "Hello from B!");
    ReceivePacket(sockA, aDNM, "A");

    // A sends two packets back-to-back; each UDP datagram is independent
    printf("\n=== Round 3: A sends two packets ===\n");
    SendPacket(sockA, *addrB, aDNM, "A", "Second message from A");
    SendPacket(sockA, *addrB, aDNM, "A", "Third message from A");
    ReceivePacket(sockB, bDNM, "B");
    ReceivePacket(sockB, bDNM, "B");

    // B acks both in one reply — A's callbacks fire for seq #2 and #3
    printf("\n=== Round 4: B acks A's last two packets ===\n");
    SendPacket(sockB, *addrA, bDNM, "B", "Acking your last two");
    ReceivePacket(sockA, aDNM, "A");

    // A sends a packet but B never replies — timeout fires after kAckTimeoutMs (1s)
    printf("\n=== Round 5: A sends, B never acks (timeout demo) ===\n");
    SendPacket(sockA, *addrB, aDNM, "A", "This packet will time out");
    ReceivePacket(sockB, bDNM, "B");  // B receives but sends no reply

    printf("[A] Waiting 1.1s for timeout...\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    aDNM.ProcessTimeoutPackets();  // fires HandleDeliveryFailure for the unacked packet

    return 0;
}
