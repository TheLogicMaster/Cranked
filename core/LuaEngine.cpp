#include "LuaEngine.hpp"
#include "Cranked.hpp"
#include "gen/LuaPatchesSource.hpp"

using namespace cranked;

LuaEngine::LuaEngine(Cranked &cranked) : cranked(cranked) {}

LuaEngine::~LuaEngine() {
    if (luaInterpreter)
        lua_close(luaInterpreter);
}

void LuaEngine::init() {
    if (!cranked.rom->hasLua or (cranked.nativeEngine.isLoaded() and cranked.nativeEngine.hasUpdateCallback()))
        return; // Don't load Lua chunks in native-only mode

    luaInterpreter = lua_newstate(luaAllocator, &cranked);

    lua_atpanic(luaInterpreter, [](lua_State *context) -> int {
        auto message = lua_tostring(context, -1);
        throw CrankedError("Lua panic: {}", message ? message : "");
    });

    luaL_openlibs(luaInterpreter);
    registerLuaGlobals();
    tracy::LuaRegister(luaInterpreter);

    // Load argv like simulator (Not modifiable)
    auto argv = pushTable();
    for (int i = 0; i < cranked.config.argv.size(); i++)
        argv.setStringElement(i + 1, cranked.config.argv[i].c_str());
    setQualifiedLuaGlobal("playdate.argv");

    lua_settop(luaInterpreter, 0);
    lua_sethook(luaInterpreter, luaHook, LUA_MASKCOUNT, 10000); // Todo: Tune count param to a few milliseconds or so

    // Create update loop coroutine
    luaUpdateThread = lua_newthread(luaInterpreter);
    setQualifiedLuaGlobal("cranked.updateThread");

    if (cranked.nativeEngine.isLoaded())
        cranked.nativeEngine.invokeEventCallback(PDSystemEvent::InitLua, 2);

    // Todo: Propertly normalize imported names
    constexpr const char *PRELOADED_SOURCES[] { "CoreLibs/animator", "CoreLibs/nineslice", "CoreLibs/graphics", "CoreLibs/sprites" }; // Preload so that functions that treat objects as `userdata` can be shimmed
    Rom::File *lastLuaFile{}; // Assuming main Lua file is always last
    for (auto &file : cranked.rom->pdzFiles)
        if (file.type == Rom::FileType::LUAC) {
            for (auto source : PRELOADED_SOURCES)
                if (file.name == source) {
                    luaL_loadbuffer(luaInterpreter, (char *) file.data.data(), file.data.size(), file.name.c_str());
                    if (int result = lua_pcall(luaInterpreter, 0, 0, 0); result != LUA_OK)
                        throw CrankedError("Failed to load '{}': {}", file.name, getLuaError(result));
                    loadedLuaFiles.emplace(source, source + strlen(source) - 4);
                    break;
                }
            lastLuaFile = &file;
        }

    if (luaL_dostring(getContext(), LUA_PATCHES_SOURCE))
        throw CrankedError("Failed to load Patches: {}", lua_tostring(getContext(), -1));

    if (!lastLuaFile)
        throw CrankedError("Failed to find main Lua file");
    luaL_loadbuffer(luaInterpreter, (char *) lastLuaFile->data.data(), lastLuaFile->data.size(), lastLuaFile->name.c_str());
    if (int result = lua_pcall(luaInterpreter, 0, 0, 0); result != LUA_OK)
        throw CrankedError("Failed to load '{}': {}", lastLuaFile->name, getLuaError(result));
    lua_settop(luaInterpreter, 0);
}

void LuaEngine::reset() {
    if (luaInterpreter)
        lua_close(luaInterpreter);
    luaInterpreter = nullptr;

    loadedLuaFiles.clear();
}

void LuaEngine::runUpdateLoop() {
    if (!getQualifiedLuaGlobal("playdate.update"))
        return;
    lua_xmove(luaInterpreter, luaUpdateThread, 1);
    int results{};
    inLuaUpdate = true;
    auto result = lua_resume(luaUpdateThread, nullptr, 0, &results);
    if (results > 0 and result == LUA_YIELD) // Ignore anything returned
        lua_pop(luaUpdateThread, results);
    if (result == LUA_OK)
        lua_resetthread(luaUpdateThread);
    else if (result != LUA_YIELD) {
        string errStr = lua_isstring(luaUpdateThread, -1) ? lua_tostring(luaUpdateThread, -1) : ""; // Todo: When does this not push an error?
        inLuaUpdate = false;
        lua_resetthread(luaUpdateThread);
        throw CrankedError("Failed to run Lua update loop: {} ({})", getLuaError(result), errStr);
    }
    inLuaUpdate = false;
}

void *LuaEngine::luaAllocator(void *userData, void *ptr, size_t osize, size_t nsize) {
    [[maybe_unused]] static const auto luaHeapProfileName = "Lua Heap";
    auto cranked = (Cranked *) userData;
    TracySecureFreeN(ptr, luaHeapProfileName);
    if (nsize == 0) {
        cranked->heap.free(ptr);
        return nullptr;
    }
    try {
        auto allocated = cranked->heap.reallocate(ptr, (int)nsize);
        TracySecureAllocN(allocated, (int)nsize, luaHeapProfileName);
        return allocated;
    } catch (exception &) {
        return nullptr;
    }
}

void LuaEngine::luaHook(lua_State *luaState, lua_Debug *luaDebug) {
    // Todo: This hook may slow things down, so potentially set the hook with a count of 1 from a different thread (sethook is the only thread-safe Lua function)
    // Todo: Yielding is destructive if there's a native function in the call stack, so probably only do so if emulator is forcefully stopped
    // Todo: Nested hooks are disabled, so a Lua->Hook->...->Lua chain could block the entire program, but probably not a real concern

    // Todo: Yielding is not sufficient if program uses other coroutines, so instead throw an error and replace pcall functions (pcall, xpcall, coroutine.resume) with ones that check if exiting on return.
    // Todo: Native exceptions are caught by pcall, so longjmp is the only option other than intercepting pcall.
    auto cranked = Cranked::fromLuaContext(luaState);
    if (cranked->state == Cranked::State::Stopped)
        lua_yield(luaState, 0);
    else
        cranked->updateInternals();
}
