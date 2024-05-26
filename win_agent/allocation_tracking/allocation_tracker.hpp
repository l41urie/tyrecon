#pragma once
#include <optional>
#include <set>
#include <vector>
#include <memory_block.hpp>

namespace ada {
struct FunctionExecutionContext;
struct Function;

enum AllocationStatus {
  ALLOCATED,
  REALLOCATED,
  FREED,
};

struct Allocation {
  ada::Block memory;
  void *allocation_callsite;
  std::set<void *> reallocation_sites;
  AllocationStatus status;
};

struct AllocationList {
  using ContainerType = std::vector<Allocation>;
  // TODO: add lock
  ContainerType allocations;

  void track_new(Allocation const &a);

  enum FreeStatus {
    OK,           // Object has been free'd successfully
    DOUBLE_FREE,  // Object has been free'd multiple times
    INVALID_FREE, // Attempt to free something that's not been tracked
  };

  FreeStatus mark_free(void *ptr);

  void mark_reallocated(void *block, void *new_block, void *from,
                        size_t new_size);

  ContainerType::iterator find(void *ptr);

  std::optional<Allocation> lookup_alloc(void *ptr);

  void print_list();
};

inline AllocationList global_allocations;

bool install_crt_replacements();
void inspect_function_parameters(FunctionExecutionContext const &ctx,
                                 Function *fn);
} // namespace ada
