

#include "RoboCat.cpp"
#include <sys/socket.h>

class RoboCatSocket {
    public:
    void static SendRoboCat(int inSocket, const RoboCat* inRoboCat) {
        OutputMemoryStream outStream;
        inRoboCat->Serialize(outStream);

        uint32_t dataLength = outStream.GetLength();
        const char* dataBuffer = outStream.GetBufferPtr();

        int bytesSent = send(inSocket, dataBuffer, dataLength, 0);
        if (bytesSent < 0) {
            ErrorUtil::ReportError(L"RoboCatSocket::SendRoboCat - send length");
            return;
        }
        if (bytesSent != dataLength) {
            wprintf(L"RoboCatSocket::SendRoboCat - sent %d of %d bytes\n", bytesSent, dataLength);
        }
    }

    static void ReceiveRoboCat(int inSocket, RoboCat* outRoboCat) {
        const uint32_t bufferSize = 1470; // typical MTU size for Ethernet - IP header - TCP header
        char buffer[bufferSize];
        int bytesReceived = recv(inSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived < 0) {
            ErrorUtil::ReportError(L"RoboCatSocket::ReceiveRoboCat - recv length");
            return;
        }

        InputMemoryStream inStream(buffer, bytesReceived);
        outRoboCat->Deserialize(inStream);
    }
};