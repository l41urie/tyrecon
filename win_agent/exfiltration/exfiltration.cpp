#include "module/module.hpp"
#include "type_tracking/type_tracker.hpp"
#include "function_finder/functions.hpp"
#include <cstdarg>
#include <fstream>
#include <windows.h>
#include <string>
#include <Psapi.h>

namespace tyrecon {
std::string log;

void println(char const *format, ...) {
  va_list args;
  va_start(args, format);
  char buf[512];
  vsprintf_s(buf, format, args);
  va_end(args);

  log.append(buf);
  log.append("\n");
}

bool format_va(void *ptr, char *buff, size_t size) {
  HMODULE mod;
  if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)ptr,
                         &mod) == NULL)
    return false;

  Module m(mod);
  auto const rva = (u64)ptr - (u64)mod;

  sprintf_s(buff, size, "%s!%llX", m.name.data(), rva);
  return true;
}

void exfiltrate_all() {
  println("Functions: ");

  for_each_function([](const tyrecon::Function &f) {
    char function_formatted[256];
    format_va(f.start, function_formatted, 256);

    println("%s: %s%s", function_formatted,
            f.type == FunctionKind::FRAME ? "frame" : "leaf",
            f.dynamic_dispatch ? ",dynamic" : "");
    if (f.dynamic_dispatch && !f.called_from.empty()) {
      println("Called from:");
      for (auto p : f.called_from) {
        char caller_name[256];
        format_va(p, caller_name, 256);
        println("\t- %s", caller_name);
      }
    }
    return false;
  });

  println("\n\nTypes:\n");

  for (auto const &t : global_typelist) {
    char vt_name[256] = "";
    format_va((void *)t.vtable.start, vt_name, 256);

    println("%s%s%s (%llX functions)", t.name.c_str(), t.rtti ? " - " : "",
            vt_name, t.vtable.len() / 8);
    if (t.allocation_size != 0)
      println("\tguessed size: %llx", t.allocation_size);

    if (!t.usage.empty()) {
      println("Passed into:");
      for (auto const &call : t.usage) {
        char fn_name[256] = "";
        format_va(call.fn, fn_name, 256);

        std::string parameter_usage;
        for (auto i = 0; i < 32; ++i)
          if (call.parameter_bits.test(i))
            parameter_usage += ",a" + std::to_string(i + 1);

        println("\t- %s (%s)", fn_name, parameter_usage.c_str() + 1);
      }
    }
  }

  std::ofstream ofs("tyrecon.log", std::ios::trunc);
  ofs << log;
  ofs.close();
}
} // namespace tyrecon