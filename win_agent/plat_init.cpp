#include <cstdio>
#include <windows.h>

namespace ada{
void configure();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpReserved)
{
    if(dwReason == DLL_PROCESS_ATTACH)
    {
        // Dispatch to init function
        // MessageBoxA(0, "Hi", "hi", 0);
        ada::configure();
    }
    return TRUE;
}