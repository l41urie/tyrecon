#include "rtti.hpp"
#include <meta.hpp>
#include <windows.h>
#include <DbgHelp.h>

namespace ada::rtti {

std::optional<std::string> TypeDescriptor::demangle() const {
  char buffer[1024];

  if (auto symbol_size = UnDecorateSymbolName(
          decorated_name + 1, buffer, 1024,
          UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY | UNDNAME_NO_ARGUMENTS |
              UNDNAME_NO_MS_KEYWORDS))
    return std::string(buffer, symbol_size);

  return std::nullopt;
}
} // namespace ada::rtti