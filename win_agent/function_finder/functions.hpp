#pragma once
#include "meta.hpp"
#include "../module/module.hpp"

namespace ada {
struct Function {
  // Points to the first instruction of the function
  void *start;

  // points behind the last instruction of the function, nullptr if unknown.
  void *end;

  // number of parameters, ~0 if that's unknown.
  u32 parameter_count;
};

void find_functions(Module const &mod);
} // namespace ada