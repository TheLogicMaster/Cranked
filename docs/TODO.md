# Cranked TODO Items

## Large Items
- Audio
- Rework execution model to handle threading and current native/Lua callback issues
- Full desktop frontend
- Playdate API compatibility matrix and unit testing

## Miscellaneous
- Finish Sprites (Collision occasionally phases a sprite off-screen in Sprite Collision example)
- Remaining graphical effects (And effects on sprites)
- Polygon (Non-zero fill rule), Mode 7
- Full font support (Alignment)
- WAV writing
- Finish C JSON decoding
- Test building on Windows/Mac
- Create a testing framework to compare console output to the official simulator (Graphics, Collisions)
- Finish font-ends (Libretro core just crashes at the moment, Desktop has no features, Android only loads a test program)
- Front-end should run in a separate thread
- Java library with native libs for Android app consumption
- Scoreboard support
- Investigate Catalog app (Web API already documented elsewhere)
- Ability to act as simulator (Useful debug features)
- See if emulator can play encrypted games with a dumped key or something (Maybe limiting to just decrypted games)
- System UI/software from SDK (Create replacement assets, *Darker Grotesque* should be able to be adopted in place of *Roobert* system font)
- Investigate pre-2.0.0 binaries to handle uncompressed data (Use PDC flag)
- Emulator API wrapper to hide all the messy internals, possibly C compatible
- Values checks, since plenty of null/illegal API parameters will cause a native seg-fault
- Dynarmic native engine support for more portability
- GitHub Actions release builds
- USB serial functionality (As controller, dump saves, backup games?)
- Better execution model (The current approach works, but has limitations, coroutines might be elegant, avoiding recursive Lua invocation would be good)
- Fix git submodules to not get in detached head state
- Can't currently build in release due to false uninitialized variable errors in Capstone
- Native Cranked API for profiling and such, maybe exposed at a fixed address or at the end of the main PD API struct
- LuaRuntime being based on tables rather than userdata is likely to cause incompatibility (Where not tables...) (Already requires patches.lua), and should probably use full userdata from C++
- Better exception handling and stack traces, potentially with disassembled Asm and Lua for context, especially more Lua context, maybe even decompilation
- Lua debugger functionality
- Support Debug Adaptor Protocol like Simulator
- Events/callbacks for pausing/stopping
- Bitmap drawing performance improvements would make a large difference in overall performance
- Hardware acceleration, if practical
