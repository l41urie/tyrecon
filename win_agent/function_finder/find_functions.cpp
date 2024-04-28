#include "functions.hpp"
#include <vector>

namespace ada {

std::vector<ada::Function> g_functions;

void find_functions_pdata(Module const &mod, std::vector<ada::Function> &list);
void analyze_functions(Module const &mod, std::vector<Function> &list);

void find_functions(Module const &mod) {
  // call into every strategy to discover functions

  find_functions_pdata(mod, g_functions);

  analyze_functions(mod, g_functions);
}
} // namespace ada