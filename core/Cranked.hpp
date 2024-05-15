#pragma once

#include "HeapAllocator.hpp"
#include "Rom.hpp"
#include "Utils.hpp"
#include "Constants.hpp"
#include "gen/PlaydateAPI.hpp"
#include "PlaydateTypes.hpp"
#include "Graphics.hpp"
#include "Audio.hpp"
#include "File.hpp"
#include "Menu.hpp"
#include "NativeEngine.hpp"
#include "LuaEngine.hpp"
#include "Debugger.hpp"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "unicorn/unicorn.h"
#include "nlohmann/json.hpp"
#include "capstone/platform.h"
#include "capstone/capstone.h"
#include "ffi.h"

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

namespace cranked {

    // Todo: A wrapper class to hide internals
    // Todo: Extract Lua functionality to some LuaEngine class
    // Todo: A safe way to either call control functions (reset) within updateInternals callbacks, or assertions to prevent doing so, or deferring until unwinding stack
    class Cranked {
    public:
        static inline const Version version{PLAYDATE_SDK_VERSION};

        enum class State {
            Stopped,
            Running,
            Stopping,
        };

        Cranked() = default;
        Cranked(const Cranked &) = delete;
        Cranked &operator=(const Cranked &) = delete;

        void load(const std::string &path);

        void unload();

        void reset();

        void start();

        void update();

        void stop();

        void terminate();

        void updateInternals();

        // Todo: Remove these temporary forwarding functions
        template<typename T = void>
        inline T *fromVirtualAddress(cref_t address, bool throws = true) {
            return nativeEngine.fromVirtualAddress<T>(address, throws);
        }

        template<typename T>
        inline cref_t toVirtualAddress(T *ptr, bool throws = true) {
            return nativeEngine.toVirtualAddress(ptr, throws);
        }

        template<typename T>
        inline void virtualWrite(uint32_t address, T value) {
            return nativeEngine.virtualWrite(address, value);
        }

        template<typename T>
        inline T virtualRead(uint32_t address) {
            return nativeEngine.virtualRead<T>(address);
        }

        inline static Cranked *fromLuaContext(lua_State *context) {
            Cranked *cranked;
            lua_getallocf(context, (void **) &cranked);
            return cranked;
        }

        inline lua_State *getLuaContext() {
            return luaEngine.getContext();
        }

        inline cref_t getEmulatedStringLiteral(const std::string &message) {
            return nativeEngine.getEmulatedStringLiteral(message);
        }

        /**
         * Gets the current crank angle change, resetting the previous angle value
         */
        float getCrankChange() {
            auto change = crankAngle - previousCrankAngle;
            previousCrankAngle = crankAngle;
            return change;
        }

        // Todo: Clean up logging, should probably send pre-formatted string and filter based on a configured log level (And use C++20 std::format)
        inline static void logMessageCallback(Cranked &cranked, LogLevel level, const char *format, va_list args) {
            vprintf(format, args);
            printf("\n");
        }

        inline void logMessageVA(LogLevel level, const char *format, va_list args) {
            if (config.loggingCallback)
                config.loggingCallback(*this, level, format, args);
        }

        inline void logMessage(LogLevel level, const char *format, ...) {
            va_list args;
            va_start(args, format);
            logMessageVA(level, format, args);
            va_end(args);
        }

        HeapAllocator heap = HeapAllocator(HEAP_SIZE);
        Graphics graphics = Graphics(this); // Todo: Replace pointers with references
        Audio audio = Audio(this);
        File files = File(this);
        Menu menu = Menu(this);
        NativeEngine nativeEngine = NativeEngine(*this);
        LuaEngine luaEngine = LuaEngine(this);
        Debugger debugger = Debugger(*this);
        std::unique_ptr<Rom> rom;

        Config config { .loggingCallback = logMessageCallback };

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
        bool use24HourTime{};
        int32_t timezoneOffset{};
        float batteryVoltage = 4.2f;
        float batteryPercentage = 100.0f;
        cref_t serialMessageCallback{};
        cref_t buttonCallback{};
        cref_t buttonCallbackUserdata{};
        cref_t buttonCallbackQueueSize{};
        std::chrono::system_clock::time_point suspendUpdateLoopUntil;
        bool disableUpdateLoop{};
        int statsInterval{};

        State state{};

    private:
        void init();

        bool initialized{};
    };

}
