#pragma once
#include "meta.hpp"
#include <Zydis/Zydis.h>

namespace ada {
struct Instruction {
  ZydisDecodedInstruction info;
  ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
  void *disassembled_at; // pointer to this instruction where it was disassembled

  ZydisDecodedInstruction *operator->() { return &info; }

  ZydisDecodedOperand const &operator[](u32 idx) { return operands[idx]; }

  bool operand_as_address(u32 idx, ZyanU64 ip, ZyanU64 &out) {
    return ZYAN_SUCCESS(
        ZydisCalcAbsoluteAddress(&info, &operands[idx], (ZyanU64)ip, &out));
  }

  bool is_a(ZydisMnemonic const mnem) { return mnem == info.mnemonic; }
};

struct FunctionDisassembler {
  ZydisDecoder decoder;
  void *ip, *end;

  FunctionDisassembler(void *ip, void *end = (void*)~0ull) : ip(ip), end(end) {
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64,
                     ZYDIS_STACK_WIDTH_64);
  }

  bool step(Instruction &out) {
    if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&decoder, ip,
                                             (ZyanUSize)((u8 *)end - (u8 *)ip),
                                             &out.info, out.operands)))
      return false;

    out.disassembled_at = ip;
    ip = (u8 *)ip + out.info.length;
    return true;
  }
};
} // namespace ada