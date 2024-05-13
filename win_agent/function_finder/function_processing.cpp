#include "../disassembler/zydis_disasm.hpp"
#include "functions.hpp"

namespace ada {
void process_functions(Module const &mod, std::vector<Function> &list) {
  // setup |second_instruction| field
  for (auto &fn : list) {
    void *pseudo_end = fn.end ? fn.end : std::numeric_limits<void *>::max();
    FunctionDisassembler dis(fn.start, fn.end);
    Instruction inst;
    // step one instruction and fill |second_instruction|
    if (dis.step(inst)) {
      fn.second_instruction = dis.ip;
    }
    else {
      // TODO: remove function?
      //       should always be able to disassemble.
      ASSERT(false);
    }
  }
}
} // namespace ada