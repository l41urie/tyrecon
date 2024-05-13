#include "meta.hpp"
#include <optional>

// windows.h
struct _EXCEPTION_POINTERS;
struct _CONTEXT;

namespace ada {
// wrapper around _CONTEXT to make usage nicer.
struct ExecutionContext {
  _CONTEXT *ctx;

  void *rip() const;
  u64 rcx() const;
  u64 rdx() const;
  u64 r8() const;
  u64 r9() const;
  u64 r10() const; // volatile
  u64 r11() const; // volatile
  u64 rsp() const;
  u64 rbp() const;
};

// Must be set up before the function sets up it's own stackframe
struct FunctionExecutionContext : ExecutionContext {
  u64 get_arg(u64 index) const;
  void *return_address() const;
};
} // namespace ada