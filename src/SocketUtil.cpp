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
    // this code only prints the first character of the wide string, which is not correct. We should use %ls to print the entire wide string.
    // wprintf(L"%s failed with error: %d\n", inOperationDesc, GetLastError());
    printf("%ls failed with error: %d\n", inOperationDesc, GetLastError());
}

int SocketUtil::GetLastError() {
    return errno;
}
