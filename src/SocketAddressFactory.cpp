#include "SocketAddress.cpp"
#include <string>
#include <netdb.h>

using namespace std;

class SocketAddressFactory {
    static SocketAddressPtr CreateIPv4FromString(const string& inString) {
        auto pos = inString.find(':');
        string host, port;

        if (pos != string::npos) {
            host = inString.substr(0, pos);
            port = inString.substr(pos + 1);
        } else {
            host = inString;
            port = "0";
        }

        addrinfo hint;
        memset(&hint, 0, sizeof(hint));
        hint.ai_family = AF_INET; // IPv4

        addrinfo* result = nullptr;

        int error = getaddrinfo(host.c_str(), port.c_str(), &hint, &result);

        if (error != 0) {
            if (result) {
                freeaddrinfo(result);
            }
            return nullptr;
        }

        // find the first result with an valid address
        while (!result->ai_addr && result->ai_next) {
            result = result->ai_next;
        }

        // If we didn't find an address from DNS
        if (!result->ai_addr) {
            freeaddrinfo(result);
            return nullptr;
        }

        auto toRet = make_shared<SocketAddress>(*result->ai_addr);
        freeaddrinfo(result);
        return toRet;
    }

};