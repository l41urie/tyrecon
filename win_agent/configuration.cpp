#include <cstdio>
#include <meta.hpp>
#include <windows.h>

#include "allocation_tracking/allocation_tracker.hpp"
#include "function_finder/functions.hpp"
#include "instrumentation/execution_context.hpp"
#include "instrumentation/fault_inducer.hpp"
#include "instrumentation/function_replacement.hpp"
#include "instrumentation/veh.hpp"

/*
  This file is meant to be a user-defined file that tells ada
  what to do with the process.
*/

namespace ada {

void configure() {
  // setup modules to monitor
  // we only care about the main program in this example

  // setup function table
  find_functions((void *)GetModuleHandleA("test_program.exe"));

  DBG_PAUSE("Attached\n");
  init_veh();

  ada::for_each_function([](ada::Function const &fn) -> bool {
    // install Instrumentation on every function
    if (!global_instrumentations.instrument(fn))
      printf("failed to install instrumentation on %p\n", fn.start);
    return false; // don't exit
  });

  if (!install_crt_replacements()) {
    printf("Failed to install CRT replacements.\n");
    return;
  }

  global_instrumentations.function_instrumentation_callback =
      [](FunctionExecutionContext const &ctx) {
        // find the corresponding function
        auto fn = find_in_list(ctx.rip());
        if (!fn)
          return;

        inspect_function_parameters(ctx, fn);
      };
}

} // namespace ada