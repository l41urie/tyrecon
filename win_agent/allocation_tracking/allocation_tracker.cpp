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

void AllocationList::track(Allocation const &a) {
  // TODO: handle re-assigning of memory
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
} // namespace ada
