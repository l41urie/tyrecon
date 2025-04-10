#include "../../shared/meta.hpp"
#include "../../shared/argparse.hpp"
#include "../process/process.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <string>
#include <windows.h>

template <typename... Args> int print_usage(char const *error, Args... a) {
  if (error) {
    printf("Error occured: ");
    printf(error, a...);
    printf("\n");
  }

  printf("Usage:\n"
         "monitor <arguments>\n"
         "arguments:\n"
         "-d (Dynamic) <application name>\n"
         "-s (Static) <application name> <Command line>\n"
         "-a (Agent) <path to agent>");

  return error ? 1 : 0;
}

int main(int argc, char *argv[]) {
  if (ARG_SUPPLIED("help"))
    return print_usage(nullptr);

  auto [static_init, application_path, cli] = ARG_NREQ("s", 2, 1);
  auto [dynamic_init, running_application] = ARG("d", 1);
  auto [agent_override, agent_path] = ARG("a", 1);

  if ((static_init != 0) == (dynamic_init != 0))
    return print_usage("no single action specified, -d or -s is required");

  tyrecon::Process proc;

  if (static_init) {
    auto p = std::filesystem::absolute(application_path);
    if (!std::filesystem::exists(p)) {
      return print_usage("application not found");
    }

    if (!cli)
      cli = "";

    auto p_str = p.string();
    printf("running \"%s\" (\"%s\") with cli=\"%s\"... ", application_path,
           p_str.c_str(), cli);

    proc = tyrecon::start_suspended_process(p_str.c_str(), cli);
    if (!proc.handle)
      return print_usage("Failed to start process");

    printf("pid %d\n", proc.pid);
  }

  if (dynamic_init) {
    printf("Attempting to attach to \"%s\"...\n", running_application);

    proc = tyrecon::get_handle_to_remote(running_application);
    if (!proc.handle)
      return print_usage("Failed to attach");

    printf("Done!\n");
  }

  if (!agent_override)
    agent_path = "win_agent.dll";

  auto path = std::filesystem::absolute(agent_path);
  if (!std::filesystem::exists(path))
    return print_usage("agent \'%s\' not found.\n", path.c_str());

  auto path_str = path.string();
  auto mem = proc.alloc(path_str.length() + 1, PAGE_READWRITE);
  if (!mem.remote_addr)
    return print_usage("Failed to allocate memory for agent path\n");
  memcpy(mem, path_str.c_str(),
         path_str.length()); // include null terminator

  mem[path_str.length()] = 0; // manually wirte null terminator

  if (mem.push())
    printf("Pushed agent path \"%s\"\n", path_str.c_str());
  else
    return print_usage("Failed to push agent path");

  // HACK: this works because the DLL implementing LoadLibraryA is loaded at the
  // same place in this process, as it is in the remote process
  // it's unlikely we'll ever have to fix this, many programs rely on this
  // behavior.
  HANDLE load_thread = CreateRemoteThread(proc.handle, 0, 0,
                                          (LPTHREAD_START_ROUTINE)LoadLibraryA,
                                          mem.remote_addr, 0, 0);

  printf("Remote thread set-up, waiting... ");
  WaitForSingleObject(load_thread, INFINITE);

  // FIXME: using GetExitCodeThread() is terribly wrong here
  //        there is no guarantee that `(imagebase & ~0u32) != 0`, but it's
  //        unlikely enough for now
  // ideally, this should iterate the loaded modules in the remote & make sure
  // the agent is loaded.
  DWORD result = 0;
  GetExitCodeThread(load_thread, &result);

  if (result != 0) {
    printf("Ok!\n");

    if (static_init) {
      // Process is still in a suspended state, resume all threads
      tyrecon::for_all_threads(proc.pid, [](HANDLE h) { ResumeThread(h); });
    }
  } else
    printf("LoadLibrary() failed!\n");

  mem.free_all();
  return 0;
}