#pragma once
#include <optional>
#include <vector>

namespace ada {
struct FunctionExecutionContext;
struct Function;

enum AllocationStatus {
  ALLOCATED,
  REALLOCATED,
  FREED,
};

struct Allocation {
  void *ptr;
  size_t size;
  void *allocation_callsite;
  AllocationStatus status;
};

struct AllocationList {
  using ContainerType = std::vector<Allocation>;
  // TODO: add lock
  ContainerType allocations;

  void track(Allocation const &a);

  enum FreeStatus {
    OK,           // Object has been free'd successfully
    DOUBLE_FREE,  // Object has been free'd multiple times
    INVALID_FREE, // Attempt to free something that's not been tracked
  };

  FreeStatus mark_free(void *ptr);

  ContainerType::iterator find(void *ptr);

  std::optional<Allocation> lookup_alloc(void *ptr);
};

inline AllocationList global_allocations;

bool install_crt_replacements();
void inspect_function_parameters(FunctionExecutionContext const &ctx,
                                 Function *fn);
} // namespace ada
