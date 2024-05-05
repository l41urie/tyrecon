#pragma once
#include <optional>

namespace ada {
namespace detail {
bool replace_function(void *fn, void ***jmp_to, void **copied);
}

template <typename Fn = void (*)()> struct FunctionReplacement {
  Fn function;    // pointer of the function that was replaced
  Fn *jmp_target; // points to the mem that points to the replacement
  Fn copied;      /* pointer to a function that calls into |function|
                      without calling the replacement */

  operator Fn() { return copied; }
};

template <typename Fn>
std::optional<FunctionReplacement<Fn>> replace_function(Fn fn) {
  void *copied;
  void **jmp_to;
  if (!detail::replace_function((void*)fn, &jmp_to, &copied))
    return std::nullopt;

  return FunctionReplacement<Fn>{
      .function = (Fn)fn,
      .jmp_target = (Fn*)jmp_to,
      .copied = (Fn)copied,
  };
}

} // namespace ada