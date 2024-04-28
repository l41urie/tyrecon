
add_library(win_agent SHARED
plat_init.cpp

configuration.cpp

debugger/veh.cpp

module/module.cpp

function_finder/find_functions.cpp
function_finder/function_analyzer.cpp
function_finder/walk_pdata.cpp
)

target_include_directories(win_agent PRIVATE
${PROJECT_SOURCE_DIR}/contrib/zydis/include
${PROJECT_SOURCE_DIR}/contrib/zydis/dependencies/zycore/include
)

target_link_directories(win_agent PRIVATE
${CMAKE_BINARY_DIR}/contrib/zydis
)

target_link_libraries(win_agent PRIVATE
zydis
)

add_executable(ada_monitor
process/process.cpp
installer/main.cpp
)

target_compile_options(win_agent PRIVATE ${ADA_COMPILE_OPTIONS})
target_compile_options(ada_monitor PRIVATE ${ADA_COMPILE_OPTIONS})