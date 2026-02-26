#pragma once
#include <sys/socket.h>
#include "Serializer.cpp"
#include "Deserializer.cpp"
#include "ErrorUtil.cpp"

class ObjectSocketUtil {
    public:
    static int Send(int inSocket, const Serializer* inObject) {
        OutputMemoryBitStream outStream;
        inObject->Serialize(outStream);

        uint32_t dataLength = outStream.GetByteLength();
        const char* dataBuffer = outStream.GetBufferPtr();

        int bytesSent = send(inSocket, dataBuffer, dataLength, 0);
        if (bytesSent < 0) {
            ErrorUtil::ReportError(L"ObjectSocket::Send - send failed");
            return bytesSent;
        }
        if (bytesSent != static_cast<int>(dataLength)) {
            wprintf(L"ObjectSocket::Send - sent %d of %d bytes\n", bytesSent, dataLength);
        }
        return bytesSent;
    }

    static int Receive(int inSocket, Deserializer* outObject) {
        const uint32_t bufferSize = 1470; // typical MTU size for Ethernet - IP header - TCP header
        char buffer[bufferSize];
        int bytesReceived = recv(inSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived < 0) {
            ErrorUtil::ReportError(L"ObjectSocket::Receive - recv failed");
            return bytesReceived;
        }

        InputMemoryBitStream inStream(buffer, bytesReceived * 8);
        outObject->Deserialize(inStream);
        return bytesReceived;
    }
};