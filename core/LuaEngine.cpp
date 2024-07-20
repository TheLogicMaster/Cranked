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

    luaInterpreter = lua_newstate(luaAllocator, &cranked);

    lua_atpanic(luaInterpreter, [](lua_State *context) -> int {
        auto message = lua_tostring(context, -1);
        throw CrankedError("Lua panic: {}", message ? message : "");
    });

    luaL_openlibs(luaInterpreter);
    registerLuaGlobals();
    tracy::LuaRegister(luaInterpreter);

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

template<>
LuaVal LuaEngine::pushResource(NodeGraph resource) {
    return pushUserdataResource(resource, "playdate.pathfinder.graph");
}

template<>
LuaVal LuaEngine::pushResource(GraphNode resource) {
    return pushUserdataResource(resource, "playdate.pathfinder.node");
}

template<>
LuaVal LuaEngine::pushResource(Bitmap resource) {
    return pushUserdataResource(resource, "playdate.graphics.image");
}

template<>
LuaVal LuaEngine::pushResource(BitmapTable resource) {
    return pushUserdataResource(resource, "playdate.graphics.imagetable");
}

template<>
LuaVal LuaEngine::pushResource(TileMap resource) {
    return pushUserdataResource(resource, "playdate.graphics.tilemap");
}

template<>
LuaVal LuaEngine::pushResource(Sprite resource) {
    if (auto value = getResourceValue(resource); not value.isNil())
        return value;
    lua_pop(getContext(), 1);
    auto value = pushUserdataResource(resource, "playdate.graphics.sprite");
    storeResourceValue(resource, value);
    return value;
}

template<>
LuaVal LuaEngine::pushResource(VideoPlayer resource) {
    return pushUserdataResource(resource, "playdate.graphics.video");
}

template<>
LuaVal LuaEngine::pushResource(Font resource) {
    return pushUserdataResource(resource, "playdate.graphics.font");
}

template<>
LuaVal LuaEngine::pushResource(File resource) {
    return pushUserdataResource(resource, "playdate.file.file");
}

template<>
LuaVal LuaEngine::pushResource(MenuItem resource) {
    return pushUserdataResource(resource, "playdate.menu.item");
}

void *LuaEngine::luaAllocator(void *userData, void *ptr, size_t osize, size_t nsize) {
    static const auto luaHeapProfileName = "Lua Heap";
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
    // Todo: Could nested hooks be hacked in?
    // Todo: Exceptions thrown in here are likely an issue
    auto cranked = Cranked::fromLuaContext(luaState);
    if (cranked->state == Cranked::State::Stopped)
        lua_yield(luaState, 0);
    else
        cranked->updateInternals();
}
