# Debugging

## Debugging the Emulator
Simply run the project built from sources in a debug or release-with-symbols using your debugger of choice.

### CLion
CLion should work out of the box on Windows and Unix. 
Under Windows, a CMake profile with the following is needed (After running the build commands to create hte ): 
- Build Type: `Debug` or `Release` (Must match build command and other settings which use this type)
- Toolchain: `Visual Studio 2022` (Also create a CLion Toolchain pointing to the Visual Studio install)
- Generator: `Let CMake Decide` (Defaults to whatever was selected in build command)
- CMake Options `-DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"` (Same as build command)
- Build Directory: `cmake-build-debug` or `cmake-build-release` (Created with build command)
- Build Options: `--config Debug` or `--config Release` (Must match others)

### Visual Studio
When using the Visual Studio CMake generator, the configured project can be built and debugged using the generated 
solution file.

## GDB Debugging the Emulated Program
Requires gdb-multiarch and is run from the project directory.
- Run emulator: `build/desktop/cranked_desktop --debug 1337 "PROGRAM.pdx"`
- Start GDB: `gdb-multiarch -x gdb_setup`
- Connect: `target remote localhost:1337`
- Set symbols: `add-symbol-file pdex.elf 0x60000020` (Or add `-ex 'add-symbol-file "pdex.elf" 0x60000020'` to gdb-multiarch command)
- Use normal commands like `break`, `step`, `continue`, `ctrl+c`, etc.

## Lua Debugging
Not yet supported. 

## Profiling the Emulator
### Building
A built Tracy profiler server is required to view profiled data (Prebuilt releases available for windows).
See [Tracy](https://github.com/wolfpld/tracy) documentation for full instructions and dependencies.
```bash
# Unix building example
cd core/libs/tracy/profiler
mkdir build && cd build
cmake -DLEGACY .. # Legacy flag needed only for X11
make
```
### Usage
Run the Tracy Profiler server and the Cranked client should connect and start streaming data when run.
All C++ functions which start with a `ZoneScoped` statement will be profiled. Lua code can also be
profiled with `tracy.ZoneBegin()` and `tracy.ZoneEnd()` around code to be profiled.

## Intellij Plugin
A CLion plugin was created to aid in debugging graphical issues. This ended up being a complete hack because the APIs are
undocumented and there are no examples. Ended up using a member called `myDriverDoNotUse` just to get it working, so best
practices are definitely being followed, but at least it can allow visualizing bitmaps while stepping through graphics code.
Something like the [OpenImageDebugger](https://github.com/OpenImageDebugger/OpenImageDebugger) project which is based on GDB
and a Python UI would be more robust, but would be a lot more work to implement. Not really recommended to use this plugin, but
it is functional in CLion *2024.3.2*.
