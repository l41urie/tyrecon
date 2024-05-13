#include "allocation_tracker.hpp"
#include <algorithm>
#include <meta.hpp>

namespace ada {
AllocationList::ContainerType::iterator AllocationList::find(void *ptr) {
  return std::find_if(
      allocations.begin(), allocations.end(), [&ptr](Allocation const &a) {
        return ptr >= a.ptr && ptr < (void *)((u64)a.ptr + a.size);
      });
}

std::optional<Allocation> AllocationList::lookup_alloc(void *ptr) {
  auto it = find(ptr);
  if (it == allocations.end())
    return std::nullopt;
  return *it;
}

void AllocationList::track_new(Allocation const &a) {
  // TODO: find conflicting allocations and remove
  allocations.emplace_back(a);
}

AllocationList::FreeStatus AllocationList::mark_free(void *ptr) {
  auto it = find(ptr);
  if (it == allocations.end())
    return INVALID_FREE;

  if (it->status == FREED)
    return DOUBLE_FREE;

  it->status = FREED;
  return OK;
}

void AllocationList::mark_reallocated(void *block, void *new_block, void *from,
                                      size_t new_size) {
  auto it = find(block);
  if (it == allocations.end())
    return; // attempting to reallocate nonexistent allocation

  // setup new fields
  it->status = REALLOCATED;
  it->ptr = new_block;
  it->size = new_size;
  it->reallocation_sites.emplace(from);

  // TODO: Check for overlap and mark unused areas as free'd?
}
} // namespace ada
