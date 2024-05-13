#include "veh.hpp"
#include "fault_inducer.hpp"
#include "execution_context.hpp"
#include <functional>
#include <windows.h>

namespace ada {
static auto constexpr SINGLE_STEP_FLAG = 0x100;

LONG WINAPI veh_handler(_EXCEPTION_POINTERS *eh) {
  static thread_local std::vector<std::function<void()>> singlestep_callbacks;

  switch (eh->ExceptionRecord->ExceptionCode) {
  case EXCEPTION_BREAKPOINT: {
    // find the corresponding code instrumentation
    Violation v{
        .fault_address = (void *)eh->ContextRecord->Rip,
        .ip = (void *)eh->ContextRecord->Rip,
        .type = Breakpoint,
    };

    auto ci = global_instrumentations.find_code_instrumentation(v, eh);
    if (!ci)
      return EXCEPTION_CONTINUE_SEARCH;

    FunctionExecutionContext ctx;
    ctx.ctx = eh->ContextRecord;
    global_instrumentations.function_instrumentation_callback(ctx);

    // found the instrumentation, singlestep the original instruction and
    // restore the breakpoint
    ci->disable();
    eh->ContextRecord->EFlags |= SINGLE_STEP_FLAG;

    singlestep_callbacks.emplace_back([ci]() { ci->enable(); });

    return EXCEPTION_CONTINUE_EXECUTION;
  }

  case EXCEPTION_SINGLE_STEP: {
    for (auto &cb : singlestep_callbacks)
      cb();
    singlestep_callbacks.clear();
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  case EXCEPTION_ACCESS_VIOLATION: {
    auto v = Violation{
        .fault_address = (void *)eh->ExceptionRecord->ExceptionInformation[1],
        .ip = (void *)eh->ContextRecord->Rip,
        .type = (ViolationType)eh->ExceptionRecord->ExceptionInformation[0],
    };
    return EXCEPTION_CONTINUE_EXECUTION;
    break;
  }

  case EXCEPTION_GUARD_PAGE: {
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  default:
    break;
  }

  DBG_PAUSE("got unhandled exception %lx at %llX\n",
            eh->ExceptionRecord->ExceptionCode, eh->ContextRecord->Rip);
  return EXCEPTION_CONTINUE_SEARCH;
}

void init_veh() { AddVectoredExceptionHandler(1, veh_handler); }
} // namespace ada