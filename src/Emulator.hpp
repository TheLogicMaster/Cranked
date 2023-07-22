#pragma once

#include "HeapAllocator.hpp"
#include "Rom.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include "gen/PlaydateAPI.hpp"
#include "PlaydateTypes.hpp"
#include "Graphics.hpp"
#include "File.hpp"
#include "Menu.hpp"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "unicorn/unicorn.h"
#include "nlohmann/json.hpp"
#include <capstone/platform.h>
#include <capstone/capstone.h>
#include <ffi.h>

#include <utility>
#include <string>
#include <memory>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <regex>
#include <optional>
#include <vector>
#include <string>

class Emulator {
public:
    typedef void(* InternalUpdateCallback)(Emulator *emulator);

    enum class State {
        Stopped,
        Running,
        Stopping,
    };

    explicit Emulator(InternalUpdateCallback updateCallback);
    Emulator(const Emulator &) = delete;
    Emulator & operator=(const Emulator &) = delete;
    ~Emulator();
    void load(std::shared_ptr<Rom> program);
    void unload();
    void reset();
    void start();
    void update();
    void stop();
    void terminate();

    template<typename T = void>
    inline T *fromVirtualAddress(cref_t address, bool throws = true) {
        if (address == 0)
            return nullptr;
        intptr_t offset;
        if (address >= HEAP_ADDRESS and address < HEAP_ADDRESS + HEAP_SIZE)
            offset = (intptr_t) heap.baseAddress() - HEAP_ADDRESS;
        else if (address >= STACK_ADDRESS and address < STACK_ADDRESS + STACK_SIZE)
            offset = (intptr_t) stackMemory.data() - STACK_ADDRESS;
        else if (address >= CODE_PAGE_ADDRESS and address < CODE_PAGE_ADDRESS + codeMemory.size())
            offset = (intptr_t) codeMemory.data() - CODE_PAGE_ADDRESS;
        else if (address >= API_ADDRESS and address < API_ADDRESS + apiSize)
            offset = (intptr_t) apiMemory.data() - API_ADDRESS;
        else {
            if (throws)
                throw std::runtime_error("Invalid native address");
            else
                return (T *) (intptr_t) -1;
        }
        return (T *) (address + offset);
    }

    template<typename T>
    inline cref_t toVirtualAddress(T *ptr, bool throws = true) {
        if (ptr == nullptr)
            return 0;
        intptr_t offset;
        auto address = (intptr_t) ptr;
        if (address >= (intptr_t) heap.baseAddress() and address < (intptr_t) heap.baseAddress() + HEAP_SIZE)
            offset = HEAP_ADDRESS - (intptr_t) heap.baseAddress();
        else if (address >= (intptr_t) stackMemory.data() and address < (intptr_t) stackMemory.data() + STACK_SIZE)
            offset = STACK_ADDRESS - (intptr_t) stackMemory.data();
        else if (address >= (intptr_t) codeMemory.data() and address < (intptr_t) codeMemory.data() + codeMemory.size())
            offset = CODE_PAGE_ADDRESS - (intptr_t) codeMemory.data();
        else if (address >= (intptr_t) apiMemory.data() and address < (intptr_t) apiMemory.data() + apiSize)
            offset = API_ADDRESS - (intptr_t) apiMemory.data();
        else {
            if (throws)
                throw std::runtime_error("Invalid native address");
            else
                return (cref_t) -1;
        }
        return address + offset;
    }

    template <typename S>
    inline void virtualWrite(uint32_t address, S value) {
        assertUC(uc_mem_write(nativeEngine, address, &value, sizeof(S)), "Mem write failed");
    }

    template <typename S>
    inline S virtualRead(uint32_t address) {
        S value;
        assertUC(uc_mem_read(nativeEngine, address, &value, sizeof(S)), "Mem write failed");
        return value;
    }

    /**
     * All passed args have to match the expected size exactly, so explicitly cast `int` literals to `int32_t`, for example
     * The emulated CPU context must be preserved by the caller if already running emulated code (For Lua emulated native C functions and such)
     */
    template<typename R, ArgType N, ArgType... A, typename... P>
    inline R invokeEmulatedFunction(uint32_t address, [[maybe_unused]] P... params) {
        if (!hasNative)
            throw std::runtime_error("Native binary not loaded");

        if (++nativeContextStackDepth > 1) {
            if (nativeContextStack.size() < nativeContextStackDepth - 1)
                uc_context_alloc(nativeEngine, &nativeContextStack.emplace_back(nullptr));
            uc_context_save(nativeEngine, nativeContextStack[nativeContextStackDepth - 2]);
        }

        auto restoreContext = [&]{
            if (--nativeContextStackDepth > 0)
                uc_context_restore(nativeEngine, nativeContextStack[nativeContextStackDepth - 1]);
        };

        try {
            auto sp = readRegister(UC_ARM_REG_SP);

            [[maybe_unused]] constexpr ArgType args[]{A...};
            [[maybe_unused]] int i{};
            [[maybe_unused]] int currentReg{};
            [[maybe_unused]] int currentFloatReg{};
            ([&] { // C++ fold magic
                const auto param = params;
                if (i >= sizeof...(A))
                    throw std::runtime_error("Argument out of range");
                auto type = args[i];
                bool wide = type == ArgType::int64_t or type == ArgType::uint64_t;
                if (wide and currentReg < 4 and (currentReg % 2) != 0)
                    currentReg++;
                if (type == ArgType::double_t and currentFloatReg < 16 and (currentFloatReg % 2) != 0)
                    currentFloatReg++;
                if (type == ArgType::float_t) {
                    if (currentFloatReg >= 16) {
                        sp -= sizeof(uint32_t);
                        virtualWrite(sp, *(uint32_t *) &param);
                    } else
                        writeRegister(UC_ARM_REG_S0 + currentFloatReg++, *(uint32_t *) &param);
                } else if (type == ArgType::double_t) {
                    if (currentFloatReg >= 15) {
                        assertUC(uc_mem_write(nativeEngine, sp, &param, sizeof(double)), "Mem write failed");
                        sp -= sizeof(double);
                    } else {
                        assertUC(uc_reg_write(nativeEngine, UC_ARM_REG_D0 + currentFloatReg / 2, &param), "Register write failed");
                        currentFloatReg += 2;
                    }
                } else if (currentReg >= 4 or (wide and currentReg > 2) or (type == ArgType::struct2_t and currentReg > 2) or (type == ArgType::struct4_t and currentReg > 0)) {
                    // Todo: Technically SP is required to be 8 byte aligned at the boundary here, so this may be an issue
                    if (type == ArgType::ptr_t) {
                        sp -= sizeof(uint32_t);
                        virtualWrite(sp, toVirtualAddress(*(void **) &param));
                    } else if (wide or type == ArgType::struct2_t) {
                        for (int i = 0; i < 2; i++) {
                            sp -= sizeof(uint32_t);
                            virtualWrite(sp, ((uint32_t *) &param)[i]);
                        }
                    } else if (type == ArgType::struct4_t) {
                        for (int i = 0; i < 4; i++) {
                            sp -= sizeof(uint32_t);
                            virtualWrite(sp, ((uint32_t *) &param)[i]);
                        }
                    } else {
                        sp -= sizeof(uint32_t);
                        virtualWrite(sp, *(uint32_t *) &param);
                    }
                } else {
                    if (type == ArgType::ptr_t)
                        writeRegister(UC_ARM_REG_R0 + currentReg++, toVirtualAddress(*(void **) &param));
                    else if (wide or type == ArgType::struct2_t) {
                        for (int i = 0; i < 2; i++)
                            writeRegister(UC_ARM_REG_R0 + currentReg + i, ((uint32_t *) &param)[i]);
                        currentReg += 2;
                    } else if (type == ArgType::struct4_t) {
                        for (int i = 0; i < 4; i++)
                            writeRegister(UC_ARM_REG_R0 + currentReg + i, ((uint32_t *) &param)[i]);
                        currentReg += 4;
                    } else
                        writeRegister(UC_ARM_REG_R0 + currentReg++, *(uint32_t *) &param);
                }
                i++;
            }(), ...);

            auto returnAddress = CODE_PAGE_ADDRESS;
            writeRegister(UC_ARM_REG_LR, returnAddress);
            writeRegister(UC_ARM_REG_SP, sp);

            uint32_t pc = address;
            while (true) {
                auto result = uc_emu_start(nativeEngine, pc | 0x1, returnAddress, 15000, 0); // Todo: Verify/fix timing
                pc = readRegister(UC_ARM_REG_PC);
                if (result == UC_ERR_OK) {
                    if (pc == returnAddress)
                        break;
                    updateInternals();
                } else if (result == UC_ERR_FETCH_UNMAPPED and pc >= FUNC_TABLE_ADDRESS and pc < FUNC_TABLE_ADDRESS + FUNCTION_TABLE_SIZE * 2) {
                    uint32_t lr = readRegister(UC_ARM_REG_LR);
                    nativeFunctionDispatch(int(pc - FUNC_TABLE_ADDRESS) / 2);
                    pc = lr;
                } else
                    assertUC(result, "Error invoking emulated function"); // Todo: Add more context for memory errors, probably need to store info from hooks, maybe full core dump
            }
        } catch (std::exception &ex) {
            restoreContext();
            throw;
        }

        constexpr bool returnWide = N == ArgType::int64_t or N == ArgType::uint64_t;
        if constexpr (N == ArgType::float_t) {
            auto value = readRegister(UC_ARM_REG_S0);
            restoreContext();
            return *(R *) &value;
        } else if constexpr (N == ArgType::double_t) {
            double value;
            assertUC(uc_reg_read(nativeEngine, UC_ARM_REG_D0, &value), "Register read failed");
            restoreContext();
            return value;
        } else if constexpr (returnWide) {
            auto value = (uint64_t) readRegister(UC_ARM_REG_R1) << 32 | readRegister(UC_ARM_REG_R0);
            restoreContext();
            return *(R *) &value;
        } else if constexpr (N == ArgType::ptr_t) {
            auto ptr = fromVirtualAddress(readRegister(UC_ARM_REG_R0));
            restoreContext();
            return (R) ptr;
        } else if constexpr (N == ArgType::void_t) {
            restoreContext();
            return;
        } else if constexpr (N == ArgType::struct2_t) {
            uint32_t value[2];
            for (int j = 0; j < 2; j++)
                value[j] = readRegister(UC_ARM_REG_R0 + j);
            restoreContext();
            return *(R *) value;
        } else if constexpr (N == ArgType::struct4_t) {
            uint32_t value[4];
            for (int j = 0; j < 4; j++)
                value[j] = readRegister(UC_ARM_REG_R0 + j);
            restoreContext();
            return *(R *) value;
        } else {
            auto value = readRegister(UC_ARM_REG_R0);
            restoreContext();
            return *(R *) &value;
        }
    }

    inline std::string getLuaError() {
        auto message = lua_tostring(getLuaContext(), -1);
        return message ? message : "Invalid error message";
    }

    inline void setQualifiedLuaGlobal(const char *name) {
        auto nameStr = std::string(name);
        size_t pos;
        auto luaContext = getLuaContext();
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

    inline void getQualifiedLuaGlobal(const char *name, bool createMissing = false)  {
        auto nameStr = std::string(name);
        size_t pos;
        auto luaContext = getLuaContext();
        lua_getglobal(luaContext, "_G");
        std::string currentName;
        auto getField = [&](bool last){
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
        auto start = lua_gettop(getLuaContext());
        lua_getglobal(getLuaContext(), "playdate");
        lua_getfield(getLuaContext(), -1, name.c_str());
        if (!lua_isnil(getLuaContext(), -1))
            lua_call(getLuaContext(), 0, 0);
        lua_settop(getLuaContext(), start);
    }

    inline bool invokeLuaInputCallback(const std::string &name, const std::vector<float> &args = {}) {
        if (!luaInterpreter)
            return false;
        auto start = lua_gettop(getLuaContext());
        lua_getglobal(getLuaContext(), "cranked");
        lua_getfield(getLuaContext(), -1, "dispatchInputEvent");
        lua_pushstring(getLuaContext(), name.c_str());
        for (float arg : args)
            lua_pushnumber(getLuaContext(), arg);
        lua_call(getLuaContext(), args.size() + 1, 1);
        bool handled = lua_toboolean(getLuaContext(), -1);
        lua_settop(getLuaContext(), start);
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
            lua_pushvalue(getLuaContext(), index);
            lua_rawsetp(getLuaContext(), -2, address);
            lua_pop(getLuaContext(), 1);
        }
    }

    template<typename T>
    inline bool releaseLuaReference(T *address) {
        // Up to 3 map accesses isn't the most efficient, but shouldn't be too often
        if (!luaInterpreter or !address or !luaReferences.contains(address))
            return false;
        if (--luaReferences[address] == 0) {
            getQualifiedLuaGlobal("cranked.references");
            lua_pushnil(getLuaContext());
            lua_rawsetp(getLuaContext(), -2, address);
            lua_pop(getLuaContext(), 1);
            luaReferences.erase(address);
            return true;
        }
        return false;
    }

    inline static Emulator *fromLuaContext(lua_State *context) {
        Emulator *emulator;
        lua_getallocf(context, (void **) &emulator);
        return emulator;
    }

    inline lua_State *getLuaContext() {
        return inLuaUpdate ? luaUpdateThread : luaInterpreter;
    }

    inline cref_t getEmulatedStringLiteral(const std::string &message) {
        try {
            return emulatedStringLiterals.at(message);
        } catch (std::out_of_range &ex) {
            auto ref = toVirtualAddress(heap.allocateString(message.c_str()));
            emulatedStringLiterals[message] = ref;
            return ref;
        }
    }

    inline void pushEmulatedLuaFunction(cref_t func) {
        auto it = std::find(emulatedLuaFunctions.begin(), emulatedLuaFunctions.end(), func);
        int index;
        if (it != emulatedLuaFunctions.end())
            index = int(it - emulatedLuaFunctions.begin());
        else {
            emulatedLuaFunctions.emplace_back(func);
            index = (int) emulatedLuaFunctions.size() - 1;
        }
        lua_pushinteger(getLuaContext(), index);
        lua_pushcclosure(getLuaContext(), luaEmulatedFunctionDispatch, 1);
    }

    /**
     * Gets the current crank angle change, resetting the previous angle value
     */
    float getCrankChange() {
        auto change = crankAngle - previousCrankAngle;
        previousCrankAngle = crankAngle;
        return change;
    }

    Graphics graphics = Graphics(this);
    File files = File(this);
    Menu menu = Menu(this);
    std::shared_ptr<Rom> rom;
    HeapAllocator heap = HeapAllocator(HEAP_SIZE);
    std::string luaWrapperError;
    std::unordered_map<std::string, cref_t> emulatedStringLiterals;
    std::unordered_set<std::string> loadedLuaFiles;

    std::chrono::system_clock::time_point elapsedTimeStart, startTime;
    uint32_t currentMillis{}; // Milliseconds elapsed while game is running
    std::chrono::system_clock::time_point lastFrameTime;
    int32_t currentInputs{}, previousInputs{};
    int32_t pressedInputs{}, releasedInputs{};
    std::chrono::system_clock::time_point buttonDownTimeA, buttonDownTimeB;
    PDLanguage systemLanguage{};
    float crankAngle{}, previousCrankAngle{};
    bool crankDocked{}, previousCrankDocked{};
    bool crankSoundsEnabled{};
    bool systemFlipped{};
    bool autoLockDisabled{};
    bool tryReduceFlashing{};
    float accelerometerX{}, accelerometerY{}, accelerometerZ{};
    cref_t nativeUpdateCallback{};
    cref_t nativeUpdateUserdata{};
    std::vector<cref_t> emulatedLuaFunctions;
    std::chrono::system_clock::time_point suspendUpdateLoopUntil;
    bool disableUpdateLoop{};
    int statsInterval{};
    std::map<void *, int> luaReferences; // Internal preserved Lua resources with reference count

    uc_engine *nativeEngine{};
    csh capstone{};
    std::vector<char> apiMemory = std::vector<char>(API_MEM_SIZE);
    bool hasLua{};
    bool hasNative{};
    cref_t nativeEventCallback{};
    InternalUpdateCallback internalUpdateCallback{};
    int apiSize;

    std::vector<char> codeMemory;
    std::vector<char> stackMemory = std::vector<char>(STACK_SIZE);
    bool initialized{};
    State state{};

private:
    static void *luaAllocator(void *ud, void *ptr, size_t osize, size_t nsize);
    static void luaHook(lua_State *luaState, lua_Debug *luaDebug);
    void init();
    void updateInternals();
    void registerLuaGlobals();
    void nativeFunctionDispatch(int index);

    static inline void assertUC(uc_err result, const std::string &message = "Native engine operation failed") {
        if (result != UC_ERR_OK)
            throw std::runtime_error(message + ": " + uc_strerror(result));
    }

    inline void writeRegister(int reg, uint32_t value) {
        assertUC(uc_reg_write(nativeEngine, reg, &value), "Register write failed");
    }

    inline uint32_t readRegister(int reg) {
        uint32_t value;
        assertUC(uc_reg_read(nativeEngine, reg, &value), "Register read failed");
        return value;
    }

    inline static int luaEmulatedFunctionDispatch(lua_State *luaContext) {
        auto emulator = fromLuaContext(luaContext);
        int index = lua_tointeger(luaContext, lua_upvalueindex(1));
        int result;
        try {
            // Pass null for lua context param since it isn't used, with `setSpriteDrawFunction` being a lie and no linkage existing for C Lua functions, anyway
            result = emulator->invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t>(emulator->emulatedLuaFunctions[index], 0);
        } catch (std::exception &ex) {
            result = luaL_error(luaContext, "Failed to invoke native Lua function: %s", ex.what());
        }
        return result;
    }

    lua_State *luaInterpreter{};
    lua_State *luaUpdateThread{};
    bool inLuaUpdate{};
    int nativeContextStackDepth{};
    std::vector<uc_context *> nativeContextStack;
};
