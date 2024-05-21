#include "memory_block.hpp"
#include "meta.hpp"

namespace ada {
template <typename T, typename PT = void> struct Rva {
  T addr;

  NODISCARDINL PT *rebase(u64 base) const { return (PT *)(base + (u64)addr); }
  NODISCARDINL PT *rebase(void *base) const { rebase((u64)base); }

  NODISCARDINL PT *rebase(ada::Block const &block) const {
    auto r = rebase(block.start);
    return r < (PT*)block.end ? r : nullptr;
  }
};

#define RVA_TYPE(name, maps_to)                                                \
  template <typename PT> using name = Rva<maps_to, PT>;                        \
  static_assert(sizeof(name<void>) == sizeof(maps_to) &&                       \
                    __builtin_alignof(name<void>) ==                           \
                        __builtin_alignof(maps_to),                            \
                "Type constraint check failed");

RVA_TYPE(Rva16, u16);
RVA_TYPE(Rva32, u32);
RVA_TYPE(Rva64, u64);

#undef RVA_TYPE

} // namespace ada
