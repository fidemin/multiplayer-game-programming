
#include <sys/socket.h>
#include <vector>
#include "TCPSocket.cpp"

class SocketSelectUtil
{
    public:
        static void FillFdSetFromSocketVector(fd_set& outFdSet, const vector<TCPSocketPtr>& inSockets);
        static void FilterSocketVectorFromFdSet(const vector<TCPSocketPtr>& inSockets, vector<TCPSocketPtr>& outSockets, const fd_set& inFdSet);
        static int Select(
            const vector<TCPSocketPtr>& inReadSet, vector<TCPSocketPtr>& outReadSet, 
            const vector<TCPSocketPtr>& inWriteSet, vector<TCPSocketPtr>& outWriteSet, 
            const vector<TCPSocketPtr>& inExceptSet, vector<TCPSocketPtr>& outExceptSet);
    private:
        static int getMaxSocket(const vector<TCPSocketPtr>& inReadSet) {
            int maxSocket = -1;

            for (const TCPSocketPtr& socketPtr : inReadSet) {
                maxSocket = max(maxSocket, socketPtr->mSocket);
            }
            return maxSocket;
        }
};

void SocketSelectUtil::FillFdSetFromSocketVector(fd_set& outFdSet, const vector<TCPSocketPtr>& inSockets) {

    FD_ZERO(&outFdSet);

    for (const TCPSocketPtr& socketPtr : inSockets) {
        // set the socket number's bit = 1 in the fd_set
        FD_SET(socketPtr->mSocket, &outFdSet);
    }
}


/*
* outSockets will be cleared and filled with the sockets from inSockets that are set in inFdSet
*/
void SocketSelectUtil::FilterSocketVectorFromFdSet(
    const vector<TCPSocketPtr>& inSockets, 
    vector<TCPSocketPtr>& outSockets, 
    const fd_set& inFdSet) 
{
    outSockets.clear();

    for (const TCPSocketPtr& socketPtr : inSockets) {
        if (FD_ISSET(socketPtr->mSocket, &inFdSet)) {
            outSockets.push_back(socketPtr);
        }
    }
}

int SocketSelectUtil::Select(
    const vector<TCPSocketPtr>& inReadSet, vector<TCPSocketPtr>& outReadSet, 
    const vector<TCPSocketPtr>& inWriteSet, vector<TCPSocketPtr>& outWriteSet, 
    const vector<TCPSocketPtr>& inExceptSet, vector<TCPSocketPtr>& outExceptSet) 
{
    fd_set readFdSet, writeFdSet, exceptFdSet;

    FillFdSetFromSocketVector(readFdSet, inReadSet);
    FillFdSetFromSocketVector(writeFdSet, inWriteSet);
    FillFdSetFromSocketVector(exceptFdSet, inExceptSet);

    int maxSocket = -1;
    maxSocket = max(maxSocket, getMaxSocket(inReadSet));
    maxSocket = max(maxSocket, getMaxSocket(inWriteSet));
    maxSocket = max(maxSocket, getMaxSocket(inExceptSet));

    int result = select(maxSocket + 1, &readFdSet, &writeFdSet, &exceptFdSet, nullptr);

    if (result < 0) {
        SocketUtil::ReportError(L"SocketSelectUtil::Select");
        return -SocketUtil::GetLastError();
    }

    if (result == 0) {
        // no sockets were ready, return 0
        outReadSet.clear();
        outWriteSet.clear();
        outExceptSet.clear();
        return result;
    }

    FilterSocketVectorFromFdSet(inReadSet, outReadSet, readFdSet);
    FilterSocketVectorFromFdSet(inWriteSet, outWriteSet, writeFdSet);
    FilterSocketVectorFromFdSet(inExceptSet, outExceptSet, exceptFdSet);

    return result;
}


