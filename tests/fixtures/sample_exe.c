#include <stdio.h>
__declspec(dllimport) int Named(int, int);
__declspec(dllimport) long long UnknownShape(void *, long long, long long, long long, long long);
int main(void) {
    long long value = UnknownShape((void *)1, 2, 3, 4, 5);
    printf("%d %lld\n", Named(20, 22), value);
    return Named(20, 22) == 42 && value == 15 ? 0 : 1;
}
