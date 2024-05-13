#include "meta.hpp"
#include <optional>

// windows.h
struct _EXCEPTION_POINTERS;
struct _CONTEXT;

namespace ada {
// wrapper around _CONTEXT to make usage nicer.
struct ExecutionContext {
  _CONTEXT *ctx;

  inline void *rip() const;
  inline u64 rcx() const;
  inline u64 rdx() const;
  inline u64 r8() const;
  inline u64 r9() const;
  inline u64 r10() const; // volatile
  inline u64 r11() const; // volatile
  inline u64 rsp() const;
  inline u64 rbp() const;
};

// Must be set up before the function sets up it's own stackframe
struct FunctionExecutionContext : ExecutionContext
{
  u64 get_arg(u64 index);
};
} // namespace ada