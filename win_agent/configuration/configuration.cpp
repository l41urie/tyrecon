#include <cstdio>
#include <meta.hpp>
#include <windows.h>

#include "allocation_tracking/allocation_tracker.hpp"
#include "function_finder/functions.hpp"
#include "instrumentation/execution_context.hpp"
#include "instrumentation/fault_inducer.hpp"
#include "type_tracking/type_tracker.hpp"
#include "instrumentation/veh.hpp"
#include "rtti/rtti.hpp"

/*
  This file is meant to be a user-defined file that specify
  what to do with the process.
*/

namespace tyrecon {

void configure() {
  // setup modules to monitor
  // we only care about the main program in this example

  // setup function table
  tyrecon::Module mod((void *)GetModuleHandleA("test_program.exe"));
  find_functions(mod);

  rtti::find_rtti(mod);

  DBG_PAUSE("Attached\n");
  init_veh();

  if (!install_crt_replacements()) {
    printf("Failed to install CRT replacements.\n");
    return;
  }

  tyrecon::for_each_function([](tyrecon::Function const &fn) -> bool {
    // install Instrumentation on every function
    if (!global_instrumentations.instrument(fn)) {
#if 0
      printf("Failed to install instrumentation on %p\n", fn.start);
#endif
    }
    return false; // don't exit
  });

  global_instrumentations.function_instrumentation_callback =
      [](FunctionExecutionContext &ctx) {
        // find the corresponding function
        auto fn = find_function(ctx.rip());
        if (!fn)
          return;

        check_use_after_free(ctx, fn);
        track_dynamic_dispatch(ctx, fn);
        track_type_usage(ctx, fn);
      };
}

} // namespace tyrecon