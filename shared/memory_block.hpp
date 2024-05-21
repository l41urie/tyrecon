#pragma once
#include "meta.hpp"
namespace ada {

struct Block {
  u64 start = 0, end = 0;

  PURE NODISCARD bool contains(u64 n) const { return n >= start && n < end; }

  PURE NODISCARD bool intersect(Block const &other) const {
    return contains(other.start) || contains(other.end) ||
           (other.start <= start && other.end >= end);
  }
};

struct BlockCut {
  u8 num_remaining = 0;
  Block remaining[2];

  Block &operator[](u32 const i) { return remaining[i]; }
  Block const &operator[](u32 const i) const { return remaining[i]; }

  Block *begin() { return remaining; }
  Block *end() { return remaining + num_remaining; }

  Block const *begin() const { return remaining; }
  Block const *end() const { return remaining + num_remaining; }

  PURE static BlockCut from_intersection(Block const &full,
                                         Block const &window);
};

inline BlockCut BlockCut::from_intersection(Block const &full,
                                            Block const &window) {
  BlockCut result;

  // filters out size = 0
  auto add = [&](u64 start, u64 end) {
    if (start != end)
      result[result.num_remaining++] = {start, end};
  };

  // ---------- < full
  //    |---------| < window
  // |--| < remaining
  if (window.start >= full.start && window.start <= full.end)
    add(full.start, window.start);

  //    ----------- < full
  // |---------| < window
  //           |--| < remaining
  if (window.end >= full.start && window.end <= full.end)
    add(window.end, full.end);

  return result;
}
} // namespace ada