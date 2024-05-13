#include "../disassembler/zydis_disasm.hpp"
#include "functions.hpp"

namespace ada {
void find_leaf_functions(Module const &mod, std::vector<Function> &list) {
  // A list of functions not yet present in |list|
  std::vector<Function> leaf_functions;

  for (auto const &fn : list) {
    void *pseudo_end = fn.end ? fn.end : std::numeric_limits<void *>::max();
    FunctionDisassembler dis(fn.start, fn.end);
    Instruction inst;
    while (dis.step(inst)) {
      // when hitting RET outside of a branch, fill out end field

      /* when discovering a CALL, figure out if we haven't seen the function
         yet. this may happen with 'leaf functions' leaf functions do not
         require a function table entry. they do not make changes to nonvolatile
         registers, don't call any other functions & don't allocate stack space.
         the stack can also be left unaligned while they execute.
      */
      if (inst.is_a(ZYDIS_MNEMONIC_CALL)) {
        ZyanU64 addr;
        if (inst.operand_as_address(0, (ZyanU64)inst.disassembled_at, addr)) {
          auto it = std::find_if(
              list.begin(), list.end(),
              [addr](Function const &fn) { return fn.start == (void *)addr; });

          if (it == list.end()) {
            // we discovered a function that's not present yet
            // figure out if we already saw another call to it?
            // if not, insert it.
            if (std::find_if(leaf_functions.begin(), leaf_functions.end(),
                             [addr](Function const &fn) {
                               return fn.start == (void *)addr;
                             }) == leaf_functions.end())
              leaf_functions.emplace_back(Function{
                  .start = (void *)addr,
                  .end = nullptr,
                  .parameter_count = ~0u,
                  .type = FunctionKind::LEAF,
              });
          }
        }
      }
    }

    // TODO: when disassembly fails, fill out `end` field.
  }

#if 0
  printf("%llu frame functions, %llu leaf functions\n", list.size(),
         leaf_functions.size());
#endif

  // merge leaf functions & list.
  for (auto const &leaf : leaf_functions) {
    // if |leaf| is contained within |fn|, don't insert it.
    auto found =
        std::find_if(list.begin(), list.end(), [&leaf](Function const &fn) {
          if (fn.end == nullptr)
            return false; // unsure
          return fn.contains(leaf.start);
        });

    if (found == list.end())
      list.emplace_back(leaf);
  }

#if 0
  printf("merged to %llu functions\n", list.size());
#endif
}
} // namespace ada