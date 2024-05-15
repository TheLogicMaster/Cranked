#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Constants.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

namespace cranked {

    class Cranked;

    class LuaEngine {
    public:
        explicit LuaEngine(Cranked *cranked);
        ~LuaEngine();

        void init();
        void reset();

        void runUpdateLoop();

        lua_State *getContext() {
            return inLuaUpdate ? luaUpdateThread : luaInterpreter;
        }

        bool isLoaded() {
            return luaInterpreter != nullptr;
        }

        inline std::string getLuaError(int result = LUA_ERRRUN) {
            switch (result) {
                case -1:
                    return "Native exception";
                case LUA_OK:
                    return "Ok";
                case LUA_YIELD:
                    return "Yield";
                case LUA_ERRRUN: {
                    auto message = lua_tostring(getContext(), -1);
                    return message ? message : "Invalid error message";
                }
                case LUA_ERRSYNTAX:
                    return "Syntax error";
                case LUA_ERRMEM:
                    return "Memory allocation error";
                case LUA_ERRERR:
                    return "Error running message handler";
                case LUA_ERRFILE:
                    return "File error";
                default:
                    return "Unknown";
            }
        }

        inline void setQualifiedLuaGlobal(const char *name) {
            auto nameStr = std::string(name);
            size_t pos;
            auto luaContext = getContext();
            lua_getglobal(luaContext, "_G");
            while ((pos = nameStr.find('.')) != std::string::npos) {
                auto currentName = nameStr.substr(0, pos);
                lua_getfield(luaContext, -1, currentName.c_str());
                if (lua_isnil(luaContext, -1)) {
                    lua_pop(luaContext, 1);
                    lua_createtable(luaContext, 0, 0);
                    lua_pushvalue(luaContext, -1);
                    lua_setfield(luaContext, -3, currentName.c_str());
                }
                lua_copy(luaContext, -1, -2);
                lua_pop(luaContext, 1);
                nameStr.erase(0, pos + 1);
            }
            lua_pushvalue(luaContext, -2);
            lua_setfield(luaContext, -2, nameStr.c_str());
            lua_pop(luaContext, 2);
        }

        inline void getQualifiedLuaGlobal(const char *name, bool createMissing = false) {
            auto nameStr = std::string(name);
            size_t pos;
            auto luaContext = getContext();
            lua_getglobal(luaContext, "_G");
            std::string currentName;
            auto getField = [&](bool last) {
                lua_getfield(luaContext, -1, currentName.c_str());
                auto missing = lua_isnil(luaContext, -1) and !last;
                if (missing and createMissing) {
                    lua_pop(luaContext, 1);
                    lua_createtable(luaContext, 0, 0);
                    lua_pushvalue(luaContext, -1);
                    lua_setfield(luaContext, -3, currentName.c_str());
                }
                lua_copy(luaContext, -1, -2);
                lua_pop(luaContext, 1);
                return missing and not createMissing;
            };
            while ((pos = nameStr.find('.')) != std::string::npos) {
                currentName = nameStr.substr(0, pos);
                if (getField(false))
                    return;
                nameStr.erase(0, pos + 1);
            }
            currentName = nameStr;
            getField(true);
        }

        inline void invokeLuaCallback(const std::string &name) {
            if (!luaInterpreter)
                return;
            auto start = lua_gettop(getContext());
            lua_getglobal(getContext(), "playdate");
            lua_getfield(getContext(), -1, name.c_str());
            if (!lua_isnil(getContext(), -1))
                lua_call(getContext(), 0, 0); // Todo: Protected?
            lua_settop(getContext(), start);
        }

        inline bool invokeLuaInputCallback(const std::string &name, const std::vector<float> &args = {}) {
            if (!luaInterpreter)
                return false;
            auto start = lua_gettop(getContext());
            lua_getglobal(getContext(), "cranked");
            lua_getfield(getContext(), -1, "dispatchInputEvent");
            lua_pushstring(getContext(), name.c_str());
            for (float arg: args)
                lua_pushnumber(getContext(), arg);
            lua_call(getContext(), args.size() + 1, 1); // Todo: Protected?
            bool handled = lua_toboolean(getContext(), -1);
            lua_settop(getContext(), start);
            return handled;
        }

        /**
         * Preserves a given Lua value associated with a native resource using a reference counter
         */
        template<typename T>
        inline void preserveLuaReference(T *address, int index) {
            if (!luaInterpreter)
                return;
            if (luaReferences[address]++ == 0) {
                getQualifiedLuaGlobal("cranked.references");
                lua_pushvalue(getContext(), index);
                lua_rawsetp(getContext(), -2, address);
                lua_pop(getContext(), 1);
            }
        }

        template<typename T>
        inline bool releaseLuaReference(T *address) {
            // Up to 3 map accesses isn't the most efficient, but shouldn't be too often
            if (!luaInterpreter or !address or !luaReferences.contains(address))
                return false;
            if (--luaReferences[address] == 0) {
                getQualifiedLuaGlobal("cranked.references");
                lua_pushnil(getContext());
                lua_rawsetp(getContext(), -2, address);
                lua_pop(getContext(), 1);
                luaReferences.erase(address);
                return true;
            }
            return false;
        }

        std::unordered_set<std::string> loadedLuaFiles; // Todo: Should be made private

    private:
        static void *luaAllocator(void *ud, void *ptr, size_t osize, size_t nsize);

        static void luaHook(lua_State *luaState, lua_Debug *luaDebug);

        void registerLuaGlobals();

        Cranked *cranked;
        lua_State *luaInterpreter{};
        lua_State *luaUpdateThread{};
        bool inLuaUpdate{};
        std::map<void *, int> luaReferences; // Internal preserved Lua resources with reference count
    };
}
