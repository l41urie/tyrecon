#include "type_tracker.hpp"
#include "rtti/rtti.hpp"
#include <algorithm>
#include "instrumentation/execution_context.hpp"

namespace tyrecon {

void TypeList::discover_rtti(rtti::CompleteObjectLocator *col,
                             rtti::TypeDescriptor *td,
                             tyrecon::Block const &vtable) {
  Type t;
  t.vtable = vtable;
  t.rtti = col;
  if (td) {
    auto demangled = td->demangle();
    if (demangled.has_value())
      t.name = *demangled;
  }

  types.emplace_back(t);
}

void TypeList::discover_vtable(tyrecon::Block const &vtable) {
  Type t;
  t.vtable = vtable;
  types.emplace_back(t);
}

void TypeList::discover_usage(tyrecon::Allocation const &allocation, void *ptr,
                              u32 index, FunctionExecutionContext const &ctx) {
  if (allocation.status != ALLOCATED)
    return; // we won't do analysis on bad stuff.

  // find the assoicated type!
  auto it = std::find_if(types.begin(), types.end(), [&](Type const &t) {
    return t.vtable.start == *(u64 *)ptr;
  });

  if (it == types.end())
    return; // not a vaid vtable.

  it->mark_used(ctx.rip(), index);

  // this is the first place we can associate |allocation| to Type
  // fill out the size if we're fairly sure that the allocation directly belongs
  // to the type and we're not dealing with some sort of subtype.
  if (allocation.memory.start == (u64)ptr) {
    if (it->allocation_size == 0)
      it->allocation_size = allocation.memory.len();
    else {
      // TODO: this is an inconsistency about our assumptions, the allocation
      // site most likely does not correspond to what set up the object
      // one example in which this might happen is std::vector, the allocation
      // happens when the reserved memory gets exceeded, thus it is not tied to
      // the object creation directly
    }
  }
}

void Type::mark_used(void *fn, u32 index) {
  auto const it = std::find(usage.begin(), usage.end(), TypeUsage{fn, {}});
  auto const index_bit = 1 << index;
  if (it == usage.end())
    // insert new entry
    usage.emplace_back(fn, index_bit);
  else
    // mark the used parameter
    it->parameter_bits |= index_bit;
#if 0
  printf("%p:%d = %s\n", fn, index, name.c_str());
#endif
}
} // namespace tyrecon