#pragma once
#include "meta.hpp"
#include <cstdio>
#include <cstring>

namespace tyrecon::argparse {

template <u32 size> struct ArgumentTuple {
  // idx 0 = "exists sentinel"
  char const *args[size + 1] = {};
};

inline char const *strskp(char const *ptr, char skip, u32 max = ~0) {
  while (*ptr == skip && (max--) != 0)
    ++ptr;
  return ptr;
}

template <u32 size>
ArgumentTuple<size> inline parse_arg(int argc, char *argv[], char const *name,
                                     u32 nrequired = 0) {
  for (u32 i = 1; i < argc; ++i) {
    char const *arg = strskp(argv[i], '-', 2);
    if (strcmp(arg, name) == 0) {
      u32 written = 0;
      ArgumentTuple<size> r{};
      if ((i + size) < argc) {
        for (u32 j = i; j < i + size + 1; ++j) {
          if (j != i && argv[j][0] == '-') {
            break;
          }

          r.args[written++] = argv[j];
        }
      }

      // 0 counts.. fix off-by-one
      written--;
      if (written < nrequired) {
        r.args[0] = nullptr;
        printf("\'%s\' requires %d followup arguments, %d were supplied\n",
               argv[i], nrequired, written);
      }

      return r;
    }
  }
  return {};
}

} // namespace tyrecon::argparse

#define ARG(name, count)                                                       \
  ::tyrecon::argparse::parse_arg<count>(argc, argv, name, count).args

#define ARG_NREQ(name, count, nreq)                                            \
  ::tyrecon::argparse::parse_arg<count>(argc, argv, name, nreq).args

#define ARG_SUPPLIED(name)                                                     \
  ::tyrecon::argparse::parse_arg<0>(argc, argv, name).args[0]