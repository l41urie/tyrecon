#include "meta.hpp"
#include <optional>
#include <vector>

// windows.h
struct _EXCEPTION_POINTERS;

namespace ada {
struct Violation;
struct Function;
struct FunctionExecutionContext;

using FunctionInstrumentationCallbackFn = void(FunctionExecutionContext &ctx);

// Represents a software breakpoint using int3
struct CodeInstrumentation {
  // instruction to be breakpointed
  void *instruction;
  // original byte that's been replaced with 0xCC
  u8 original_byte;
  // original protection flags returned by `VirtualQuery()`
  u32 original_protection;

  // attempts to set up a new CodeInstruction at |instr|
  static std::optional<CodeInstrumentation> install(void *instr);

  // writes INT3 to |instruction|
  void enable();
  // restores |instruction| to |original_byte|
  void disable();
};

struct InstrumentationHandler {
  std::vector<CodeInstrumentation> instrumented_instructions;
  FunctionInstrumentationCallbackFn *function_instrumentation_callback =
      [](FunctionExecutionContext &ctx) {};
  bool instrument(ada::Function const &fn);
  CodeInstrumentation *find_code_instrumentation(Violation const &v,
                                                 _EXCEPTION_POINTERS *eh);
};

inline InstrumentationHandler global_instrumentations;
} // namespace ada
