// robocat_serialization.cpp - Demonstrates RoboCat serialization with memory streams
//
// Build: g++ -std=c++14 -o robocat_serialization.o examples/robocat_serialization.cpp
// Usage: ./robocat_serialization.o
//
// Shows how a sender writes a RoboCat into an OutputMemoryStream and a receiver
// reconstructs it from an InputMemoryStream — simulating what would happen when
// sending game object state over the network.

#include <cstdio>
#include <cstring>
#include "../src/RoboCat.cpp"

int main() {
    // --- Sender side ---
    RoboCat sender;
    sender.SetHealth(42);
    sender.SetMeowCount(7);
    sender.SetName("Whiskers");

    wprintf(L"[Sender] %hs\n", sender.GetDescription().c_str());

    OutputMemoryStream outStream;
    sender.Write(outStream);

    wprintf(L"[Sender] Serialized %u bytes\n", outStream.GetLength());

    // --- Receiver side ---
    InputMemoryStream inStream(outStream.GetBufferPtr(), outStream.GetLength());

    RoboCat receiver;
    receiver.Read(inStream);

    wprintf(L"[Receiver] %hs\n", receiver.GetDescription().c_str());

    return 0;
}
