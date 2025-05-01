#include "../../shared/meta.hpp"
#include "../../shared/argparse.hpp"
#include "../process/process.hpp"
#include "parameterized_load.hpp"
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
         "-a (Agent) <path to agent>\n"
         "-p (Parameterized Load) <export> <parameter passed>\n");

  return error ? 1 : 0;
}

void fire_thread_and_wait(LPTHREAD_START_ROUTINE r, void *param,
                          tyrecon::Process &proc) {

  tyrecon::WrapHandle thr =
      CreateRemoteThread(proc.handle, 0, 0, r, param, 0, 0);

  printf("Remote thread set-up, waiting... ");
  WaitForSingleObject(thr, INFINITE);

  // FIXME: using GetExitCodeThread() is terribly wrong here
  //        there is no guarantee that `(imagebase & ~0u32) != 0`, but it's
  //        unlikely enough for now
  // ideally, this should iterate the loaded modules in the remote & make sure
  // the agent is loaded.
  DWORD result = 0;
  GetExitCodeThread(thr, &result);

  printf("(%lx)!\n", result);
}

struct LoadsAgent {
  virtual ~LoadsAgent() {}
  virtual bool load(tyrecon::Process &proc) const {
    ASSERT(false);
    return false;
  }
};

struct AgentLoadSettings : public LoadsAgent {
  char const *path;

  AgentLoadSettings(char const *path) : path(path) {}

  bool load(tyrecon::Process &proc) const override {
    auto const plen = strlen(path);
    auto mem = proc.alloc(plen + 1, PAGE_READWRITE);
    if (!mem.remote_addr) {
      print_usage("Failed to allocate memory for agent path\n");
      return false;
    }

    memcpy(mem, path, plen);

    mem[plen] = 0; // manually wirte null terminator
    if (mem.push())
      printf("Pushed agent path \"%s\"\n", path);

    // HACK: this works because the DLL implementing LoadLibraryA is loaded at
    // the same place in this process, as it is in the remote process it's
    // unlikely we'll ever have to fix this, many programs rely on this
    // behavior.
    fire_thread_and_wait((LPTHREAD_START_ROUTINE)LoadLibraryA, mem.remote_addr,
                         proc);

    return true;
  }
};

struct ParameterizedAgentLoadSettings : public AgentLoadSettings {
  char const *param_export;
  char const *param;

  ParameterizedAgentLoadSettings(char const *path, char const *param_export,
                                 char const *param)
      : AgentLoadSettings(path), param_export(param_export), param(param) {}

  bool load(tyrecon::Process &proc) const override {
    std::vector<u8> bytes;
#define PUSH(what, sizefn, name_offs)                                          \
  u32 const name_offs = bytes.size();                                          \
  bytes.insert(bytes.end(), what, what + sizefn(what));

    // push shellcode
    PUSH(load_dll_and_set_parameters, sizeof, _);

    // make space for parameters
    u32 const offs_parameter_struct = bytes.size();
    bytes.resize(bytes.size() + sizeof(LoadData));

    PUSH(path, 1 + strlen, offs_path)
    PUSH(param_export, 1 + strlen, offs_export);
    PUSH(param, 1 + strlen, offs_param);
#undef PUSH

    auto mem = proc.alloc(bytes.size(), PAGE_EXECUTE_READWRITE);
    if (!mem.remote_addr) {
      print_usage("Failed to allocate memory for agent parameters\n");
      return false;
    }

    // write struct
    LoadData *ld = (LoadData *)(bytes.data() + offs_parameter_struct);
    ld->loadlib = (void *)LoadLibraryA;
    ld->getprocaddr = (void *)GetProcAddress;

    ld->path = (char const *)mem.remote_addr + offs_path;
    ld->exp = (char const *)mem.remote_addr + offs_export;
    ld->param = (void *)((uintptr_t)mem.remote_addr + offs_param);

    memcpy(mem, bytes.data(), bytes.size());

    if (mem.push())
      printf("Pushed agent parameters...\n");
    else
      printf("Failed to push agent parameters\n");

    fire_thread_and_wait(
        (LPTHREAD_START_ROUTINE)mem.remote_addr,
        (void *)((uintptr_t)mem.remote_addr + offs_parameter_struct), proc);

    return true;
  }
};

void perform_static_init(char const *path, char const *cli,
                         LoadsAgent const &als) {
  auto p = std::filesystem::absolute(path);
  if (!std::filesystem::exists(p)) {
    print_usage("application not found");
  }

  if (!cli)
    cli = "";

  auto p_str = p.string();
  printf("running \"%s\" (\"%s\") with cli=\"%s\"... ", path, p_str.c_str(),
         cli);
  auto proc = tyrecon::start_suspended_process(path, cli);
  if (!proc.handle) {
    print_usage("Failed to create process\n");
    return;
  }

  als.load(proc);

  tyrecon::for_all_threads(proc.pid, [](HANDLE h) { ResumeThread(h); });
}

void perform_dynamic_init(char const *proc_name, LoadsAgent const &als) {
  printf("Trying to attach to \'%s\'\n", proc_name);
  auto proc = tyrecon::get_handle_to_remote(proc_name);
  if (!proc.handle) {
    print_usage("Failed to open handle\n");
    return;
  }

  als.load(proc);
}

int main(int argc, char *argv[]) {
  if (ARG_SUPPLIED("help"))
    return print_usage(nullptr);

  auto [static_init, application_path, cli] = ARG_NREQ("s", 2, 1);
  auto [dynamic_init, running_application] = ARG("d", 1);
  auto [agent_override, agent_path] = ARG("a", 1);
  auto [parameterized_load, agent_param_export, agent_parameter] = ARG("p", 2);

  if ((static_init != 0) == (dynamic_init != 0))
    return print_usage("no single action specified, -d or -s is required");

  if (!agent_override)
    agent_path = "win_agent.dll";

  auto absolute_path = std::filesystem::absolute(agent_path);
  if (!std::filesystem::exists(absolute_path))
    return print_usage("agent \'%s\' not found.\n", absolute_path.c_str());
  auto path_str = absolute_path.string();
  agent_path = path_str.c_str();

  std::unique_ptr<LoadsAgent> loadp;

  if (!parameterized_load) {
    loadp = std::make_unique<AgentLoadSettings>(agent_path);
  } else {
    loadp = std::make_unique<ParameterizedAgentLoadSettings>(
        agent_path, agent_param_export, agent_parameter);
  }

  if (static_init)
    perform_static_init(application_path, cli, *loadp);
  else
    perform_dynamic_init(running_application, *loadp);

  return 0;
}