#pragma once
#include "../module/module.hpp"
#include "meta.hpp"
#include <set>

namespace ada {
struct FunctionExecutionContext;

enum class FunctionKind {
  FRAME,
  LEAF,
};

struct Function {
  // TODO: replace this with ada::Block
  // Points to the first instruction of the function
  void *start;

  // points behind the last instruction of the function, nullptr if unknown.
  void *end;

  NODISCARDINL bool contains(void *ptr) const {
    return ptr >= start && ptr < end;
  }

  // number of parameters, ~0 if that's unknown.
  static u32 constexpr PARAMTER_COUNT_UNKNOWN = ~0;
  u32 parameter_count = PARAMTER_COUNT_UNKNOWN;

  u32 get_parameter_count() {
    if (parameter_count == PARAMTER_COUNT_UNKNOWN)
      return 4; // Seems like a reasonable amount to guess, Also matches up with
                // Number of arguments passed in registers in the MSVC ABI
    return parameter_count;
  }

  FunctionKind type;

  // Points behind the first instruction
  void *second_instruction = nullptr;

  // Is this function present in any dispatch table?
  bool dynamic_dispatch = false;

  // if the function is dynamically dispatched, we will save a unqiue set of
  // callstacks associated with this
  // ada::UniqueVec<ada::CallStack> callstacks;
  // stackwalking seems to be broken? fix this.
  std::set<void *> called_from;
};

void find_functions(Module const &mod);

void for_each_function(bool (*cb)(ada::Function const &fn));
ada::Function *find_function(void *start);

void track_dynamic_dispatch(FunctionExecutionContext &ctx, Function *fn);
} // namespace ada