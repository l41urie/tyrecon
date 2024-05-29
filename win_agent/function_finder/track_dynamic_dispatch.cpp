#include "function_finder/functions.hpp"
#include "instrumentation/execution_context.hpp"

namespace ada {
void track_dynamic_dispatch(FunctionExecutionContext &ctx, Function *fn) {
  if (fn->dynamic_dispatch) {
    /* TODO: change this to capture the callstack instead
        for some reason, from my (limited testing) this seems to crash.
    */
    fn->called_from.emplace(ctx.return_address());
  }
}
} // namespace ada