#pragma once

#include "HeapAllocator.hpp"
#include "Graphics.hpp"
#include "Audio.hpp"
#include "Files.hpp"
#include "Menu.hpp"
#include "NativeEngine.hpp"
#include "LuaEngine.hpp"
#include "Debugger.hpp"
#include "Bump.hpp"
#include "PathFinding.hpp"

namespace cranked {

    // Todo: Multi-threaded execution with locking to prevent deadlocks? This avoids callbacks from Lua hooks but requires the UI to lock and process audio.
    // Todo: To enforce thread-safety, a wrapper object which requires you to lock before exposing internals via dereferencing operator could help.
    // Todo: A safe way to either call control functions (reset) within updateInternals callbacks, or assertions to prevent doing so, or deferring until unwinding stack
    // Todo: Serial callbacks
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
        Cranked(Cranked &&) = delete;
        ~Cranked();
        Cranked &operator=(const Cranked &) = delete;

        void load(const string &path);

        void unload();

        void reset();

        void start();

        void update();

        void stop();

        void terminate();

        void updateInternals();

        // Todo: Remove these temporary forwarding functions
        template<typename T = void>
        T *fromVirtualAddress(cref_t address, bool throws = true) {
            return nativeEngine.fromVirtualAddress<T>(address, throws);
        }

        template<typename T>
        cref_t toVirtualAddress(T *ptr, bool throws = true) {
            return nativeEngine.toVirtualAddress(ptr, throws);
        }

        template<typename T>
        void virtualWrite(uint32 address, T value) {
            return nativeEngine.virtualWrite(address, value);
        }

        template<typename T>
        T virtualRead(uint32 address) {
            return nativeEngine.virtualRead<T>(address);
        }

        static Cranked *fromLuaContext(lua_State *context) {
            Cranked *cranked;
            lua_getallocf(context, (void **) &cranked);
            return cranked;
        }

        lua_State *getLuaContext() const {
            return luaEngine.getContext();
        }

        cref_t getEmulatedStringLiteral(const string &message) {
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

        float getFrameDelta() {
            return lastFrameDelta;
        }

        // Todo: Clean up logging, should probably send pre-formatted string and filter based on a configured log level (And use C++20 std::format)
        static void logMessageCallback(Cranked &cranked, LogLevel level, const char *fmt, va_list args) {
            vprintf(fmt, args);
            printf("\n");
        }

        void logMessageVA(LogLevel level, const char *fmt, va_list args) {
            if (config.loggingCallback)
                config.loggingCallback(*this, level, fmt, args);
        }

        void logMessage(LogLevel level, const char *fmt, ...) {
            va_list args;
            va_start(args, fmt);
            logMessageVA(level, fmt, args);
            va_end(args);
        }

        HeapAllocator heap{ HEAP_SIZE };
        NativeEngine nativeEngine{ *this };
        Graphics graphics{ *this };
        Audio audio{ *this };
        Files files{ *this };
        Menu menu{ *this };
        LuaEngine luaEngine{ *this };
        Bump bump { *this };
        Debugger debugger { *this };
        unique_ptr<Rom> rom;

        Config config { .loggingCallback = logMessageCallback };

        chrono::system_clock::time_point elapsedTimeStart, startTime;
        uint32 currentMillis{}; // Milliseconds elapsed while game is running
        chrono::system_clock::time_point lastFrameTime;
        int32 currentInputs{}, previousInputs{};
        int32 pressedInputs{}, releasedInputs{};
        chrono::system_clock::time_point buttonDownTimeA, buttonDownTimeB;
        PDLanguage systemLanguage{};
        float crankAngle{}, previousCrankAngle{};
        bool crankDocked{}, previousCrankDocked{};
        bool crankSoundsEnabled{};
        bool systemFlipped{};
        bool autoLockDisabled{};
        bool tryReduceFlashing{};
        float accelerometerX{}, accelerometerY{}, accelerometerZ{};
        bool use24HourTime{};
        int32 timezoneOffset{};
        float batteryVoltage = 4.2f;
        float batteryPercentage = 100.0f;
        cref_t serialMessageCallback{};
        cref_t buttonCallback{};
        cref_t buttonCallbackUserdata{};
        cref_t buttonCallbackQueueSize{};
        chrono::system_clock::time_point suspendUpdateLoopUntil;
        bool disableUpdateLoop{};
        int statsInterval{};

        State state{};

    private:
        void init();

        bool initialized{};
        float lastFrameDelta{}; // Todo: Move to Graphics
    };

}
