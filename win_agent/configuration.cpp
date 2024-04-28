#include <windows.h>
#include <meta.hpp>

#include "function_finder/functions.hpp"

/*
  This file is meant to be a user-defined file that tells ada
  what to do with the process.
*/

namespace ada {
void configure()
{
  // setup modules to monitor
  // we only care about the main program in this example

  // setup function table
  find_functions((void*)GetModuleHandleA("test_program.exe"));
}
} // namespace ada