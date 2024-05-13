#include <meta.hpp>
#include <windows.h>

#include "function_finder/functions.hpp"
#include "instrumentation/fault_inducer.hpp"
#include "instrumentation/function_replacement.hpp"
#include "instrumentation/veh.hpp"
#include "instrumentation/execution_context.hpp"

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

  DBG_PAUSE("attach");
  init_veh();

  ada::for_each_function([](ada::Function const &fn) -> bool {
    // install Instrumentation on every function
    if (!global_instrumentations.instrument(fn)) {
      printf("failed to install instrumentation on %p\n", fn.start);
    } else {
      printf("installed instrumentation on %p\n", fn.start);
    }
    return false; // don't exit
  });

  global_instrumentations.function_instrumentation_callback = [](FunctionExecutionContext const &ctx)
  {
    printf("Hit function at %p returns to %p\n", ctx.rip(), ctx.return_address());
    for(auto i = 0; i < 8; ++i)
      printf("\t[%d]: %llx\n", i, ctx.get_arg(i) & 0xFFFFFFFF);
  };
}
} // namespace ada