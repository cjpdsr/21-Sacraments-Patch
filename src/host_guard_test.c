#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
typedef LONG (*StatusFn)(void);
int main(void) {
    HMODULE mod=LoadLibraryW(L".\\silenttraverse.asi");
    if(!mod){printf("load failed %lu\n",GetLastError());return 1;}
    StatusFn status=(StatusFn)GetProcAddress(mod,"SilentTraverseStatus");
    if(!status){puts("status export missing");return 2;}
    for(int i=0;i<100 && status()==1;i++)Sleep(50);
    if(status()!=2){printf("unexpected status %ld\n",status());return 3;}
    if(GetFileAttributesW(L"silenttraverse.ini")!=INVALID_FILE_ATTRIBUTES ||
       GetFileAttributesW(L"silenttraverse.log")!=INVALID_FILE_ATTRIBUTES){puts("unexpected config/log side effect");return 4;}
    puts("PASS: unsupported host rejected before config I/O and hook initialization");
    return 0;
}
