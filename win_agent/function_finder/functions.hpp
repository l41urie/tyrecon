#pragma once
#include "../module/module.hpp"
#include "meta.hpp"

namespace ada {

enum class FunctionKind {
  FRAME,
  LEAF,
};

struct Function {
  // Points to the first instruction of the function
  void *start;

  // points behind the last instruction of the function, nullptr if unknown.
  void *end;

  [[nodiscard]] inline bool contains(void *ptr) const {
    return ptr >= start && ptr < end;
  }

  // number of parameters, ~0 if that's unknown.
  u32 parameter_count;

  FunctionKind type;

  // Points behind the first instruction
  void *second_instruction = nullptr;
};

void find_functions(Module const &mod);
void for_each_function(bool (*cb)(ada::Function const &fn));
} // namespace ada