// robocat_socket.cpp - Demonstrates RoboCatSocket send and receive over a socket pair
//
// Build: g++ -std=c++14 -o robocat_socket.o examples/robocat_socket.cpp
// Usage: ./robocat_socket.o
//
// Uses socketpair() to create a connected pair of file descriptors that act as
// a local network connection. The sender serializes a RoboCat and sends it;
// the receiver reads from the other end and deserializes it.

#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include "../src/RoboCatSocket.cpp"

int main() {
    // Create a connected socket pair: fds[0] = sender, fds[1] = receiver
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        ErrorUtil::ReportError(L"socketpair");
        return 1;
    }

    // --- Sender side ---
    RoboCat sender;
    sender.SetHealth(42);
    sender.SetMeowCount(7);
    sender.SetName("Whiskers");

    wprintf(L"[Sender]   %hs\n", sender.GetDescription().c_str());

    RoboCatSocket::SendRoboCat(fds[0], &sender);
    wprintf(L"[Sender]   RoboCat sent.\n");

    // --- Receiver side ---
    RoboCat receiver;
    RoboCatSocket::ReceiveRoboCat(fds[1], &receiver);

    wprintf(L"[Receiver] %hs\n", receiver.GetDescription().c_str());

    close(fds[0]);
    close(fds[1]);
    return 0;
}
