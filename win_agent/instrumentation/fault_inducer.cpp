#include "fault_inducer.hpp"
#include "meta.hpp"
#include "veh.hpp"
#include <function_finder/functions.hpp>
#include <optional>
#include <windows.h>

namespace ada {

std::optional<MEMORY_BASIC_INFORMATION>
set_protect(void *begin, void *end, u64 (*change_prot)(u64 prot)) {
  // set page_guard flag
  MEMORY_BASIC_INFORMATION prot;
  if (VirtualQuery(begin, &prot, sizeof(prot)) == 0)
    return std::nullopt;

  DWORD oldprot = 0;
  ASSERT(end > begin);
  u64 size = (u64)end - (u64)begin;

  if (VirtualProtect(begin, size, change_prot(prot.Protect), &oldprot) == 0)
    return std::nullopt;

  return prot;
}

bool InstrumentationHandler::instrument(ada::Function const &fn) {
  auto instr = CodeInstrumentation::install(fn.start);
  if (!instr.has_value())
    return false;

  instrumented_instructions.emplace_back(*instr);
  instr->enable();
  return true;
}

CodeInstrumentation *
InstrumentationHandler::find_code_instrumentation(Violation const &v,
                                                  _EXCEPTION_POINTERS *eh) {
  auto instrumentation_hit = std::find_if(
      instrumented_instructions.begin(), instrumented_instructions.end(),
      [&v](CodeInstrumentation const &instr) {
        return instr.instruction == v.ip;
      });

  if (instrumentation_hit == instrumented_instructions.end())
    return nullptr;

  return &(*instrumentation_hit);
}

void CodeInstrumentation::enable() {
  // Fix the permission.
  VirtualProtect(instruction, 1, PAGE_EXECUTE_READWRITE,
                 (DWORD *)&this->original_protection);

  // write INT3
  *(u8 *)instruction = 0xCC;

  DWORD prot;
  VirtualProtect(instruction, 1, this->original_protection, &prot);
}

void CodeInstrumentation::disable() {
  // Fix the permission.
  VirtualProtect(instruction, 1, PAGE_EXECUTE_READWRITE,
                 (DWORD *)&this->original_protection);

  // Restore original byte
  *(u8 *)instruction = original_byte;

  DWORD prot;
  VirtualProtect(instruction, 1, this->original_protection, &prot);
}

std::optional<CodeInstrumentation> CodeInstrumentation::install(void *instr) {
  CodeInstrumentation result;
  result.instruction = instr;

  MEMORY_BASIC_INFORMATION prot;
  if (VirtualQuery(instr, &prot, sizeof(prot)) == 0)
    return std::nullopt;

  // can't execute anyway, this surely is a bad call to `install()`
  if (prot.Protect != PAGE_EXECUTE && prot.Protect != PAGE_EXECUTE_READ &&
      prot.AllocationProtect != PAGE_EXECUTE_READWRITE) {
    return std::nullopt;
  }

  result.original_byte = *(u8 *)instr;
  result.original_protection = prot.AllocationProtect;
  return result;
}
} // namespace ada