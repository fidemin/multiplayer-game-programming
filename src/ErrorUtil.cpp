#pragma once
#include <cerrno>
#include <cwchar>

using namespace std;

class ErrorUtil {
public:
    static void ReportError(const wchar_t* inOperationDesc);
    static int GetLastError();

};

void ErrorUtil::ReportError(const wchar_t* inOperationDesc) {
    wprintf(L"%s failed with error: %d\n", inOperationDesc, GetLastError());
}

int ErrorUtil::GetLastError() {
    return errno;
}
