#include "functions.hpp"

namespace ada {
void analyze_functions(Module const &mod, std::vector<Function> &list) {

  // TODO: disassemble every function in the list
  // when hitting `ret` or disassembly fails, fill out `end` field
  // when discovering a `call` figure out where it points, and insert if not
  // present
}
} // namespace ada