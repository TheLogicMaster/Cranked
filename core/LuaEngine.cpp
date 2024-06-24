#include "LuaEngine.hpp"
#include "Cranked.hpp"

using namespace cranked;

LuaEngine::LuaEngine(Cranked &cranked) : cranked(cranked) {}

LuaEngine::~LuaEngine() {
    if (luaInterpreter)
        lua_close(luaInterpreter);
}

void LuaEngine::init() {
    if (!cranked.rom->hasLua or (cranked.nativeEngine.isLoaded() and cranked.nativeEngine.hasUpdateCallback()))
        return; // Don't load Lua chunks in native-only mode

    luaInterpreter = lua_newstate(luaAllocator, this);
    lua_atpanic(luaInterpreter, [](lua_State *context) -> int {
        auto message = lua_tostring(context, -1);
        throw CrankedError("Lua panic: {}", message ? message : "");
    });
    luaL_openlibs(luaInterpreter);
    registerLuaGlobals();
    lua_settop(luaInterpreter, 0);
    lua_sethook(luaInterpreter, luaHook, LUA_MASKCOUNT, 10000); // Todo: Tune count param to a few milliseconds or so

    // Create update loop coroutine
    luaUpdateThread = lua_newthread(luaInterpreter);
    setQualifiedLuaGlobal("cranked.updateThread");

    if (cranked.nativeEngine.isLoaded())
        cranked.nativeEngine.invokeEventCallback(PDSystemEvent::InitLua, 2);

    constexpr const char *PRELOADED_SOURCES[] { "sprites.lua", "nineslice.lua", "graphics.lua" }; // Preload so that functions that treat objects as `userdata` can be shimmed
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

    luaReferences.clear();
    loadedLuaFiles.clear();
}

void LuaEngine::runUpdateLoop() {
    getQualifiedLuaGlobal("playdate.update");
    lua_xmove(luaInterpreter, luaUpdateThread, 1);
    int results{};
    inLuaUpdate = true;
    auto result = lua_resume(luaUpdateThread, nullptr, 0, &results);
    if (results > 0 and result == LUA_YIELD) // Ignore anything returned
        lua_pop(luaUpdateThread, results);
    if (result == LUA_OK)
        lua_resetthread(luaUpdateThread);
    else if (result != LUA_YIELD) {
        string error(getLuaError(result));
        inLuaUpdate = false;
        lua_resetthread(luaUpdateThread);
        throw CrankedError("Failed to run Lua update loop: {}", error);
    }
    inLuaUpdate = false;
}

void *LuaEngine::luaAllocator(void *userData, void *ptr, size_t osize, size_t nsize) {
    auto cranked = (Cranked *) userData;
    if (nsize == 0) {
        cranked->heap.free(ptr);
        return nullptr;
    }
    try {
        return cranked->heap.reallocate(ptr, (int) nsize);
    } catch (exception &ex) {
        return nullptr;
    }
}

void LuaEngine::luaHook(lua_State *luaState, lua_Debug *luaDebug) {
    // Todo: This hook may slow things down, so potentially set the hook with a count of 1 from a different thread (sethook is the only thread-safe Lua function)
    // Todo: Yielding is destructive if there's a native function in the call stack, so probably only do so if emulator is forcefully stopped
    // Todo: Nested hooks are disabled, so a Lua->Hook->...->Lua chain could block the entire program, but probably not a real concern
    // Todo: Could nested hooks be hacked in?
    // Todo: Exceptions thrown in here are likely an issue
    auto cranked = Cranked::fromLuaContext(luaState);
    if (cranked->state == Cranked::State::Stopped)
        lua_yield(luaState, 0);
    else
        cranked->updateInternals();
}
