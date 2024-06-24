#include "NativeEngine.hpp"
#include "Cranked.hpp"

using namespace cranked;

NativeEngine::NativeEngine(Cranked &cranked) : cranked(cranked), debugger(cranked.debugger), heap(cranked.heap) {
    assertUC(uc_open(UC_ARCH_ARM, UC_MODE_THUMB, &nativeEngine), "Failed to initialize unicorn");
    assertUC(uc_ctl_set_cpu_model(nativeEngine, UC_CPU_ARM_CORTEX_M7));

    // Register hooks
    assertUC(uc_hook_add(nativeEngine, &interruptHook, UC_HOOK_INTR, (void *) handleInterrupt, &cranked, 1, 0), "Failed to add interrupt hook");
    assertUC(uc_hook_add(nativeEngine, &unmappedReadHook, UC_HOOK_MEM_READ_UNMAPPED, (void *) handleUnmappedAccess, &cranked, 1, 0), "Failed to add unmapped read hook");
    assertUC(uc_hook_add(nativeEngine, &unmappedWriteHook, UC_HOOK_MEM_WRITE_UNMAPPED, (void *) handleUnmappedAccess, &cranked, 1, 0), "Failed to add unmapped write hook");

    // Map memory
    assertUC(uc_mem_map_ptr(nativeEngine, HEAP_ADDRESS, HEAP_SIZE, UC_PROT_READ|UC_PROT_WRITE, heap.baseAddress()), "Failed to map heap memory");
    assertUC(uc_mem_map_ptr(nativeEngine, STACK_ADDRESS, STACK_SIZE, UC_PROT_READ|UC_PROT_WRITE, stackMemory.data()), "Failed to map stack memory");
    assertUC(uc_mem_map_ptr(nativeEngine, API_ADDRESS, API_MEM_SIZE, UC_PROT_READ, apiMemory.data()), "Failed to map API memory");
}

NativeEngine::~NativeEngine() {
    assertUC(uc_hook_del(nativeEngine, interruptHook), "Failed to delete interrupt hook");
    assertUC(uc_hook_del(nativeEngine, unmappedReadHook), "Failed to delete unmapped read hook");
    assertUC(uc_hook_del(nativeEngine, unmappedWriteHook), "Failed to delete unmapped write hook");
    assertUC(uc_close(nativeEngine), "Failed to close unicorn");
}

void NativeEngine::init() {
    if (loaded) {
        writeRegister(UC_ARM_REG_SP, STACK_ADDRESS + STACK_SIZE);
        memcpy(codeMemory.data() + CODE_OFFSET, cranked.rom->binary.data(), cranked.rom->binary.size());
        memset(codeMemory.data() + CODE_OFFSET + cranked.rom->binary.size(), 0, codeMemory.size() - cranked.rom->binary.size() - CODE_OFFSET);
        invokeEventCallback(PDSystemEvent::Init, 2);
    }
}

void NativeEngine::reset() {
    nativeUpdateCallback = 0;
    nativeUpdateUserdata = 0;
    emulatedLuaFunctions.clear();

    nativeContextStackDepth = 0;
    for (auto context : nativeContextStack)
        assertUC(uc_context_free(context), "Failed to free unicorn context");
    nativeContextStack.clear();

    emulatedStringLiterals.clear();

    nativeResources.clear();
}

void NativeEngine::load() {
    if (cranked.rom->binarySize <= 0)
        return;
    loaded = true;
    nativeEventCallback = cranked.rom->eventHandlerAddress;
    // Code section size includes program address offset, as a multiple of 4KB for UC
    auto size = roundMemorySize(cranked.rom->combinedProgramSize + CODE_OFFSET, 4096);
    codeMemory.resize(size);
    assertUC(uc_mem_map_ptr(nativeEngine, CODE_PAGE_ADDRESS, size, UC_PROT_READ|UC_PROT_WRITE|UC_PROT_EXEC, codeMemory.data()), "Failed to map program memory");
    apiSize = populatePlaydateApiStruct(apiMemory.data(), cranked.rom->sdkVersion.isValid() ? cranked.rom->sdkVersion : Cranked::version);
}

void NativeEngine::unload() {
    if (!loaded)
        return;
    assertUC(uc_mem_unmap(nativeEngine, CODE_PAGE_ADDRESS, codeMemory.size()), "Failed to unmap code memory");
    codeMemory.clear();
}

void NativeEngine::freeResource(void *ptr) {
    if (nativeResources.contains((NativeResource *) ptr))
        static_cast<NativeResource *>(ptr)->dereference();
    else
        heap.free(ptr);
}

void NativeEngine::pushEmulatedLuaFunction(cref_t func) {
    auto it = find(emulatedLuaFunctions.begin(), emulatedLuaFunctions.end(), func);
    int index;
    if (it != emulatedLuaFunctions.end())
        index = int(it - emulatedLuaFunctions.begin());
    else {
        emulatedLuaFunctions.emplace_back(func);
        index = (int) emulatedLuaFunctions.size() - 1;
    }
    lua_pushinteger(cranked.getLuaContext(), index);
    lua_pushcclosure(cranked.getLuaContext(), luaEmulatedFunctionDispatch, 1);
}

void NativeEngine::updateInternals() {
    cranked.updateInternals();
}

void NativeEngine::handleInterrupt(uc_engine *uc, uint32 interrupt, void *userdata) {
    if (interrupt != BREAKPOINT_INTERRUPT)
        throw NativeExecutionError("Unknown interrupt");
    auto cranked = (Cranked *) userdata;
    cranked->debugger.handleBreakpoint();
    cranked->nativeEngine.assertUC(uc_emu_stop(uc), "Failed to stop unicorn emulation");
    while (cranked->debugger.isHalted()) {
        cranked->updateInternals();
        this_thread::sleep_for(chrono::milliseconds(15));
    }
}

bool NativeEngine::handleUnmappedAccess(uc_engine *uc, uc_mem_type type, uint64 address, int size, int64 value, void *userdata) {
    ((Cranked *) userdata)->nativeEngine.lastBadAccessAddress = (cref_t) address;
    return false;
}

void NativeEngine::nativeFunctionDispatch(int index) {
    struct ArgBuffer {
        uint32 data[4]; // This must hold enough data to store any native argument
    };

    if (index >= FUNCTION_TABLE_SIZE)
        throw NativeExecutionError("Native function out of range");
    auto &metadata = playdateFunctionTable[index];
    auto argTypes = metadata.argTypes;
    auto returnType = metadata.returnType;

    auto isWide = [](ArgType type) {
        return type == ArgType::int64_t or type == ArgType::uint64_t or type == ArgType::double_t;
    };

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
    vector<ArgType> varargsTypes{};
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

        uint32 virtualAddress;
        if (offset < 4)
            virtualAddress = readRegister(UC_ARM_REG_R0 + offset);
        else
            virtualAddress = virtualRead<uint32>(readRegister(UC_ARM_REG_SP) + 4 * (offset - 4));
        formatString = fromVirtualAddress<const char>(virtualAddress);
        offset++;

        if (isVaList) {
            if (offset < 4)
                vaListPtr = readRegister(UC_ARM_REG_R0 + offset);
            else
                vaListPtr = virtualRead<uint32>(readRegister(UC_ARM_REG_SP) + 4 * (offset - 4));
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
                while (*c and (*c == 'h' or *c == 'l' or *c == 'j' or *c == 'z' or *c == 't' or *c == 'L' or not isalpha(*c)))
                    c++;
                if (!*c)
                    break;
                // %[flags][width][.precision][length]specifier
                static regex regex(R"(%([-+ #0]*)((?:[\d*]+)?)((?:\.[\d*]+)?)([hljztL]*)[diuoxXfFeEgGaAcspn%])");
                cmatch match;
                if (regex_match(start, c + 1, match, regex)) {
                    auto &width = match[2];
                    auto &precision = match[3];
                    if (width.str().find('*') != string::npos)
                        varargsTypes.push_back(ArgType::int32_t);
                    if (precision.str().find('*') != string::npos)
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
    vector<ffi_type *> ffiArgTypes(argCount);
    vector<ArgBuffer> argValues(argCount - 1);
    vector<void *> ffiArgs(argCount);

    // Pass `this` as first arg
    ffiArgTypes[0] = &ffi_type_pointer;
    auto emulator = &cranked;
    ffiArgs[0] = &emulator;

    for (int i = 0; i < argCount - 1; i++)
        ffiArgs[i + 1] = argValues[i].data;

    auto argTypeToFFI = [](ArgType type){
        static ffi_type struct2Type, struct4Type, struct4fType;
        if (!struct2Type.elements) {
            static ffi_type *struct2Elements[]{ &ffi_type_sint32, &ffi_type_sint32, nullptr };
            static ffi_type *struct4Elements[]{ &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, &ffi_type_sint32, nullptr };
            static ffi_type *struct4fElements[]{ &ffi_type_float, &ffi_type_float, &ffi_type_float, &ffi_type_float, nullptr };
            struct2Type.type = FFI_TYPE_STRUCT;
            struct2Type.elements = struct2Elements;
            struct4Type.type = FFI_TYPE_STRUCT;
            struct4Type.elements = struct4Elements;
            struct4fType.type = FFI_TYPE_STRUCT;
            struct4fType.elements = struct4fElements;
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
            case ArgType::struct4f_t: return &struct4fType;
            default: throw CrankedError("Invalid function arg");
        }
    };

    // Todo: Variadic parameters should be either converted to native sizes or use emulated sizes with custom string formatting implementations (Passing too many parameters seems to be problematic at present)

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
        if (type == ArgType::struct2_t or type == ArgType::struct4_t) {
            for (int j = 0; j < (type == ArgType::struct2_t ? 2 : 4); j++) {
                uint32 value;
                if (currentReg >= 4) {
                    value = virtualRead<uint32>(sp);
                    sp += 4;
                } else
                    value = readRegister(UC_ARM_REG_R0 + currentReg++);
                argValues[i].data[j] = value;
            }
        } else if (type == ArgType::struct4f_t) {
            for (int j = 0; j < 4; j++) {
                uint32 value;
                if (currentFloatReg >= 16) {
                    value = virtualRead<uint32>(sp);
                    sp += 4;
                } else
                    value = readRegister(UC_ARM_REG_S0 + currentFloatReg++);
                argValues[i].data[j] = value;
            }
        } else if (type == ArgType::float_t and !isVarargs) {
            if (currentFloatReg >= 16) {
                *(float *) &argValues[i] = virtualRead<float>(sp);
                sp += 4;
            } else
                *(uint32 *) &argValues[i] = readRegister(UC_ARM_REG_S0 + currentFloatReg++);
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
                *(uint64 *) &argValues[i] = virtualRead<uint64>(sp);
                sp += 8;
            } else if (type == ArgType::ptr_t) {
                *(void **) &argValues[i] = fromVirtualAddress(virtualRead<uint32>(sp)); // Pass pointers in native address space
                sp += 4;
            } else {
                *(uint32 *) &argValues[i] = virtualRead<uint32>(sp);
                sp += 4;
            }
        } else {
            // Upper bytes will always be zeroed/sign-extended, so there's no distinction between uint32 and smaller
            if (wide)
                *(uint64 *) &argValues[i] = uint64(readRegister(UC_ARM_REG_R0 + currentReg + 1)) << 32 | readRegister(UC_ARM_REG_R0 + currentReg);
            else if (type == ArgType::ptr_t)
                *(void **) &argValues[i] = fromVirtualAddress(readRegister(UC_ARM_REG_R0 + currentReg));
            else
                *(uint32 *) &argValues[i] = readRegister(UC_ARM_REG_R0 + currentReg);
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
                    *(uint8 *) &argValues[valueIndex] = virtualRead<uint8>(vaListPtr);
                    vaListPtr += 1;
                    break;
                case ArgType::uint16_t:
                case ArgType::int16_t:
                    *(uint16 *) &argValues[valueIndex] = virtualRead<uint16>(vaListPtr);
                    vaListPtr += 2;
                    break;
                case ArgType::uint32_t:
                case ArgType::int32_t:
                case ArgType::float_t:
                case ArgType::ptr_t:
                    *(uint32 *) &argValues[valueIndex] = virtualRead<uint32>(vaListPtr);
                    vaListPtr += 4;
                    break;
                case ArgType::uint64_t:
                case ArgType::int64_t:
                case ArgType::double_t:
                    *(uint64 *) &argValues[valueIndex] = virtualRead<uint64>(vaListPtr);
                    vaListPtr += 8;
                    break;
                default:
                    throw CrankedError("Unsupported va_list type");
            }
        }

    // Set return type
    auto ffiReturnType = argTypeToFFI(returnType);
    bool wideReturn = isWide(returnType);

    // Call function
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argCount, ffiReturnType, ffiArgTypes.data()) != FFI_OK)
        throw CrankedError("Failed to prep FFI CIF");
    ArgBuffer returnValue{};
    ffi_call(&cif, (void (*)()) metadata.func, returnValue.data, ffiArgs.data());

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
    } else if (returnType == ArgType::struct4f_t) {
        for (int i = 0; i < 4; i++)
            writeRegister(UC_ARM_REG_S0 + i, returnValue.data[i]);
    } else if (returnType != ArgType::void_t)
        writeRegister(UC_ARM_REG_R0, returnValue.data[0]);
}

string NativeEngine::dumpCore() {
    if (!nativeEngine)
        return "<Native engine not initialized>";
    string dump;
    uint32 pc, reg;
    // Don't bother checking results here
    uc_reg_read(nativeEngine, UC_ARM_REG_PC, &pc);
    dump += format(" PC={:08X} ", pc);
    uc_reg_read(nativeEngine, UC_ARM_REG_SP, &reg);
    dump += format(" SP={:08X} ", reg);
    uc_reg_read(nativeEngine, UC_ARM_REG_LR, &reg);
    dump += format(" LR={:08X} ", reg);
    uc_reg_read(nativeEngine, UC_ARM_REG_XPSR, &reg);
    dump += format(" SR={:08X}\n", reg);
    for (int i = 0; i < 13; i++) {
        uc_reg_read(nativeEngine, UC_ARM_REG_R0 + i, &reg);
        dump += format("R{:02}={:08X} ", i, reg);
        if (i % 4 == 3)
            dump += '\n';
    }
    dump += '\n';
    for (int i = 0; i < 32; i++) {
        uc_reg_read(nativeEngine, UC_ARM_REG_S0 + i, &reg);
        dump += format("S{:02}={:08X} ", i, reg);
        if (i % 4 == 3)
            dump += '\n';
    }
    pc &= ~0x1;
    auto disassembly = cranked.debugger.getDisassembly();
    int disassemblySize = cranked.debugger.getDisassemblySize();
    for (int i = -8; i < 16; i++) {
        cref_t address = pc + i * 2;
        for (int j = 0; j < disassemblySize; j++) {
            auto inst = disassembly[j];
            if (inst.address == address) {
                dump += format("{:08X}: {} {}\n", address, inst.mnemonic, inst.op_str);
                break;
            }
        }
    }
    return dump;
}

int NativeEngine::luaEmulatedFunctionDispatch(lua_State *luaContext) {
    auto cranked = Cranked::fromLuaContext(luaContext);
    int index = lua_tointeger(luaContext, lua_upvalueindex(1));
    int result;
    try {
        // Pass null for lua context param since it isn't used, with `setSpriteDrawFunction` being a lie and no linkage existing for C Lua functions, anyway
        result = cranked->nativeEngine.invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::uint32_t>(cranked->nativeEngine.emulatedLuaFunctions[index], 0);
    } catch (exception &ex) {
        result = luaL_error(luaContext, "Failed to invoke native Lua function: %s", ex.what());
    }
    return result;
}
