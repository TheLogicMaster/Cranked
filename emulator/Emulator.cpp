#include "Emulator.hpp"

//static bool hookUnmappedFetch(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
//    printf(">>> Unmapped fetch at 0x%" PRIx64 ", block size = 0x%x\n", address, size);
//    return false;
//}
//
//static bool hookUnmappedRead(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
//    printf(">>> Unmapped read at 0x%" PRIx64 "\n", address);
//    return false;
//}
//
//static bool hookUnmappedWrite(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
//    printf(">>> Unmapped write at 0x%" PRIx64 "\n", address);
//    return false;
//}
//
//static void hookRead(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
//    uint64_t data{};
//    uc_mem_read(uc, address, &data, size);
//    if (size > 4)
//        printf(">>> Read at 0x%" PRIx64 " of 0x%" PRIx64 "\n", address, data);
//    else
//        printf(">>> Read at 0x%" PRIx64 " of 0x%" PRIx32 "\n", address, (uint32_t) data);
//}
//
//static void hookCode(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
//    uint32_t data{};
//    uc_mem_read(uc, address, &data, size);
//    printf(">>> Tracing instruction at 0x%" PRIx64 ", instruction size = 0x%x, op: 0x%x\n", address, size, data);
//}
//
//static bool hookProtFetch(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
//    printf(">>> Unmapped prot fetch at 0x%" PRIx64 ", block size = 0x%x\n", address, size);
//    return false;
//}

Emulator::Emulator(InternalUpdateCallback updateCallback, void *userdata) : internalUpdateCallback(updateCallback), internalUserdata(userdata) {
    assertUC(uc_open(UC_ARCH_ARM, UC_MODE_THUMB, &nativeEngine), "Failed to initialize native engine");

    // Enable VFP
    writeRegister(UC_ARM_REG_C1_C0_2, readRegister(UC_ARM_REG_C1_C0_2) | (0xF << 20));
    writeRegister(UC_ARM_REG_FPEXC, 0x40000000);

    // Map memory
    assertUC(uc_mem_map_ptr(nativeEngine, HEAP_ADDRESS, HEAP_SIZE, UC_PROT_READ|UC_PROT_WRITE, heap.baseAddress()), "Failed to map heap memory");
    assertUC(uc_mem_map_ptr(nativeEngine, STACK_ADDRESS, STACK_SIZE, UC_PROT_READ|UC_PROT_WRITE, stackMemory.data()), "Failed to map stack memory");
    assertUC(uc_mem_map_ptr(nativeEngine, API_ADDRESS, API_MEM_SIZE, UC_PROT_READ, apiMemory.data()), "Failed to map API memory");

    // Todo: Remove/consolidate based on core dump needs
//    uc_hook memFetchHook;
//    uc_hook_add(nativeEngine, &memFetchHook, UC_HOOK_MEM_FETCH_UNMAPPED, (void *) hookUnmappedFetch, nullptr, 1, 0);
//    uc_hook memUnmappedReadHook;
//    uc_hook_add(nativeEngine, &memUnmappedReadHook, UC_HOOK_MEM_READ_UNMAPPED, (void *) hookUnmappedRead, nullptr, 1, 0);
//    uc_hook memUnmappedWriteHook;
//    uc_hook_add(nativeEngine, &memUnmappedWriteHook, UC_HOOK_MEM_WRITE_UNMAPPED, (void *) hookUnmappedWrite, nullptr, 1, 0);
//    uc_hook memMappedReadHook;
//    uc_hook_add(nativeEngine, &memMappedReadHook, UC_HOOK_MEM_READ, (void *) hookRead, nullptr, 1, 0);
//    uc_hook memProtFetchHook;
//    uc_hook_add(nativeEngine, &memProtFetchHook, UC_HOOK_MEM_FETCH_PROT, (void *) hookProtFetch, nullptr, 1, 0);
//    uc_hook codeHook;
//    uc_hook_add(nativeEngine, &codeHook, UC_HOOK_CODE, (void *) hookCode, nullptr, 1, 0);

// Todo: Disassembly

//    cs_open(CS_ARCH_ARM, CS_MODE_THUMB, &capstone);
//    cs_option(capstone, CS_OPT_DETAIL, CS_OPT_ON);
//    auto count = cs_disasm(capstone, platforms[i].code, platforms[i].size, address, 0, &insn);
}

Emulator::~Emulator() {
    if (nativeEngine)
        uc_close(nativeEngine);
    if (luaInterpreter)
        lua_close(luaInterpreter);
    if (capstone)
        cs_close(&capstone);
}

void Emulator::init() {
    graphics.init();
    files.init();

    startTime = elapsedTimeStart = std::chrono::system_clock::now();

    if (hasNative) {
        apiSize = populatePlaydateApiStruct(apiMemory.data(), rom->sdkVersion.isValid() ? rom->sdkVersion : version);
        writeRegister(UC_ARM_REG_SP, STACK_ADDRESS + STACK_SIZE);
        memcpy(codeMemory.data() + CODE_OFFSET, rom->binary.data(), rom->binary.size());
        memset(codeMemory.data() + CODE_OFFSET + rom->binary.size(), 0, codeMemory.size() - rom->binary.size() - CODE_OFFSET);
        invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                (nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::Init, uint32_t(2));
        if (nativeUpdateCallback != 0) // Don't load Lua chunks in native-only mode
            hasLua = false;
    }

    if (hasLua) {
        luaInterpreter = lua_newstate(luaAllocator, this);
        lua_atpanic(luaInterpreter, [](lua_State *context) -> int {
            auto message = lua_tostring(context, -1);
            throw std::runtime_error(std::string("Lua panic: ") + (message ? message : ""));
        });
        luaL_openlibs(luaInterpreter);
        registerLuaGlobals();
        lua_settop(luaInterpreter, 0);
        lua_sethook(luaInterpreter, luaHook, LUA_MASKCOUNT, 10000); // Todo: Tune count param to a few milliseconds or so

        // Create update loop coroutine
        luaUpdateThread = lua_newthread(luaInterpreter);
        setQualifiedLuaGlobal("cranked.updateThread");

        if (hasNative)
            invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                    (nativeEventCallback, toVirtualAddress(apiMemory.data()), (int32_t) PDSystemEvent::InitLua, uint32_t(2));

        constexpr const char *PRELOADED_SOURCES[] { "sprites.lua", "nineslice.lua", "graphics.lua" }; // Preload so that functions that treat objects as `userdata` can be shimmed
        Rom::File *lastLuaFile{}; // Assuming main Lua file is always last
        for (auto &file : rom->pdzFiles)
            if (file.type == Rom::FileType::LUAC) {
                for (auto source : PRELOADED_SOURCES)
                    if (file.name == source) {
                        luaL_loadbuffer(luaInterpreter, (char *) file.data.data(), file.data.size(), file.name.c_str());
                        if (int result = lua_pcall(luaInterpreter, 0, 0, 0); result != LUA_OK)
                            throw std::runtime_error("Failed to load '" + file.name + "': " + getLuaError(result));
                        loadedLuaFiles.emplace(source, source + strlen(source) - 4);
                        break;
                    }
                lastLuaFile = &file;
            }
        if (!lastLuaFile)
            throw std::runtime_error("Failed to find main Lua file");
        luaL_loadbuffer(luaInterpreter, (char *) lastLuaFile->data.data(), lastLuaFile->data.size(), lastLuaFile->name.c_str());
        if (int result = lua_pcall(luaInterpreter, 0, 0, 0); result != LUA_OK)
            throw std::runtime_error("Failed to load '" + lastLuaFile->name + "': " + getLuaError(result));
        lua_settop(luaInterpreter, 0);
    }

    initialized = true;
}

void Emulator::load(const std::string &path) {
    reset();

    unload();
    rom = std::make_unique<Rom>(path, this);
    rom->load();

    hasLua = rom->hasLua;
    hasNative = rom->binarySize > 0;
    nativeEventCallback = rom->eventHandlerAddress;

    if (hasNative) {
        // Code section size includes program address offset, as a multiple of 4KB for UC
        auto size = roundMemorySize(rom->combinedProgramSize + CODE_OFFSET, 4096);
        codeMemory.resize(size);
        // Probably can't use JIT, since the memory page has to be read/writable since relocations being specifically for code or data probably isn't known from PDX
        assertUC(uc_mem_map_ptr(nativeEngine, CODE_PAGE_ADDRESS, size, UC_PROT_READ|UC_PROT_WRITE|UC_PROT_EXEC, codeMemory.data()), "Failed to map program memory");
    }
}

void Emulator::unload() {
    reset();

    if (rom)
        rom->unload();
    if (hasNative) {
        uc_mem_unmap(nativeEngine, CODE_PAGE_ADDRESS, codeMemory.size());
        codeMemory.clear();
    }
    hasLua = false;
    hasNative = false;
}

void Emulator::reset() {
    state = State::Stopped;
    nativeUpdateCallback = 0;
    nativeUpdateUserdata = 0;
    emulatedLuaFunctions.clear();
    initialized = false;
    lastFrameTime = {};
    disableUpdateLoop = false;
    crankSoundsEnabled = true;
    statsInterval = 0;

    nativeContextStackDepth = 0;
    for (auto context : nativeContextStack)
        uc_context_free(context);
    nativeContextStack.clear();

    if (luaInterpreter)
        lua_close(luaInterpreter);
    luaInterpreter = nullptr;

    luaReferences.clear();
    loadedLuaFiles.clear();
    emulatedStringLiterals.clear();
    graphics.reset();
    menu.reset();
    files.reset();

    currentInputs = previousInputs = pressedInputs = releasedInputs = 0;
    crankAngle = previousCrankAngle = 0;
    crankDocked = previousCrankDocked = true;

    heap.reset(); // Reset heap only after objects are freed
}

void Emulator::update() {
    // Todo: Handle `Stopping` state and call trigger corresponding events

    if (state != State::Running)
        return;

    auto currentMillisStart = std::chrono::system_clock::now();

    auto currentTime = std::chrono::system_clock::now();
    if (currentTime > lastFrameTime + std::chrono::milliseconds(int(1 / graphics.framerate))) {
        pressedInputs = currentInputs & ~previousInputs;
        releasedInputs = ~currentInputs & previousInputs;

        if (pressedInputs & int(PDButtons::A))
            buttonDownTimeA = std::chrono::system_clock::now();
        if (pressedInputs & int(PDButtons::B))
            buttonDownTimeB = std::chrono::system_clock::now();
        if (releasedInputs & int(PDButtons::A))
            buttonDownTimeA = {};
        if (releasedInputs & int(PDButtons::B))
            buttonDownTimeB = {};

        bool menuOpen = menu.isOpen; // Delay a frame when closing menu to prevent duplicate button inputs

        menu.update();
        graphics.update();

        if (!menuOpen) {
            // Call native input events
            if (nativeEventCallback) {
                for (int i = 0; i < 6; i++) {
                    auto bit = 1 << i;
                    if (pressedInputs & bit)
                        invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                                (nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::KeyPressed, uint32_t(bit));
                    if (releasedInputs & bit)
                        invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                                (nativeEventCallback, API_ADDRESS, (int32_t) PDSystemEvent::KeyReleased, uint32_t(bit));
                }
            }

            // Call Lua input events
            if (luaInterpreter) {
                if (pressedInputs & int(PDButtons::A))
                    invokeLuaInputCallback("AButtonDown");
                if (pressedInputs & int(PDButtons::B))
                    invokeLuaInputCallback("BButtonDown");
                if (pressedInputs & int(PDButtons::Down))
                    invokeLuaInputCallback("downButtonDown");
                if (pressedInputs & int(PDButtons::Left))
                    invokeLuaInputCallback("leftButtonDown");
                if (pressedInputs & int(PDButtons::Right))
                    invokeLuaInputCallback("rightButtonDown");
                if (pressedInputs & int(PDButtons::Up))
                    invokeLuaInputCallback("upButtonDown");

                if (releasedInputs & int(PDButtons::A))
                    invokeLuaInputCallback("AButtonUp");
                if (releasedInputs & int(PDButtons::B))
                    invokeLuaInputCallback("BButtonUp");
                if (releasedInputs & int(PDButtons::Down))
                    invokeLuaInputCallback("downButtonUp");
                if (releasedInputs & int(PDButtons::Left))
                    invokeLuaInputCallback("leftButtonUp");
                if (releasedInputs & int(PDButtons::Right))
                    invokeLuaInputCallback("rightButtonUp");
                if (releasedInputs & int(PDButtons::Up))
                    invokeLuaInputCallback("upButtonUp");

                if (buttonDownTimeA.time_since_epoch() != std::chrono::system_clock::time_point::duration::zero() and currentTime > buttonDownTimeA + std::chrono::seconds(1)) {
                    invokeLuaInputCallback("AButtonHeld");
                    buttonDownTimeA = {};
                }
                if (buttonDownTimeB.time_since_epoch() != std::chrono::system_clock::time_point::duration::zero() and currentTime > buttonDownTimeB + std::chrono::seconds(1)) {
                    invokeLuaInputCallback("BButtonHeld");
                    buttonDownTimeB = {};
                }

                if (crankAngle != previousCrankAngle) {
                    auto change = crankAngle - previousCrankAngle;
                    if (invokeLuaInputCallback("cranked", {change, change})) // Todo: `Acceleration`
                        previousCrankAngle = crankAngle;
                }

                if (crankDocked and not previousCrankDocked)
                    invokeLuaInputCallback("crankDocked");
                if (previousCrankDocked and not crankDocked)
                    invokeLuaInputCallback("crankUndocked");
            }

            if (!disableUpdateLoop and currentTime > suspendUpdateLoopUntil) {
                lastFrameTime = currentTime;
                if (nativeUpdateCallback) {
                    if (invokeEmulatedFunction<uint32_t, ArgType::uint32_t, ArgType::uint32_t>(nativeUpdateCallback, nativeUpdateUserdata))
                        graphics.flushDisplayBuffer();
                } else if (luaInterpreter) {
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
                        std::string error(getLuaError(result));
                        inLuaUpdate = false;
                        lua_resetthread(luaUpdateThread);
                        throw std::runtime_error("Failed to run Lua update loop: " + error);
                    }
                    inLuaUpdate = false;

                    graphics.flushDisplayBuffer();
                }
            }
        }

        previousCrankDocked = crankDocked;
        previousInputs = currentInputs;
    }

    updateInternals();

    currentMillis += duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() - currentMillisStart)).count(); // Todo: Does this increase while locked/in-menu?
}

void Emulator::start() {
    if (!rom)
        throw std::runtime_error("Rom not loaded");
    state = State::Running;
    if (!initialized)
        init();
}

void Emulator::stop() {
    if (state != State::Running)
        return;
    state = State::Stopping;
    // Todo: Stop from UC and Lua hooks
}

void Emulator::terminate() {
    state = State::Stopped; // Todo: Maybe a different flag
}

void Emulator::updateInternals() {
    // Todo: Update audio
    if (internalUpdateCallback)
        internalUpdateCallback(this);
}

void *Emulator::luaAllocator(void *userData, void *ptr, [[maybe_unused]] size_t osize, size_t nsize) {
    auto emulator = (Emulator *) userData;
    if (nsize == 0) {
        emulator->heap.free(ptr);
        return nullptr;
    }
    try {
        return emulator->heap.reallocate(ptr, (int) nsize);
    } catch (std::exception &ex) {
        return nullptr;
    }
}

void Emulator::luaHook(lua_State *luaState, [[maybe_unused]] lua_Debug *luaDebug) {
    // Todo: This hook may slow things down, so potentially set the hook with a count of 1 from a different thread (sethook is the only thread-safe Lua function)
    // Todo: Yielding is destructive if there's a native function in the call stack, so probably only do so if emulator is forcefully stopped
    // Todo: Nested hooks are disabled, so a Lua->Hook->...->Lua chain could block the entire program, but probably not a real concern
    // Todo: Could nested hooks be hacked in?
    // Todo: Exceptions thrown in here are likely an issue
    auto emulator = fromLuaContext(luaState);
    if (emulator->state == State::Stopped)
        lua_yield(luaState, 0);
    else
        emulator->updateInternals();
}

void Emulator::nativeFunctionDispatch(int index) {
    struct ArgBuffer {
        uint32_t data[4]; // This must hold enough data to store any native argument
    };

    if (index >= FUNCTION_TABLE_SIZE)
        throw std::runtime_error("Native function out of range");
    auto &metadata = playdateFunctionTable[index];
    auto argTypes = metadata.argTypes;
    auto returnType = metadata.returnType;

    auto isWide = [](ArgType type) {
        return type == ArgType::int64_t or type == ArgType::uint64_t or type == ArgType::double_t;
    };

    // Substitute struct args with 2 or 4 integers, which is sufficient for the current structs
    for (int i = 0; i < argTypes.size(); i++) {
        auto arg = argTypes[i];
        if (arg != ArgType::struct2_t and arg != ArgType::struct4_t)
            continue;
        auto it = argTypes.begin() + i;
        argTypes.erase(it);
        for (int j = 0; j < (arg == ArgType::struct2_t ? 2 : 4); j++)
        argTypes.insert(it, ArgType::int32_t);
    }

    // Varargs only works with format string parameter
    bool isVarargs = false;
    bool isVaList = false;
    cref_t vaListPtr{};
    if (metadata.argTypes.size() >= 2) {
        auto lastType = metadata.argTypes[metadata.argTypes.size() - 1];
        isVarargs = lastType == ArgType::varargs_t;
        isVaList = lastType == ArgType::va_list_t;
    }
    bool isVariadic = isVarargs or isVaList;
    std::vector<ArgType> varargsTypes{};
    const char *formatString;
    if (isVariadic) {
        int offset = 0;
        for (auto type : argTypes) {
            if (type == ArgType::varargs_t or type == ArgType::va_list_t)
                break;
            bool wide = isWide(type);
            if (wide and (offset % 2) != 0)
                offset++;
            offset += wide ? 2 : 1;
        }
        offset -= 1;
        argTypes.pop_back();

        uint32_t virtualAddress;
        if (offset < 4)
            virtualAddress = readRegister(UC_ARM_REG_R0 + offset);
        else
            virtualAddress = virtualRead<uint32_t>(readRegister(UC_ARM_REG_SP) + 4 * (offset - 4));
        formatString = fromVirtualAddress<const char>(virtualAddress);
        offset++;

        if (isVaList) {
            if (offset < 4)
                vaListPtr = readRegister(UC_ARM_REG_R0 + offset);
            else
                vaListPtr = virtualRead<uint32_t>(readRegister(UC_ARM_REG_SP) + 4 * (offset - 4));
        }

        const char *c = formatString;
        while (*c) {
            if (*c == '%') {
                const char *start = c;
                c++;
                if (*c == '%') {
                    c++;
                    continue;
                }
                while (*c and (*c == 'h' or *c == 'l' or *c == 'j' or *c == 'z' or *c == 't' or *c == 'L' or not std::isalpha(*c)))
                    c++;
                if (!*c)
                    break;
                // %[flags][width][.precision][length]specifier
                std::regex regex(R"(%([-+ #0]*)((?:[\d*]+)?)((?:\.[\d*]+)?)([hljztL]*)[diuoxXfFeEgGaAcspn%])");
                std::cmatch match;
                if (std::regex_match(start, c + 1, match, regex)) {
                    auto &width = match[2];
                    auto &precision = match[3];
                    if (width.str().find('*') != std::string::npos)
                        varargsTypes.push_back(ArgType::int32_t);
                    if (precision.str().find('*') != std::string::npos)
                        varargsTypes.push_back(ArgType::int32_t);
                }
                if (*c == 'c')
                    varargsTypes.push_back(*(c - 1) == 'l' ? ArgType::uint16_t : ArgType::uint8_t);
                else if (*c == 's' or *c == 'n' or *c == 'p')
                    varargsTypes.push_back(ArgType::ptr_t);
                else if (*c == 'd' or *c == 'i') {
                    if (*(c - 1) == 'h') {
                        if (*(c - 2) == 'h')
                            varargsTypes.push_back(ArgType::int8_t);
                        else
                            varargsTypes.push_back(ArgType::int16_t);
                    } else if ((*(c - 1) == 'l' and *(c - 2) == 'l') or *(c - 1) == 'j')
                        varargsTypes.push_back(ArgType::int64_t);
                    else if (*(c - 1) == 'z')
                        varargsTypes.push_back(ArgType::uint32_t);
                    else
                        varargsTypes.push_back(ArgType::int32_t);
                }
                else if (*c == 'x' or *c == 'X' or *c == 'u' or *c == 'o') {
                    if (*(c - 1) == 'h') {
                        if (*(c - 2) == 'h')
                            varargsTypes.push_back(ArgType::uint8_t);
                        else
                            varargsTypes.push_back(ArgType::uint16_t);
                    } else if ((*(c - 1) == 'l' and *(c - 2) == 'l') or *(c - 1) == 'j')
                        varargsTypes.push_back(ArgType::uint64_t);
                    else if (*(c - 1) == 't')
                        varargsTypes.push_back(ArgType::int32_t);
                    else
                        varargsTypes.push_back(ArgType::uint32_t);
                }
                else if (*c == 'f' or *c == 'F' or *c == 'e' or *c == 'E' or *c == 'a' or *c == 'A' or *c == 'g' or *c == 'G')
                    varargsTypes.push_back(ArgType::double_t);
                else
                    break;
            }
            c++;
        }
        if (isVarargs)
            argTypes.insert(argTypes.end(), varargsTypes.begin(), varargsTypes.end());
    }

    auto argCount = argTypes.size() + 1 + (isVaList ? (int)varargsTypes.size() : 0);
    std::vector<ffi_type *> ffiArgTypes(argCount);
    std::vector<ArgBuffer> argValues(argCount - 1);
    std::vector<void *> ffiArgs(argCount);

    // Pass `this` as first arg
    ffiArgTypes[0] = &ffi_type_pointer;
    auto emulator = this;
    ffiArgs[0] = &emulator;

    for (int i = 0; i < argCount - 1; i++)
        ffiArgs[i + 1] = argValues[i].data;

    auto argTypeToFFI = [](ArgType type){
        static ffi_type struct2Type, struct4Type;
        if (!struct2Type.elements) {
            static ffi_type *struct2Elements[]{ &ffi_type_sint32, &ffi_type_sint32, nullptr };
            static ffi_type *struct4Elements[]{ &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, nullptr };
            struct2Type.type = FFI_TYPE_STRUCT;
            struct2Type.elements = struct2Elements;
            struct4Type.type = FFI_TYPE_STRUCT;
            struct4Type.elements = struct4Elements;
        }
        switch (type) {
            case ArgType::uint8_t: return &ffi_type_uint8;
            case ArgType::int8_t: return &ffi_type_sint8;
            case ArgType::uint16_t: return &ffi_type_uint16;
            case ArgType::int16_t: return &ffi_type_sint16;
            case ArgType::uint32_t: return &ffi_type_uint32;
            case ArgType::int32_t: return &ffi_type_sint32;
            case ArgType::uint64_t: return &ffi_type_uint64;
            case ArgType::int64_t: return &ffi_type_sint64;
            case ArgType::float_t: return &ffi_type_float;
            case ArgType::double_t: return &ffi_type_double;
            case ArgType::void_t: return &ffi_type_void;
            case ArgType::ptr_t: return &ffi_type_pointer;
            case ArgType::struct2_t: return &struct2Type;
            case ArgType::struct4_t: return &struct4Type;
            default: throw std::runtime_error("Invalid function arg");
        }
    };

    // Todo: Variadic parameters should be either converted to native sizes or use emulated sizes with custom string formatting implementations

    // Read args from registers and stack and set types
    int currentReg = 0;
    int currentFloatReg = 0;
    auto sp = readRegister(UC_ARM_REG_SP);
    for (int i = 0; i < (int)argTypes.size(); i++) {
        auto type = argTypes[i];
        bool wide = isWide(type);
        if ((wide and (isVarargs or type != ArgType::double_t)) and currentReg < 4 and currentReg % 2 != 0)
            currentReg++;
        if (type == ArgType::double_t and currentFloatReg < 16 and currentFloatReg % 2 != 0)
            currentFloatReg++;
        if (type == ArgType::float_t and !isVarargs) {
            if (currentFloatReg >= 16) {
                *(float *) &argValues[i] = virtualRead<float>(sp);
                sp += 4;
            } else
                *(uint32_t *) &argValues[i] = readRegister(UC_ARM_REG_S0 + currentFloatReg++);
        } else if (type == ArgType::double_t and !isVarargs) {
            if (currentFloatReg >= 15) {
                *(double *) &argValues[i] = virtualRead<double>(sp);
                sp += 8;
            } else {
                assertUC(uc_reg_read(nativeEngine, UC_ARM_REG_D0 + currentFloatReg / 2, &argValues[i]), "Register read failed");
                currentFloatReg += 2;
            }
        } else if (currentReg >= 4 or (wide and currentReg >= 3)) {
            // Stack accesses are 4 byte aligned, so there may be garbage in upper bytes, but FFI only reads required bytes, so it's fine to copy full 32-bit values
            if (wide) {
                *(uint64_t *) &argValues[i] = virtualRead<uint64_t>(sp);
                sp += 8;
            } else if (type == ArgType::ptr_t) {
                *(void **) &argValues[i] = fromVirtualAddress(virtualRead<uint32_t>(sp)); // Pass pointers in native address space
                sp += 4;
            } else {
                *(uint32_t *) &argValues[i] = virtualRead<uint32_t>(sp);
                sp += 4;
            }
        } else {
            // Upper bytes will always be zeroed/sign-extended, so there's no distinction between uint32_t and smaller
            if (wide)
                *(uint64_t *) &argValues[i] = uint64_t(readRegister(UC_ARM_REG_R0 + currentReg + 1)) << 32 | readRegister(UC_ARM_REG_R0 + currentReg);
            else if (type == ArgType::ptr_t)
                *(void **) &argValues[i] = fromVirtualAddress(readRegister(UC_ARM_REG_R0 + currentReg));
            else
                *(uint32_t *) &argValues[i] = readRegister(UC_ARM_REG_R0 + currentReg);
            currentReg += wide ? 2 : 1;
        }

        ffiArgTypes[i + 1] = argTypeToFFI(type);
    }

    if (isVaList)
        for (int i = 0; i < (int)varargsTypes.size(); i++) {
            auto type = varargsTypes[i];
            ffiArgTypes[argTypes.size() + 1 + i] = argTypeToFFI(type);
            int valueIndex = (int)argTypes.size() + i;
            switch (type) {
                case ArgType::uint8_t:
                case ArgType::int8_t:
                    *(uint8_t *) &argValues[valueIndex] = virtualRead<uint8_t>(vaListPtr);
                    vaListPtr += 1;
                    break;
                case ArgType::uint16_t:
                case ArgType::int16_t:
                    *(uint16_t *) &argValues[valueIndex] = virtualRead<uint16_t>(vaListPtr);
                    vaListPtr += 2;
                    break;
                case ArgType::uint32_t:
                case ArgType::int32_t:
                case ArgType::float_t:
                case ArgType::ptr_t:
                    *(uint32_t *) &argValues[valueIndex] = virtualRead<uint32_t>(vaListPtr);
                    vaListPtr += 4;
                    break;
                case ArgType::uint64_t:
                case ArgType::int64_t:
                case ArgType::double_t:
                    *(uint64_t *) &argValues[valueIndex] = virtualRead<uint64_t>(vaListPtr);
                    vaListPtr += 8;
                    break;
                default:
                    throw std::runtime_error("Unsupported va_list type");
            }
        }

    // Set return type
    auto ffiReturnType = argTypeToFFI(returnType);
    bool wideReturn = isWide(returnType);

    // Call function
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argCount, ffiReturnType, ffiArgTypes.data()) != FFI_OK)
        throw std::runtime_error("Failed to prep FFI CIF");
    ArgBuffer returnValue{};
    ffi_call(&cif, (void (*)()) metadata.func, returnValue.data, ffiArgs.data());
    // Todo: Exception handling to restore state?

    // Push return type
    if (returnType == ArgType::float_t and !isVarargs)
        writeRegister(UC_ARM_REG_S0, returnValue.data[0]);
    else if (returnType == ArgType::double_t and !isVarargs)
        assertUC(uc_reg_write(nativeEngine, UC_ARM_REG_D0, returnValue.data), "Register write failed");
    else if (wideReturn) {
        writeRegister(UC_ARM_REG_R0 + 0, returnValue.data[0]);
        writeRegister(UC_ARM_REG_R0 + 1, returnValue.data[1]);
    } else if (returnType == ArgType::ptr_t)
        writeRegister(UC_ARM_REG_R0, toVirtualAddress(*(void **) returnValue.data));
    else if (returnType == ArgType::struct2_t or returnType == ArgType::struct4_t) {
        for (int i = 0; i < (returnType == ArgType::struct2_t ? 2 : 4); i++)
            writeRegister(UC_ARM_REG_R0 + i, returnValue.data[i]);
    } else if (returnType != ArgType::void_t)
        writeRegister(UC_ARM_REG_R0, returnValue.data[0]);
}
