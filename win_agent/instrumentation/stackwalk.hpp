#pragma once
#include <vector>
#include <meta.hpp>

struct _CONTEXT;
struct _tagSTACKFRAME64;
typedef _tagSTACKFRAME64 STACKFRAME64;

namespace tyrecon {

// fields from STACKFRAME64
struct Call {
  u64 instruction_ptr;
  u64 ret_addr;

  bool operator==(Call const &other) {
    return instruction_ptr == other.instruction_ptr &&
           ret_addr == other.ret_addr;
  }
};

// represents a callstack
struct CallStack {
  // TODO: filter recursion?
  std::vector<Call> calls;
  FWD_ITERATORS(calls);

  bool operator==(CallStack const &other) {
    if (calls.size() != other.calls.size())
      return false;

    for (auto i = 0; i < calls.size(); ++i)
      if (calls[i] != other.calls[i])
        return false;

    return true;
  }
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
} // namespace tyrecon