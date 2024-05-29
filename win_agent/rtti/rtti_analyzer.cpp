#include "module/module.hpp"
#include "rtti.hpp"
#include <cstdio>
#include <set>
#include "type_tracking/type_tracker.hpp"
#include "function_finder/functions.hpp"

namespace ada::rtti {
/* Finds aligned pointer-tables in |containing_section|
   that point into |pointing_section|
*/
void find_pointer_tables(ada::Block const &containing_section,
                         ada::Block const &pointing_section,
                         std::set<ada::Block> &tables,
                         u32 const minimum_table_size = 2) {

  // Walk |containing_section|
  auto addr = containing_section.start;
  while (addr < containing_section.end - sizeof(u64)) {
    auto inside = addr;
    u64 found = 0;
    while (inside < containing_section.end - sizeof(u64)) {
      if (!pointing_section.contains(*(u64 *)inside))
        break;

      found++;
      inside += sizeof(u64);
    }

    // addr points to start
    // inside points to end
    if (found >= minimum_table_size) {
      tables.emplace(addr, inside);
      addr = inside; // skip what we just found.
    } else
      addr += sizeof(u64);
  }
}

void find_dispatch_tables(ada::Module const &mod,
                          std::set<ada::Block> &dispatch_tables) {
  auto *text = mod.section(".text");
  auto *rdata = mod.section(".rdata");

  if (!text || !rdata)
    return;

  find_pointer_tables(rdata->memory_block, text->memory_block, dispatch_tables);

#if 0
  printf("Dispatch tables:\n");
  for (auto ptr : dispatch_tables)
    printf("\t- %llX [%lld]\n", ptr.start, ptr.len() / 8);
#endif
}

// Performs dispatch table & rtti discovery
void find_rtti(ada::Module const &mod) {
  auto *text = mod.section(".text");
  auto *rdata = mod.section(".rdata");
  if (!text || !rdata)
    return;

  std::set<ada::Block> tables;
  rtti::find_dispatch_tables(mod, tables);

  for (auto const &t : tables) {
    bool discovered_rtti = [&]() {
      auto rtti_addr = t.start - sizeof(u64);
      if (!rdata->memory_block.contains(rtti_addr))
        return false;

      auto col_addr = *(u64 *)rtti_addr;
      if (!rdata->memory_block.contains(col_addr))
        return false;

      auto *col = (rtti::CompleteObjectLocator *)col_addr;
      if (!col->check_signature())
        return false;

      auto td = col->type_descriptor.rebase(mod.memory_block);
      if (!td)
        return false;

      global_typelist.discover_rtti(col, td, t);
      return true;
    }();

    if (!discovered_rtti)
      global_typelist.discover_vtable(t);

    // mark every function in the table as dynamically dispatchable
    for (auto p = t.start; p < t.end; p += sizeof(void *)) {
      // try to find the corresponding function
      if (auto fn = ada::find_function(*(void **)p)) {
        fn->dynamic_dispatch = true;
      }
    }
  }
}
} // namespace ada::rtti