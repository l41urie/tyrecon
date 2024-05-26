#include "allocation_tracker.hpp"
#include "function_finder/functions.hpp"
#include "instrumentation/execution_context.hpp"
#include "instrumentation/stackwalk.hpp"

namespace ada {
void inspect_function_parameters(FunctionExecutionContext const &ctx,
                                 Function *fn) {
  auto params = 4; // fallback to all register params
  if (fn->parameter_count != Function::PARAMTER_COUNT_UNKNOWN)
    params = fn->parameter_count;

  for (auto i = 0; i < params; ++i) {
    auto arg = ctx.get_arg(i);

    auto allocation = global_allocations.lookup_alloc((void *)arg);
    if (allocation.has_value()) {
      if (allocation->status == FREED) {
        printf("Detected Use-after-free in %p with allocation %p "
               "allocated from %p\n",
               ctx.rip(), allocation->ptr, allocation->allocation_callsite);

      }
    }
  }
}
} // namespace ada