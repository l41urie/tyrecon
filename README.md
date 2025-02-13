# Tyrecon - Type reconnaissance through RTTI

Tyrecon is a hackable automated debugger that attempts to provide in-depth analysis of dynamic program execution by breakpointing all functions.

Currently, it's main features are
1. Providing call-site analysis for Virtual functions / dynamic dispatch.
2. Uncovering Parameter types that are pointers to composite types through RTTI.

# Usage
As of now, tyrecon needs a bunch of adjustments so you can properly analyze a binary.

`win_agent/configuration.cpp` provides a starting point for this.

`find_functions()` needs to be called on all modules containing functions that should be monitored.

In case of Allocators other than the one imported by `win_agent.dll` itself (`ucrtbase!mallloc`), The hook for the allocator in `allocation_tracking\crt_instrumentation.cpp` needs to be adjusted.


# Building
Prerequisites:
* CMake
* Clang (GNU CLI)
* Visual studio Toolchain

Cloning the repository:
```sh
git clone --recursive https://github.com/l41urie/tyrecon.git
cd tyrecon

# from here on, using vscode's cmake extension will work fine
code .
```

