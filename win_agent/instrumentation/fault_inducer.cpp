#include "meta.hpp"
#include "veh.hpp"
#include <function_finder/functions.hpp>
#include <functional>
#include <optional>
#include <windows.h>

namespace ada {

struct SoftwareInstrumentation {
  void *begin, *end;
  u64 original_protection;

  SoftwareInstrumentation(void *begin, void *end, u64 original_protection)
      : begin(begin), end(end), original_protection(original_protection) {}

  bool triggered_violation(Violation const &violation) const {
    return violation.fault_address >= begin && violation.fault_address < end;
  }

  virtual void on_execute() {}

  static std::optional<SoftwareInstrumentation> install_singlehit(void *begin,
                                                                  void *end) {
    ASSERT(end > begin);

    // set page_guard flag
    MEMORY_BASIC_INFORMATION prot;
    if (VirtualQuery(begin, &prot, sizeof(prot)) == 0)
      return std::nullopt;

    auto newprot = prot.Protect | PAGE_GUARD;
    DWORD oldprot = 0;

    u64 size = (((u64)end - (u64)begin) & ~(0x1000 - 1)) + 0x1000;

    VirtualProtect(begin, size, newprot, &oldprot);
    return SoftwareInstrumentation{begin, end, oldprot};
  }
};

struct InstrumentationHandler {
  std::vector<SoftwareInstrumentation> instrumented_functions;

  void handle_fault();

  
};
} // namespace ada