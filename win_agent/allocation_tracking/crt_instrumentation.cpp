#include "allocation_tracker.hpp"
#include "instrumentation/function_replacement.hpp"
#include <windows.h>

namespace ada {
namespace replacements {
inline FunctionReplacement<decltype(&::malloc)> malloc;
inline FunctionReplacement<decltype(&::free)> free;
inline FunctionReplacement<decltype(&::realloc)> realloc;

namespace fn {
void *malloc(size_t size) {
  void *ret = _ReturnAddress();

  auto result = replacements::malloc(size);

  // If the return address points into the agent module
  // skip tracking it as it would lead to infinite recursion.
  // TODO: avoid calling GetModuleHandleExA() all the time here.
  HMODULE mod;
  GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                         GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     (LPCSTR)ret, &mod);
  static auto ourmod = GetModuleHandleA("win_agent.dll");
  if (mod == ourmod)
    return result;

  global_allocations.track_new({
      .memory = {(u64)result, (u64)result + size},
      .allocation_callsite = ret,
      .status = ALLOCATED,
  });

#if 0
  printf("malloc(%llu) -> %p\n", size, result);
  global_allocations.print_list();
#endif

  return result;
}

void free(void *ptr) {
  printf("free(%p)\n", ptr);
  replacements::free(ptr);
  global_allocations.mark_free(ptr);
}

void *realloc(void *block, size_t size) {
  auto result = replacements::realloc(block, size);
  void *ret = _ReturnAddress();
  global_allocations.mark_reallocated(block, result, ret, size);
  return result;
}

} // namespace fn

} // namespace replacements

bool install_crt_replacements() {
#define SETUP_REPLACEMENT(libraryfunc)                                         \
  {                                                                            \
    if (auto replaced = ada::replace_function(::libraryfunc)) {                \
      replacements::libraryfunc = *replaced;                                   \
      *replaced->jmp_target = replacements::fn::libraryfunc;                   \
    } else                                                                     \
      return false;                                                            \
  }

  SETUP_REPLACEMENT(malloc);
  SETUP_REPLACEMENT(realloc);
  SETUP_REPLACEMENT(free);

  return true;
}
} // namespace ada