#include "../disassembler/zydis_disasm.hpp"
#include <cstring>
#include <vector>
#include <windows.h>

/*
This replaces a function in the target binary by placing
a JMP at the beginning of the function.

In order to preserve original behavior, if desired, we disassemble
the function & copy / relocate instructions until we can fit a jump.
*/

#if 1
#define INFO printf
#else
#define INFO(...)
#endif

namespace ada::detail {

// TODO: consider switching to multiple buffers
struct CodeBuffer {
  void *mem;
  u64 size;
  u64 reserved;

  u64 last_reserved_size = 0; // last argument to reserve_unaligned()

  static_assert(sizeof(void *) == sizeof(u64), "mismatched architecture");

  void release() { VirtualFree(mem, size, MEM_DECOMMIT | MEM_RELEASE); }

  void *reserve_unaligned(u64 size) {
    reserved += size;
    last_reserved_size = size;
    return (void *)((u64)mem + reserved);
  }

  void rewind(u64 size) { reserved -= size; }

  void revert_allocation() { rewind(last_reserved_size); }

  void *reserve_aligned(u64 size, u64 align = 64) {
    // align size, example:
    // size = 68, align = 64
    // pad = (64 - ((68 % 64) == 4)) == 60
    // pad(60) + size(68) = 128
    auto const pad = align - (size % align);
    size = size + pad;

    return reserve_unaligned(size);
  }

  static CodeBuffer allocate(size_t size) {
    auto alloc = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);

    return CodeBuffer{
        .mem = alloc,
        .size = size,
        .reserved = 0,
    };
  }
};

static CodeBuffer &get_code_buffer() {
  // FIXME: keep track of multiple buffers
  // 8kb should be enough for now
  static CodeBuffer code_buffer = CodeBuffer::allocate(0x8000);
  return code_buffer;
}

struct AbsoluteJmp {
  struct __attribute__((packed, aligned(1))) Code {
    /* jmp [rip+0] */
    u8 instr[6] = {0xff, 0x25, 0x00, 0x00, 0x00, 0x00};
    u64 address;
  } code;

  static consteval size_t size() { return sizeof(code); }
  void set_target(u64 jt) { code.address = jt; }
  void write_to(void *mem) { memcpy(mem, &code, size()); }
};

bool replace_function(void *fn, void ***jmp_to, void **copied) {
  /*
    This function currently fails if the first few bytes of a function use
    RIP-Relative addressing, fixing this bug is sadly not trivial.
    There's multiple ways to accomplish this:

    - Make sure `CodeBuffer` gets allocated near |fn|
      upsides:
       * Easy to implement
      downsides:
       * May fail if we don't find a suitable spot for the allocation

    - Translate the code to be position-independent
      upsides:
       * works all the time, unless you're dealing with code no sane compiler
    would generate. downsides:
       * hard to implement
         * needs to generate new code
         * possibly needs further analysis on the original code?
           (register volatility?)
       * mild (probably unnoticeable) performance drawbacks

    - Replace every instruction that performs rip-relative addressing with INT3,
      singlestep the original instruction.
      upsides:
       * works all the time
       * Relatively easy to implement (just needs mapping from int3 ->
        original instruction)
      downsides:
       * Awful performance impact, very much noticeable
       * Can't be used in time-critical code
  */

  FunctionDisassembler dis(fn);

  // figure out what to re-encode & how long it is
  u64 copy_len = 0;
  std::vector<ZydisEncoderRequest> to_encode;
  Instruction instr;
  while (copy_len < AbsoluteJmp::size() && dis.step(instr)) {
    ZydisEncoderRequest request;

    auto status = ZydisEncoderDecodedInstructionToEncoderRequest(
        &instr.info, instr.operands, instr->operand_count_visible, &request);
    if (ZYAN_FAILED(status)) {
      INFO("Failed to assemble encoder requests.\n");
      return false;
    }

    to_encode.emplace_back(request);
    copy_len += instr->length;
  }

  // to_encode & copy_len are now filled
  // reserve space in the codebuffer & re-encode
  CodeBuffer &cb = get_code_buffer();
  void *buf = cb.reserve_aligned(copy_len + AbsoluteJmp::size() * 2);
  u64 assembled = 0;

  for (auto &req : to_encode) {
    u64 len = copy_len - assembled; // length left in buffer
    if (!ZYAN_SUCCESS(ZydisEncoderEncodeInstructionAbsolute(
            &req, ((u8 *)buf + assembled), &len, (ZyanU64)buf + assembled))) {
      cb.revert_allocation();
      INFO("Failed to Re-assemble after %llu instructions\n", assembled);
      return false;
    }
    assembled += len;
  }

  *copied = buf;
  INFO("Copied %llu instructions (%llu bytes) to %p\n", to_encode.size(),
       assembled, buf);

  // Write a `jmp` into |fn| right behind what we copied
  AbsoluteJmp jmp;
  jmp.set_target((u64)fn + assembled);
  jmp.write_to((void *)((u8 *)buf + copy_len));

  // Write a `jmp` that points to the original, but can be changed via |jmp_to|
  jmp.set_target((u64)buf);
  void *replacement_jmp = (void *)((u8 *)buf + copy_len + AbsoluteJmp::size());
  jmp.write_to(replacement_jmp);

  // let |jmp_to| point to the address that can be changed, CodeBuffer remains
  // rwx
  *jmp_to =
      (void **)((u8 *)replacement_jmp + offsetof(AbsoluteJmp::Code, address));

  // finally, put a `jmp` to |fn| that jumps to |replacement_jmp|
  {
    // TODO: using VirtualProtect here is rather ugly
    //       wrap this so we'll have an easier time when porting to different platforms.
    
    DWORD old;
    VirtualProtect(fn, AbsoluteJmp::size(), PAGE_EXECUTE_READWRITE,
                   &old);

    jmp.set_target((u64)replacement_jmp);
    jmp.write_to(fn); // this will intercept any call to |fn|

    VirtualProtect(fn, AbsoluteJmp::size(), old,
                   &old);
  }
  return true;
}

} // namespace ada::detail

namespace ada {}
