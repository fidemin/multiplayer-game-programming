#include <cerrno>
#include <cwchar>
#include <memory>

using namespace std;

class SocketUtil {
public:
    static void ReportError(const wchar_t* inOperationDesc);
    static int GetLastError();

};

void SocketUtil::ReportError(const wchar_t* inOperationDesc) {
    wprintf(L"%s failed with error: %d\n", inOperationDesc, GetLastError());
}

int SocketUtil::GetLastError() {
    return errno;
}
