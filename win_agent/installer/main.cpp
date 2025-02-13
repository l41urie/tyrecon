#include "../../shared/meta.hpp"
#include "../process/process.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <string>
#include <windows.h>

int arg_idx(int argc, char *argv[], char const *name) {
  for (auto i = 1; i < argc; ++i)
    if (strcmp(argv[i], name) == 0)
      return i;
  return 0;
}

#define ARG(name) arg_idx(argc, argv, name)

int print_usage(char const *error) {
  if (error)
    printf("Error occured: %s\n", error);

  printf("Usage:\n"
         "monitor <arguments>\n"
         "arguments:\n"
         "-d (Dynamic) <application name>\n"
         "-s (Static) <application name> <Command line>\n");

  return error ? 1 : 0;
}

int main(int argc, char *argv[]) {
  if (ARG("-help"))
    return print_usage(nullptr);

  int static_init = ARG("-s");
  int dynamic_init = ARG("-d");

  if ((static_init != 0) == (dynamic_init != 0))
    return print_usage("no single action specified, -d or -s is required");

  tyrecon::Process proc;

  if (static_init) {
    ASSERT(argc >= static_init + 2);
    char const *application_path = argv[static_init + 1];
    char *cli = argv[static_init + 2];

    auto p = std::filesystem::absolute(application_path);
    if(!std::filesystem::exists(p))
    {
      return print_usage("application not found");
    }

    if(!cli)
      cli = "";

    auto p_str = p.string();
    printf("running \"%s\" (\"%s\") with cli=\"%s\"... ", application_path, p_str.c_str(), cli);
  
    proc = tyrecon::start_suspended_process(p_str.c_str(), cli);
    if(!proc.handle)
      return print_usage("Failed to start process");

    printf("pid %d\n", proc.pid);
  }

  if (dynamic_init) {
    ASSERT(argc >= dynamic_init + 1);
    char const *running_application = argv[dynamic_init + 1];

    printf("Attempting to attach to \"%s\"...\n", running_application);

    proc = tyrecon::get_handle_to_remote(running_application);
    if(!proc.handle)
      return print_usage("Failed to attach");

    printf("Done!\n");
  }

  auto path = std::filesystem::absolute("win_agent.dll");
  if (!std::filesystem::exists(path))
    return print_usage("agent not found, installation is corrupted.\n");

  auto path_str = path.string();
  auto mem = proc.alloc(path_str.length() + 1, PAGE_READWRITE);
  if(!mem.remote_addr)
    return print_usage("Failed to allocate memory for agent path\n");
  memcpy(mem, path_str.c_str(),
         path_str.length()); // include null terminator

  mem[path_str.length()] = 0; // manually wirte null terminator
  
  if(mem.push())
    printf("Pushed agent path \"%s\"\n", path_str.c_str());
  else
    return print_usage("Failed to push agent path");

  // HACK: this works because the DLL implementing LoadLibraryA is loaded at the
  // same place in this process, as it is in the remote process
  // it's unlikely we'll ever have to fix this, many programs rely on this behavior.
  HANDLE load_thread = CreateRemoteThread(proc.handle, 0, 0,
                                          (LPTHREAD_START_ROUTINE)LoadLibraryA,
                                          mem.remote_addr, 0, 0);

  printf("Remote thread set-up, waiting... ");
  WaitForSingleObject(load_thread, INFINITE);

  // FIXME: using GetExitCodeThread() is terribly wrong here
  //        there is no guarantee that `(imagebase & ~0u32) != 0`, but it's unlikely enough for now
  // ideally, this should iterate the loaded modules in the remote & make sure the agent is loaded.
  DWORD result = 0;
  GetExitCodeThread(load_thread, &result);

  if(result != 0){
    printf("Ok!\n");

    if(static_init)
    {
      // Process is still in a suspended state, resume all threads
      tyrecon::for_all_threads(proc.pid, [](HANDLE h) { ResumeThread(h); });
    }
  }
  else
    printf("LoadLibrary() failed!\n");

  mem.free_all();
  return 0;
}