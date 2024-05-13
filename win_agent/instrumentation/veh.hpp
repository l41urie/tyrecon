#pragma once

#include "meta.hpp"
namespace ada {
enum ViolationType : u64 {
  ReadViolation = 0, // the thread attempted to read the inaccessible data
  WriteViolation =
      1,            // the thread attempted to write to an inaccessible address
  DEPViolation = 8, // the thread caused a user-mode data execution prevention
                    // (DEP) violation
  Breakpoint = 16,
};

struct Violation {
  void *fault_address;
  void *ip;
  ViolationType type;
};

void init_veh();

} // namespace ada