#pragma once
#include "../../shared/meta.hpp"
#include <windows.h>

namespace tyrecon {
struct ForeignMemory;

struct WrapHandle
{
  HANDLE h;
  WrapHandle(HANDLE h) : h(h)
  {}

  operator HANDLE()
  {
    return h;
  }

  ~WrapHandle(){
    CloseHandle(h);
  }
};

struct Process {
  HANDLE handle = NULL;
  u32 pid = 0;

  ForeignMemory map(void *addr, size_t size);
  ForeignMemory alloc(size_t size, u32 flags);
};

struct ForeignMemory {
  Process proc;
  void *remote_addr;
  size_t size;
  void *local_buff;

  ~ForeignMemory();

  bool push();
  bool pull();

  void free_all();

  operator u8 *() { return (u8 *)local_buff; }

  operator char *() { return (char *)local_buff; }

  operator void *() { return (void *)local_buff; }

  template <typename T> T as() { return (T)local_buff; }

  u8 &operator[](size_t idx) {
    ASSERT(idx < size);
    return ((u8 *)local_buff)[idx];
  }

  u32 set_protection(u32 flags);
  bool get_protection(u32 &out);
};

tyrecon::Process get_handle_to_remote(char const *name);

void for_all_threads(u32 pid, void (*fn)(HANDLE thread),
                     DWORD flags = THREAD_ALL_ACCESS);

tyrecon::Process start_suspended_process(char const *path, char *cli);
} // namespace tyrecon