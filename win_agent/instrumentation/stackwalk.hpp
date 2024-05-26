#pragma once
#include <vector>
#include <meta.hpp>

struct _CONTEXT;
struct _tagSTACKFRAME64;
typedef _tagSTACKFRAME64 STACKFRAME64;

namespace ada {

// fields from STACKFRAME64
struct Call {
  u64 instruction_ptr;
  u64 ret_addr;
};

// represents a callstack
struct CallStack {
  std::vector<Call> calls;
  FWD_ITERATORS(calls);
};

// utility to capture a callstack
struct StackWalker {
  STACKFRAME64 *frame;
  _CONTEXT *ctx;

  ~StackWalker();

  // Create a new Stackwalker from an existing context, e.g. exception handler
  static StackWalker attach(_CONTEXT *c);

  /* Capture the context from the executing function & thread in-place and
     attach a new StackWalker*/
  static StackWalker attach_from_here();

  bool step();

  CallStack capture();
};
} // namespace ada