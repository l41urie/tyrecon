
add_library(win_agent SHARED
plat_init.cpp

configuration/configuration.cpp
exfiltration/exfiltration.cpp

instrumentation/veh.cpp
instrumentation/fault_inducer.cpp
instrumentation/function_replacement.cpp
instrumentation/execution_context.cpp
instrumentation/stackwalk.cpp

allocation_tracking/allocation_tracker.cpp
allocation_tracking/crt_instrumentation.cpp
allocation_tracking/track_usage.cpp

type_tracking/type_tracker.cpp
type_tracking/instrumentation.cpp

module/module.cpp

rtti/rtti.cpp
rtti/rtti_analyzer.cpp

function_finder/find_functions.cpp
function_finder/function_analyzer.cpp
function_finder/walk_pdata.cpp
function_finder/function_processing.cpp
function_finder/track_dynamic_dispatch.cpp
)

target_include_directories(win_agent PRIVATE
${PROJECT_SOURCE_DIR}/contrib/zydis/include
${PROJECT_SOURCE_DIR}/contrib/zydis/dependencies/zycore/include
${PROJECT_SOURCE_DIR}/win_agent/
)

target_link_directories(win_agent PRIVATE
${CMAKE_BINARY_DIR}/contrib/zydis
)

target_link_libraries(win_agent PRIVATE
zydis
dbghelp
)

add_executable(ada_monitor
process/process.cpp
installer/main.cpp
)

target_compile_options(win_agent PRIVATE ${ADA_COMPILE_OPTIONS})
target_compile_options(ada_monitor PRIVATE ${ADA_COMPILE_OPTIONS})