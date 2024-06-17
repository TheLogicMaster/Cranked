#pragma once

#include "gen/PlaydateAPI.hpp"
#include "Constants.hpp"
#include "Rom.hpp"
#include "HeapAllocator.hpp"
#include "Debugger.hpp"
#include "NativeResource.hpp"
#include "Utils.hpp"

#include "dynarmic/interface/A32/a32.h"
#include "dynarmic/interface/A32/config.h"
#include "unicorn/unicorn.h"
#include "capstone/platform.h"
#include "capstone/capstone.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <concepts>

// Todo: Dynarmic implementation

namespace cranked {

    class Cranked;

    class NativeEngine {
        friend NativeResource;

    public:
        class NativeExecutionError : public CrankedError {
        public:
            explicit NativeExecutionError(const char *message, std::string dump = "") : NativeExecutionError(std::string(message), std::move(dump)) {}

            explicit NativeExecutionError(const std::string &message, std::string dump = "") : CrankedError(message), dump(std::move(dump)) {}

            const std::string &getDump() {
                return dump;
            }
        private:
            std::string dump;
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

        cref_t getEmulatedStringLiteral(const std::string &message) {
            try { // Todo: Don't try-catch
                return emulatedStringLiterals.at(message);
            } catch (std::out_of_range &ex) {
                auto ref = toVirtualAddress(heap.allocateString(message.c_str()));
                emulatedStringLiterals[message] = ref;
                return ref;
            }
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
                else
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
                else
                    return 0;
            }
            return address + offset;
        }

        template<typename S>
        void virtualWrite(uint32_t address, S value) {
            assertUC(uc_mem_write(nativeEngine, address, &value, sizeof(S)), "Mem write failed");
        }

        template<typename S>
        S virtualRead(uint32_t address) {
            S value;
            assertUC(uc_mem_read(nativeEngine, address, &value, sizeof(S)), "Mem read failed");
            return value;
        }

        void writeRegister(int reg, uint32_t value) {
            assertUC(uc_reg_write(nativeEngine, reg, &value), "Register write failed");
        }

        void writeRegister64(int reg, uint64_t value) {
            assertUC(uc_reg_write(nativeEngine, reg, &value), "Register write failed");
        }

        uint32_t readRegister(int reg) {
            uint32_t value;
            assertUC(uc_reg_read(nativeEngine, reg, &value), "Register read failed");
            return value;
        }

        uint64_t readRegister64(int reg) {
            uint64_t value;
            assertUC(uc_reg_read(nativeEngine, reg, &value), "Register read failed");
            return value;
        }

        void freeResource(void *ptr);

        template<typename T, typename ...Args> requires std::derived_from<T, NativeResource>
        T *createReferencedResource(Args ...args) {
            T *resource = heap.construct<T>(cranked, args...);
            resource->reference();
            return resource;
        }

        int32_t invokeEventCallback(PDSystemEvent event, uint32_t arg) {
            return invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t, ArgType::int32_t, ArgType::uint32_t>
                    (nativeEventCallback, toVirtualAddress(apiMemory.data()), (int32_t) event, (uint32_t) arg);
        }

        int32_t invokeUpdateCallback() {
            if (!nativeUpdateCallback)
                throw CrankedError("Update callback not set");
            return invokeEmulatedFunction<int32_t, ArgType::int32_t, ArgType::uint32_t>(nativeUpdateCallback, nativeUpdateUserdata);
        }

        std::vector<uint8_t> &getCodeMemory() {
            return codeMemory;
        }

        void flushCaches() {
            assertUC(uc_ctl_flush_tlb(nativeEngine), "Failed to flush TLB");
        }

        /**
         * All passed args have to match the expected size exactly, so explicitly cast `int` literals to `int32_t`, for example
         * The emulated CPU context must be preserved by the caller if already running emulated code (For Lua emulated native C functions and such)
         */
        template<typename R, ArgType N, ArgType... A, typename... P>
        R invokeEmulatedFunction(uint32_t address, [[maybe_unused]] P... params) {
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

                auto returnAddress = CODE_PAGE_ADDRESS; // Todo: This could point to previous native stack PC to help with debugging
                writeRegister(UC_ARM_REG_LR, returnAddress);
                writeRegister(UC_ARM_REG_SP, sp);

                writeRegister(UC_ARM_REG_PC, address | 0x1); // Todo: Helps debugging?

                uint32_t pc = address;
                while (true) {
                    bool shouldStep = debugger.handleInvocationDebugging();
                    auto result = uc_emu_start(nativeEngine, pc | 0x1, returnAddress, 15000, shouldStep ? 1 : 0); // Todo: Verify/fix timing
                    pc = readRegister(UC_ARM_REG_PC);
                    if (result == UC_ERR_OK) {
                        if (pc == returnAddress)
                            break;
                        updateInternals();
                    } else if (result == UC_ERR_FETCH_UNMAPPED and pc >= FUNC_TABLE_ADDRESS and pc < FUNC_TABLE_ADDRESS + FUNCTION_TABLE_SIZE * 2) {
                        uint32_t lr = readRegister(UC_ARM_REG_LR);
                        nativeFunctionDispatch(int(pc - FUNC_TABLE_ADDRESS) / 2);
                        pc = lr;
                    } else if (result == UC_ERR_FETCH_UNMAPPED or result == UC_ERR_READ_UNMAPPED or result == UC_ERR_WRITE_UNMAPPED) {
                        throw NativeExecutionError(std::format("Unmapped memory access at {}", lastBadAccessAddress), dumpCore());
                    } else {
                        assertUC(result, "Error invoking emulated function"); // Todo: Add more context for memory errors, probably need to store info from hooks, maybe full core dump
                    }
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

    private:
        static constexpr uint32_t BREAKPOINT_INTERRUPT = 7;

        void updateInternals();

        static void handleInterrupt(uc_engine *uc, uint32_t interrupt, void *userdata);

        static void handleUnmappedAccess(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *userdata);

        void nativeFunctionDispatch(int index);

        std::string dumpCore();

        void assertUC(uc_err result, const std::string &message = "Native engine operation failed") {
            if (result != UC_ERR_OK)
                throw NativeExecutionError(message + ": " + uc_strerror(result), dumpCore());
        }

        static int luaEmulatedFunctionDispatch(lua_State *luaContext);

        HeapAllocator &heap;
        Cranked &cranked;
        Debugger &debugger;
        uc_engine *nativeEngine{};
        uc_hook interruptHook{}, unmappedReadHook{}, unmappedWriteHook{};
        std::array<uint8_t, API_MEM_SIZE> apiMemory{};
        std::array<uint8_t, STACK_SIZE> stackMemory{};
        std::vector<uint8_t> codeMemory;
        int nativeContextStackDepth{};
        std::vector<uc_context *> nativeContextStack;
        int apiSize{};
        bool loaded{};
        cref_t nativeEventCallback{};
        cref_t nativeUpdateCallback{};
        cref_t nativeUpdateUserdata{};
        std::unordered_map<std::string, cref_t> emulatedStringLiterals;
        std::vector<cref_t> emulatedLuaFunctions;
        cref_t lastBadAccessAddress{};
        std::unordered_set<NativeResource *> nativeResources;
    };

}
