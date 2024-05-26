#include "functions.hpp"
#include <vector>

namespace ada {

std::vector<ada::Function> g_functions;

void find_functions_pdata(Module const &mod, std::vector<ada::Function> &list);
void find_leaf_functions(ada::Module const &mod,
                         std::vector<ada::Function> &list);
void process_functions(ada::Module const &mod,
                       std::vector<ada::Function> &list);

void find_functions(ada::Module const &mod) {
  // call into every strategy to discover functions

  find_functions_pdata(mod, g_functions);

  find_leaf_functions(mod, g_functions);

  process_functions(mod, g_functions);
}

void for_each_function(bool (*cb)(ada::Function const &fn)) {
  for (auto const &fn : g_functions) {
    if (cb(fn))
      break;
  }
}

ada::Function *find_function(void *start) {
  auto it =
      std::find_if(g_functions.begin(), g_functions.end(),
                   [&start](Function const &f) { return f.start == start; });

  if (it == g_functions.end())
    return nullptr;

  return &(*it);
}

} // namespace ada