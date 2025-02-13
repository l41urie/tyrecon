#pragma once
#include "allocation_tracking/allocation_tracker.hpp"
#include "memory_block.hpp"
#include "meta.hpp"
#include "rtti/rtti.hpp"
#include <bitset>
#include <list>
#include <string>

namespace tyrecon {
namespace rtti {
struct CompleteObjectLocator;
struct TypeDescriptor;
} // namespace rtti

struct Type {
  /* The name of the type, may only be available if it has RTTI data attached,
  or if it has been added manually.
  */
  std::string name;

#if 0
  // TODO: this needs more thoughts.
  /*T Where does this type get allocated?
  Could be filled from a few Datasources, including but not limited to:
   - RTTI
   - Overlapping function calls (Allocated from X and Y, passed into function Z)
  */
  std::vector<tyrecon::CallStack> allocation_sites;
#endif

  // Guessed size of this type in bytes, 0 if unknown.
  size_t allocation_size = 0;

  /*
    A list of functions using this type
  */
  struct TypeUsage {
    void *fn; // Function using this type.

    bool operator==(TypeUsage const &rhs) const { return fn == rhs.fn; }

    // Let's just assume there are no functions with more than 32
    // parameters. Every bit that's set in here means that the function uses
    // this exact type as parameter.
    std::bitset<32> parameter_bits;
  };
  std::list<TypeUsage> usage;
  void mark_used(void *fn, u32 index);

  // May be null if this type does not have RTTI attached to it
  tyrecon::rtti::CompleteObjectLocator *rtti = nullptr;

  // Most likely the vtable (may also exist when rtti = null)
  tyrecon::Block vtable{};

  /*
  TODO:
  Add written_to field, may be obtained by adding memory-breakpoints when
  allocating. describes the CodeLocation / Callstack(?) this type has been first
  written to from.
  */
};

struct TypeList {
  std::list<Type> types;
  FWD_ITERATORS(types);

  // called for every discovered rtti type when scanning a module
  void discover_rtti(rtti::CompleteObjectLocator *col, rtti::TypeDescriptor *td,
                     tyrecon::Block const &vtable);

  // called for every discovered vtable when scanning
  void discover_vtable(tyrecon::Block const &vtable);

  // called for every argument of any tracked function
  void discover_usage(tyrecon::Allocation const &allocation, void *ptr, u32 index,
                      FunctionExecutionContext const &ctx);
};

inline TypeList global_typelist;

void track_type_usage(FunctionExecutionContext &ctx, Function *fn);
} // namespace tyrecon