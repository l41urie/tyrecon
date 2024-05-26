#include "stackwalk.hpp"
#include <windows.h>
#include <DbgHelp.h>

namespace ada {

StackWalker StackWalker::attach(_CONTEXT *c) {
  StackWalker result;

  result.frame = (STACKFRAME64 *)malloc(sizeof(STACKFRAME64));
  memset(result.frame, 0, sizeof(STACKFRAME64));
  result.frame->AddrPC = {.Offset = c->Rip, .Segment = 0, .Mode = AddrModeFlat};
  result.frame->AddrFrame = {
      .Offset = c->Rsp, .Segment = 0, .Mode = AddrModeFlat};
  result.frame->AddrStack = {
      .Offset = c->Rsp, .Segment = 0, .Mode = AddrModeFlat};

  result.ctx = c;

  return result;
}

StackWalker StackWalker::attach_from_here() {
  static thread_local _CONTEXT ctx;
  RtlCaptureContext(&ctx);
  return attach(&ctx);
}

bool StackWalker::step() {
  return StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(),
                     GetCurrentThread(), frame, ctx, NULL,
                     SymFunctionTableAccess64, SymGetModuleBase64,
                     NULL) == TRUE;
}

CallStack StackWalker::capture() {
  CallStack r;
  do {
    r.calls.emplace_back(Call{frame->AddrPC.Offset, frame->AddrReturn.Offset});
  } while (step());
  return r;
}

StackWalker::~StackWalker() { free(frame); }
} // namespace ada