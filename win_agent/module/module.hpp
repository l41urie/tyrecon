#pragma once
#include "meta.hpp"
#include <string>
#include <vector>

namespace ada {
struct Section {
  std::string name;

  void *begin, *end;

  bool contains(void *p) { return p >= begin && p < end; }
};

struct NamedFunction
{
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