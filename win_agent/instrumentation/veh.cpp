#include "veh.hpp"
#include <windows.h>

namespace ada {

LONG WINAPI veh_handler(_EXCEPTION_POINTERS *eh)
{
  // eh->ExceptionRecord.
  return EXCEPTION_CONTINUE_SEARCH;
}

void init_veh() { AddVectoredExceptionHandler(1, veh_handler); }
} // namespace ada