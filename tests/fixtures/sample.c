#include <windows.h>
__declspec(dllexport) int Named(int a, int b) { return a + b; }
__declspec(dllexport) long long UnknownShape(void *p, long long a, long long b, long long c, long long d) {
    return (p ? 1 : 0) + a + b + c + d;
}
BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID reserved) { (void)h; (void)reason; (void)reserved; return TRUE; }
