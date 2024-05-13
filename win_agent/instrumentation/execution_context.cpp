#include "execution_context.hpp"
#include <windows.h>

namespace ada {

void *ExecutionContext::rip() const { return (void *)ctx->Rip; }
u64 ExecutionContext::rcx() const { return ctx->Rcx; }
u64 ExecutionContext::rdx() const { return ctx->Rdx; }
u64 ExecutionContext::r8() const { return ctx->R8; }
u64 ExecutionContext::r9() const { return ctx->R9; }
u64 ExecutionContext::r10() const { return ctx->R10; }
u64 ExecutionContext::r11() const { return ctx->R11; }
u64 ExecutionContext::rsp() const { return ctx->Rsp; }
u64 ExecutionContext::rbp() const { return ctx->Rbp; }

u64 FunctionExecutionContext::get_arg(u64 index) {
  // https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
  // "Integer arguments are passed in registers RCX, RDX, R8, and R9"
  switch (index) {
  case 0:
    return rcx();
  case 1:
    return rdx();
  case 2:
    return r8();
  case 3:
    return r9();
  default:
    break;
  }

  // fifth argument and higher gets passed on the stack
  // rsp + 0 = return address from the previously executed CALL
  // rsp + 8 = fifth argument
  u64 const ptr_add = (index - 3) * 8;
  u64 *arg = (u64*)(rsp() + ptr_add);

  return *arg;
}
} // namespace ada