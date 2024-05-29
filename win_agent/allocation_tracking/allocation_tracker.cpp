#include "allocation_tracker.hpp"
#include <algorithm>
#include <meta.hpp>

// TODO: this needs testing, im unsure if the part that splits stuff up works.

namespace ada {
AllocationList::ContainerType::iterator AllocationList::find(void *ptr) {
  return std::find_if(
      allocations.begin(), allocations.end(),
      [&ptr](Allocation const &a) { return a.memory.contains((u64)ptr); });
}

std::optional<Allocation> AllocationList::lookup_alloc(void *ptr) {
  auto it = find(ptr);
  if (it == allocations.end())
    return std::nullopt;
  return *it;
}

void AllocationList::track_new(Allocation const &a) {
  for (auto it = allocations.begin(); it != allocations.end();) {
    // make sure to remove all collisions with the new block
    if (it->memory.intersect(a.memory)) {
      // it will get removed and replaced by new ones that don't collide.
      auto const cuts = BlockCut::from_intersection(it->memory, a.memory);

      for (auto const &cut : cuts) {
        // we get all left-over memory blocks in here
        // construct new allocation from |cut| and |it| and ensure they're
        // marked as free'd
        auto new_alloc = *it;
        ASSERT(it->status == FREED);

        new_alloc.memory = cut;
        allocations.emplace_back(new_alloc);
      }

      // erase and start over to avoid issues with the iterator changing because
      // of the insert this can probably be done much cleaner
      allocations.erase(it);
      it = allocations.begin();
    } else
      ++it;
  }
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
  // can this be replaced by handling new / free?
  auto it = find(block);
  if (it == allocations.end())
    return; // attempting to reallocate nonexistent allocation

  // setup new fields
  it->status = REALLOCATED;
  it->memory = {(u64)new_block, (u64)new_block + new_size};
  it->reallocation_sites.emplace(from);

  // TODO: Check for overlap and mark unused areas as free'd?
}

char const *status_to_string(AllocationStatus s) {
  switch (s) {
  case ALLOCATED:
    return "ALLOCATED";
  case REALLOCATED:
    return "REALLOCATED";
  case FREED:
    return "FREED";
    break;
  }
  return "?";
}

void AllocationList::print_list() {
  printf("Allocations:\n");
  for (auto const &alloc : allocations) {
    printf("\t[%llX - %llX] %s <- %p\n", alloc.memory.start, alloc.memory.end,
           status_to_string(alloc.status), alloc.allocation_callsite);
  }
}

} // namespace ada
