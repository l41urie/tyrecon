#pragma once
#include "memory_block.hpp"
#include "meta.hpp"
#include <string>
#include <vector>

namespace ada {
struct Section {
  std::string name;
  ada::Block memory_block;

  NODISCARDINL bool contains(void *p) const { return memory_block.contains((u64)p); }
};

struct NamedFunction {
  void *location;
  std::string name;
};

struct Module {
  std::string module_path;
  std::string_view name;
  ada::Block memory_block;
  void *image;

  std::vector<Section> sections;

  std::vector<NamedFunction> exports;
  std::vector<NamedFunction> imports;

  Section const *section(std::string const &symbol_name) const {
    auto it =
        std::find_if(sections.begin(), sections.end(),
                     [&](Section const &s) { return s.name == symbol_name; });
    return it == sections.end() ? nullptr : &(*it);
  }
  // TODO: add the same for |exports| and |imports|, by templates?

  Module(void *image);
};
} // namespace ada