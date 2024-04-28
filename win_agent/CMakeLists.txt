
add_library(win_agent SHARED
plat_init.cpp

configuration.cpp

debugger/veh.cpp

module/module.cpp

function_finder/find_functions.cpp
function_finder/walk_pdata.cpp
)

add_executable(ada_monitor
process/process.cpp
installer/main.cpp
)