#include <windows.h>
#include <meta.hpp>

#include "function_finder/functions.hpp"
#include "instrumentation/function_replacement.hpp"

/*
  This file is meant to be a user-defined file that tells ada
  what to do with the process.
*/

namespace ada {

int square(int a)
{
  return a * a;
}

FunctionReplacement<decltype(&square)> square_replacement;
int fakesquare(int a)
{
  printf("spoofing real value for %d: %d -> %d\n", a, square_replacement(a), a * 2);
  return a * 2;
}

void configure()
{
  // setup modules to monitor
  // we only care about the main program in this example

  // setup function table
  find_functions((void*)GetModuleHandleA("test_program.exe"));

  int real = square(3);

  square_replacement = *ada::replace_function(square);
  int real2 = square(3);
  *square_replacement.jmp_target = fakesquare;
  int fake = square(3);

  printf("real: %d & %d, fake %d\n", real, real2, fake);
}
} // namespace ada