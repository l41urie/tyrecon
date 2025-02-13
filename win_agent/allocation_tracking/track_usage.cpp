#include "allocation_tracker.hpp"
#include "function_finder/functions.hpp"
#include "instrumentation/execution_context.hpp"

namespace tyrecon {
void check_use_after_free(FunctionExecutionContext const &ctx, Function *fn) {
  auto const nparams = fn->get_parameter_count();

  for (auto i = 0; i < nparams; ++i) {
    auto arg = ctx.get_arg(i);

    auto allocation = global_allocations.lookup_alloc((void *)arg);
    if (allocation.has_value()) {
      if (allocation->status == FREED) {
        printf("Detected Use-after-free in %p with allocation %llX "
               "allocated from %p\n",
               ctx.rip(), allocation->memory.start,
               allocation->allocation_callsite);
      }
    }
  }
}
} // namespace tyrecon