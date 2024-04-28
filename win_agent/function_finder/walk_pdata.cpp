#include <windows.h>

#include "functions.hpp"
#include <vector>

namespace ada {
Function rebase_rtfunction(RUNTIME_FUNCTION const *const rtfunction,
                           u8 const *const base) {
  // RUNTIME_FUNCTION consists of RVAs, rebase to modulebase
  return Function{
      .start = (void *)(base + rtfunction->BeginAddress),
      .end = (void *)(base + rtfunction->EndAddress),
      .parameter_count = ~0u,
  };
}

void find_functions_pdata(Module const &mod, std::vector<ada::Function> &list) {
  // find .pdata section
  auto it =
      std::find_if(mod.sections.begin(), mod.sections.end(),
                   [](Section const &sec) { return sec.name == ".pdata"; });

  // failed to find section?
  if (it == mod.sections.end())
    return;

  // .pdata is really an array of RUNTIME_FUNCTION, walk it & insert into our list.
  for (RUNTIME_FUNCTION const *cur = (RUNTIME_FUNCTION *)it->begin;
       cur != it->end; cur++) {
    list.emplace_back(rebase_rtfunction(cur, (u8 *)mod.image));
  }

#if 0
  for(auto fn : list)
  {
    printf("%p - %p\n", fn.start, fn.end);
  }
#endif
}
} // namespace ada