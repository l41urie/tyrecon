#pragma once

#include "meta.hpp"
namespace ada {
enum ViolationType : u64 {
  ReadViolation = 0, // the thread attempted to read the inaccessible data
  WriteViolation =
      1,            // the thread attempted to write to an inaccessible address
  DEPViolation = 8, // the thread caused a user-mode data execution prevention
                    // (DEP) violation
};

struct Violation {
  void *fault_address;
  void *ip;
  ViolationType type;

  Violation(void *fault, void *ip, ViolationType vt)
      : fault_address(fault), ip(ip), type(vt) {}
};

void init_veh();

} // namespace ada