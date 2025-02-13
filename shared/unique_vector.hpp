#pragma once
#include "meta.hpp"
#include <vector>

namespace tyrecon {
// Don't use this if std::set is an option
// in our case it isn't, because we cannot sort certain things
template <typename T> struct UniqueVec {
  std::vector<T> container;
  FWD_ITERATORS_CONST(container);

  bool insert(T const &el) {
    auto it = std::find(container.begin(), container.end(), el);
    if (it != container.end())
      return false;

    container.emplace_back(el);
    return true;
  }
};
} // namespace tyrecon