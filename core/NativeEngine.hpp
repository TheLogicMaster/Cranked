#pragma once

#include "gen/PlaydateAPI.hpp"
#include "HeapAllocator.hpp"
#include "Debugger.hpp"

// Todo: Dynarmic implementation

namespace cranked {

    class Cranked;

    class NativeEngine {
        friend NativeResource;

    public:
        class NativeExecutionError : public CrankedError {
        public:
            explicit NativeExecutionError(const char *message, string dump = "") : NativeExecutionError(string(message), std::move(dump)) {}

            explicit NativeExecutionError(const string &message, string dump = "") : CrankedError(message), dump(std::move(dump)) {}

            const string &getDump() {
                return dump;
            }
        private:
            string dump;
        };

        explicit NativeEngine(Cranked &cranked);
        ~NativeEngine();

        void init();
        void reset();
        void load();
        void unload();

        bool isLoaded() const {
            return loaded;
        }

        bool hasUpdateCallback() const {
            return nativeUpdateCallback != 0;
        }

        void setUpdateCallback(cref_t callback, cref_t userdata) {
            nativeUpdateCallback = callback;
            nativeUpdateUserdata = userdata;
        }

        int getApiSize() const {
            return apiSize;
        }

        cref_t getEmulatedStringLiteral(const string &message) {
            auto &value = emulatedStringLiterals[message];
            if (!value)
                value = toVirtualAddress(heap.allocateString(message.c_str()));
            return value;
        }

        void pushEmulatedLuaFunction(cref_t func);

        template<typename T = void>
        T *fromVirtualAddress(cref_t address, bool throws = true) {
            // Todo: Ensure value, not just address, is in range
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
                    throw CrankedError("Invalid native address");
                return (T *) (intptr_t) nullptr;
            }
            return (T *) (address + offset);
        }

        template<typename T>
        cref_t toVirtualAddress(T *ptr, bool throws = true) {
            // Todo: Ensure value, not just address, is in range
            if (ptr == nullptr)
                return 0;
            intptr_t offset;
            auto address = (intptr_t) ptr;
            if (address >= (intptr_t) heap.baseAddress() and address < (intptr_t) heap.baseAddress() + HEAP_SIZE)
                offset = HEAP_ADDRESS - (intptr_t) heap.baseAddress();
            else if (address >= (intptr_t) stackMemory.data() and address < (intptr_t) stackMemory.data() + STACK_SIZE)
                offset = STACK_ADDRESS - (intptr_t) stackMemory.data();
            else if (address >= (intptr_t) codeMemory.data() and
                     address < (intptr_t) codeMemory.data() + codeMemory.size())
                offset = CODE_PAGE_ADDRESS - (intptr_t) codeMemory.data();
            else if (address >= (intptr_t) apiMemory.data() and address < (intptr_t) apiMemory.data() + apiSize)
                offset = API_ADDRESS - (intptr_t) apiMemory.data();
            else {
                if (throws)
                    throw CrankedError("Invalid native address");
                return 0;
            }
            return address + offset;
        }

        template<typename S>
        void virtualWrite(uint32 address, S value) {
            assertUC(uc_mem_write(nativeEngine, address, &value, sizeof(S)), "Mem write failed");
        }

        template<typename S>
        S virtualRead(uint32 address) {
            S value;
            assertUC(uc_mem_read(nativeEngine, address, &value, sizeof(S)), "Mem read failed");
            return value;
        }

        void writeRegister(int reg, uint32 value) {
            assertUC(uc_reg_write(nativeEngine, reg, &value), "Register write failed");
        }

        void writeRegister64(int reg, uint64 value) {
            assertUC(uc_reg_write(nativeEngine, reg, &value), "Register write failed");
        }

        uint32 readRegister(int reg) {
            uint32 value;
            assertUC(uc_reg_read(nativeEngine, reg, &value), "Register read failed");
            return value;
        }

        uint64 readRegister64(int reg) {
            uint64 value;
            assertUC(uc_reg_read(nativeEngine, reg, &value), "Register read failed");
            return value;
        }

        void freeResource(void *ptr);

        template<typename T, typename ...Args> requires is_resource_ptr<T>
        T createReferencedResource(Args ...args) {
            T resource = heap.construct<remove_pointer_t<T>>(cranked, args...);
            resource->reference();
            return resource;
        }

        int32 invokeEventCallback(PDSystemEvent event, uint32 arg) {
            return invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                    (nativeEventCallback, toVirtualAddress(apiMemory.data()), (int32) event, arg);
        }

        int32 invokeUpdateCallback() {
            if (!nativeUpdateCallback)
                throw CrankedError("Update callback not set");
            return invokeEmulatedFunction<int32, ArgType::int32_t, ArgType::uint32_t>(nativeUpdateCallback, nativeUpdateUserdata);
        }

        vector<uint8> &getCodeMemory() {
            return codeMemory;
        }

        void flushCaches() {
            assertUC(uc_ctl_flush_tlb(nativeEngine), "Failed to flush TLB");
        }

        /**
         * All passed args have to match the expected size exactly, so explicitly cast `int` literals to `int32`, for example // Todo: Argument type assertions/conversions
         */
        template<typename R, ArgType N, ArgType... A, typename... P>
        R invokeEmulatedFunction(uint32 address, [[maybe_unused]] P... params) {
            // Todo: Is the `R` parameter needed if the return type is auto?
            if (!loaded)
                throw CrankedError("Native binary not loaded");

            if (++nativeContextStackDepth > 1) {
                if (nativeContextStack.size() < nativeContextStackDepth - 1)
                    assertUC(uc_context_alloc(nativeEngine, &nativeContextStack.emplace_back(nullptr)), "Failed to allocate unicorn context");
                assertUC(uc_context_save(nativeEngine, nativeContextStack[nativeContextStackDepth - 2]), "Failed to save unicorn context");
            }

            auto restoreContext = [&] {
                if (--nativeContextStackDepth > 0)
                    assertUC(uc_context_restore(nativeEngine, nativeContextStack[nativeContextStackDepth - 1]), "Failed to restore unicorn context");
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
                        throw CrankedError("Argument out of range");
                    auto type = args[i];
                    bool wide = type == ArgType::int64_t or type == ArgType::uint64_t;
                    if (wide and currentReg < 4 and (currentReg % 2) != 0)
                        currentReg++;
                    if (type == ArgType::double_t and currentFloatReg < 16 and (currentFloatReg % 2) != 0)
                        currentFloatReg++;
                    if (type == ArgType::float_t) {
                        if (currentFloatReg >= 16) {
                            sp -= sizeof(uint32);
                            virtualWrite(sp, *(uint32 *) &param);
                        } else
                            writeRegister(UC_ARM_REG_S0 + currentFloatReg++, *(uint32 *) &param);
                    } else if (type == ArgType::double_t) {
                        if (currentFloatReg >= 15) {
                            assertUC(uc_mem_write(nativeEngine, sp, &param, sizeof(double)), "Mem write failed");
                            sp -= sizeof(double);
                        } else {
                            assertUC(uc_reg_write(nativeEngine, UC_ARM_REG_D0 + currentFloatReg / 2, &param), "Register write failed");
                            currentFloatReg += 2;
                        }
                    } else if (type == ArgType::struct4f_t) {
                        for (int i = 0; i < 4; i++) {
                            if (currentFloatReg >= 16) {
                                sp -= sizeof(uint32);
                                virtualWrite(sp, ((uint32 *) &param)[i]);
                            } else
                                writeRegister(UC_ARM_REG_S0 + currentFloatReg++, ((uint32 *) &param)[i]);
                        }
                    } else if (currentReg >= 4 or (wide and currentReg > 2) or (type == ArgType::struct2_t and currentReg > 2) or (type == ArgType::struct4_t and currentReg > 0)) {
                        // Todo: Technically SP is required to be 8 byte aligned at the boundary here, so this may be an issue
                        if (type == ArgType::ptr_t) {
                            sp -= sizeof(uint32);
                            virtualWrite(sp, toVirtualAddress(*(void **) &param));
                        } else if (wide or type == ArgType::struct2_t) {
                            for (int i = 0; i < 2; i++) {
                                sp -= sizeof(uint32);
                                virtualWrite(sp, ((uint32 *) &param)[i]);
                            }
                        } else if (type == ArgType::struct4_t) {
                            for (int i = 0; i < 4; i++) {
                                sp -= sizeof(uint32);
                                virtualWrite(sp, ((uint32 *) &param)[i]);
                            }
                        } else {
                            sp -= sizeof(uint32);
                            virtualWrite(sp, *(uint32 *) &param);
                        }
                    } else {
                        if (type == ArgType::ptr_t)
                            writeRegister(UC_ARM_REG_R0 + currentReg++, toVirtualAddress(*(void **) &param));
                        else if (wide or type == ArgType::struct2_t) {
                            for (int i = 0; i < 2; i++)
                                writeRegister(UC_ARM_REG_R0 + currentReg + i, ((uint32 *) &param)[i]);
                            currentReg += 2;
                        } else if (type == ArgType::struct4_t) {
                            for (int i = 0; i < 4; i++)
                                writeRegister(UC_ARM_REG_R0 + currentReg + i, ((uint32 *) &param)[i]);
                            currentReg += 4;
                        } else
                            writeRegister(UC_ARM_REG_R0 + currentReg++, *(uint32 *) &param);
                    }
                    i++;
                }()
#ifndef __CLION_IDE__ // Todo: Remove yet another CLion workaround when it is fixed
                , ...
#endif
                );

                // Todo: PC could point to previous native stack PC to help with debugging
                // Todo: Unicorn randomly segfaults when stopping execution, using zero for return address may help?
                auto returnAddress = CODE_PAGE_ADDRESS + 2; // Unicorn freaks out if this is at the code page boundary
                writeRegister(UC_ARM_REG_LR, returnAddress);
                writeRegister(UC_ARM_REG_SP, sp);

                writeRegister(UC_ARM_REG_PC, address | 0x1); // Todo: Helps debugging?

                uint32 pc = address;
                while (true) {
                    bool shouldStep = debugger.handleInvocationDebugging();
                    auto result = uc_emu_start(nativeEngine, pc | 0x1, returnAddress & ~0x1, 15000, shouldStep ? 1 : 0); // Todo: Verify/fix timing
                    pc = readRegister(UC_ARM_REG_PC);
                    if (result == UC_ERR_OK) {
                        if (pc == returnAddress)
                            break;
                        updateInternals();
                    } else if (result == UC_ERR_FETCH_UNMAPPED and pc >= FUNC_TABLE_ADDRESS and pc < FUNC_TABLE_ADDRESS + FUNCTION_TABLE_SIZE * 2) {
                        uint32 lr = readRegister(UC_ARM_REG_LR);
                        nativeFunctionDispatch(int(pc - FUNC_TABLE_ADDRESS) / 2);
                        pc = lr;
                    } else if (result == UC_ERR_FETCH_UNMAPPED or result == UC_ERR_READ_UNMAPPED or result == UC_ERR_WRITE_UNMAPPED) {
                        throw NativeExecutionError(format("Unmapped memory access at {}", lastBadAccessAddress), dumpCore());
                    } else {
                        assertUC(result, "Error invoking emulated function"); // Todo: Add more context for memory errors, probably need to store info from hooks, maybe full core dump
                    }
                }
            } catch (exception &ex) {
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
                auto value = (uint64) readRegister(UC_ARM_REG_R1) << 32 | readRegister(UC_ARM_REG_R0);
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
                uint32 value[2];
                for (int j = 0; j < 2; j++)
                    value[j] = readRegister(UC_ARM_REG_R0 + j);
                restoreContext();
                return *(R *) value;
            } else if constexpr (N == ArgType::struct4_t) {
                uint32 value[4];
                for (int j = 0; j < 4; j++)
                    value[j] = readRegister(UC_ARM_REG_R0 + j);
                restoreContext();
                return *(R *) value;
            } else if constexpr (N == ArgType::struct4_t) {
                uint32 value[4];
                for (int j = 0; j < 4; j++)
                    value[j] = readRegister(UC_ARM_REG_S0 + j);
                restoreContext();
                return *(R *) value;
            }  else {
                auto value = readRegister(UC_ARM_REG_R0);
                restoreContext();
                return *(R *) &value;
            }
        }

    private:
        static constexpr uint32 BREAKPOINT_INTERRUPT = 7;

        void updateInternals();

        static void handleInterrupt(uc_engine *uc, uint32 interrupt, void *userdata);

        static bool handleUnmappedAccess(uc_engine *uc, uc_mem_type type, uint64 address, int size, int64 value, void *userdata);

        void nativeFunctionDispatch(int index);

        string dumpCore();

        void assertUC(uc_err result, const string &message = "Native engine operation failed") {
            if (result != UC_ERR_OK)
                throw NativeExecutionError(message + ": " + uc_strerror(result), dumpCore());
        }

        static int luaEmulatedFunctionDispatch(lua_State *luaContext);

        HeapAllocator &heap;
        Cranked &cranked;
        Debugger &debugger;
        uc_engine *nativeEngine{};
        uc_hook interruptHook{}, unmappedReadHook{}, unmappedWriteHook{};
        array<uint8, API_MEM_SIZE> apiMemory{};
        array<uint8, STACK_SIZE> stackMemory{};
        vector<uint8> codeMemory;
        int nativeContextStackDepth{};
        vector<uc_context *> nativeContextStack;
        int apiSize{};
        bool loaded{};
        cref_t nativeEventCallback{};
        cref_t nativeUpdateCallback{};
        cref_t nativeUpdateUserdata{};
        unordered_string_map<cref_t> emulatedStringLiterals;
        vector<cref_t> emulatedLuaFunctions;
        cref_t lastBadAccessAddress{};
        unordered_set<NativeResource *> nativeResources;
    };

}
