#include "process.hpp"
#include <cstddef>
#include <cstdlib>
#include <handleapi.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <vector>
#include <winnt.h>


#include <Psapi.h>
#include <TlHelp32.h>

namespace ada {
ForeignMemory Process::map(void *addr, size_t size) {
  return {.proc = *this,
          .remote_addr = addr,
          .size = size,
          .local_buff = malloc(size)};
}

ForeignMemory Process::alloc(size_t size, u32 flags) {
  void *addr =
      VirtualAllocEx(handle, NULL, size, MEM_COMMIT | MEM_RESERVE, flags);
  return map(addr, size);
}

ForeignMemory::~ForeignMemory() { free(local_buff); }

bool ForeignMemory::pull() {
  u64 read = 0;
  ReadProcessMemory(proc.handle, remote_addr, local_buff, size, &read);
  return read == size;
}

bool ForeignMemory::push() {
  u64 written = 0;
  WriteProcessMemory(proc.handle, remote_addr, local_buff, size, &written);
  return written == size;
}

void ForeignMemory::free_all() {
  VirtualFreeEx(proc.handle, remote_addr, size, MEM_DECOMMIT | MEM_RELEASE);
  free(local_buff);
  local_buff = 0;
  remote_addr = 0;
  size = 0;
}

u32 ForeignMemory::set_protection(u32 flags) {
  u32 old = 0;
  VirtualProtectEx(proc.handle, remote_addr, size, flags, (PDWORD)&old);
  return old;
}

bool ForeignMemory::get_protection(u32 &out) {
  MEMORY_BASIC_INFORMATION mem;
  if (VirtualQueryEx(proc.handle, remote_addr, &mem,
                     sizeof(MEMORY_BASIC_INFORMATION)) == 0)
    return false;

  out = mem.Protect;
  return true;
}

ada::Process start_suspended_process(char const *path, char *cli) {
  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(PROCESS_INFORMATION));

  STARTUPINFO si;
  memset(&si, 0, sizeof(STARTUPINFO));

  CreateProcess(path, cli, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &si,
                &pi);

  return {pi.hProcess, pi.dwProcessId};
}

void for_all_threads(u32 pid, void (*fn)(HANDLE thread), DWORD flags) {
  WrapHandle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
  THREADENTRY32 entry{};
  entry.dwSize = sizeof(entry);

  do {
    if (entry.th32OwnerProcessID != pid)
      continue;
    WrapHandle thread =
        OpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID);
    fn(thread);
  } while (Thread32Next(snapshot, &entry));
}

ada::Process get_handle_to_remote(char const *name) {
  std::vector<u32> pids;
  PROCESSENTRY32 entry{};
  entry.dwSize = sizeof(entry);

  WrapHandle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

  do {
    if (!strcmp(entry.szExeFile, name)) {
      pids.emplace_back(entry.th32ProcessID);
    }
  } while (Process32Next(snapshot, &entry));

  if (pids.empty()) {
    return {};
  }

  // TODO: select the process that doesn't have the agent loaded already
  return {OpenProcess(PROCESS_ALL_ACCESS, FALSE, pids[0]), pids[0]};
}
} // namespace ada