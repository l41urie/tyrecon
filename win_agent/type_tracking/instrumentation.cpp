#include "allocation_tracking/allocation_tracker.hpp"
#include "function_finder/functions.hpp"
#include "instrumentation/execution_context.hpp"
#include "type_tracker.hpp"

namespace tyrecon {

void track_type_usage(FunctionExecutionContext &ctx, Function *fn) {
  // Walk all parameters and see if we find anything in our type list
  auto const nparams = fn->get_parameter_count();
  for (auto param_idx = 0; param_idx < nparams; ++param_idx) {
    auto const p = ctx.get_arg(param_idx);

    // Can we find an allocation that corresponds with the type?
    auto alloc = global_allocations.lookup_alloc((void *)p);
    if (alloc.has_value()) {
      // Okay, so this is either the start of an allocation, or it points into
      // one. read the allocation, figure out if we find a dispatch table
      // associated with the value.
      auto v = *(u64 *)p;
      auto type_it =
          std::find_if(global_typelist.begin(), global_typelist.end(),
                       [&](Type const &t) { return t.vtable.start == v; });

      if (type_it != global_typelist.end())
        type_it->mark_used(fn->start, param_idx);
    }
    // TODO: handle module-memory (i.e. pointers into sections of modules)
    // lookup if it's readable via VirtualQuery()?
  }
}
} // namespace tyrecon