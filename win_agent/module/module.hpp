#pragma once
#include "memory_block.hpp"
#include "meta.hpp"
#include <string>
#include <vector>

namespace ada {
struct Section {
  std::string name;
  Block memory_block;

  PURE NODISCARD bool contains(void *p) const { return memory_block.contains((u64)p); }
};

struct NamedFunction {
  void *location;
  std::string name;
};

struct Module {
  std::string module_path;
  std::string_view name;
  void *image;

  std::vector<Section> sections;

  std::vector<NamedFunction> exports;
  std::vector<NamedFunction> imports;

  Module(void *image);
};
} // namespace ada