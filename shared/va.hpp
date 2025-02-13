#include "rva.hpp"
#include <string>

namespace tyrecon {
struct Module;

enum VAType {
  REL_MODULE,
  ABSOLUTE,
};

template <typename T> struct NamedVa {
  // in case of ABSOLUTE, this is an absolute address.
  Rva64<T> rva;

  // ABSOLUTE -> |name|
  // REL_MODULE -> |mod|
  VAType type;
  union {
    tyrecon::Module *mod;
    std::string name;
  };

  std::string format()
  {
    
  }
};
} // namespace tyrecon