#include "veh.hpp"
#include <windows.h>

namespace ada {

LONG WINAPI veh_handler(_EXCEPTION_POINTERS *eh) {
  switch (eh->ExceptionRecord->ExceptionCode) {
  case EXCEPTION_ACCESS_VIOLATION: {
    // https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-exception_record
    void *data_accessed = (void *)eh->ExceptionRecord->ExceptionInformation[1];
    auto type = (ViolationType)eh->ExceptionRecord->ExceptionInformation[0];
    
    break;
  }

  case EXCEPTION_BREAKPOINT: {
    break;
  }

  case EXCEPTION_SINGLE_STEP: {
    break;
  }

  default:
    break;
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

void init_veh() { AddVectoredExceptionHandler(1, veh_handler); }
} // namespace ada