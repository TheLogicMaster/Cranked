#include "Cranked.hpp"

using namespace cranked;

void Cranked::init() {
    graphics.init();
    files.init();
    nativeEngine.init();
    luaEngine.init();
    debugger.init();
    startTime = elapsedTimeStart = std::chrono::system_clock::now();
    initialized = true;
}

void Cranked::load(const std::string &path) {
    unload();
    rom = std::make_unique<Rom>(path, this);
    rom->load();
    nativeEngine.load();
}

void Cranked::unload() {
    reset();
    if (rom)
        rom->unload();
    nativeEngine.unload();
}

void Cranked::reset() {
    state = State::Stopped;
    initialized = false;
    lastFrameTime = {};
    disableUpdateLoop = false;
    crankSoundsEnabled = true;
    statsInterval = 0;

    debugger.reset();
    graphics.reset();
    menu.reset();
    files.reset();
    nativeEngine.reset();

    currentInputs = previousInputs = pressedInputs = releasedInputs = 0;
    crankAngle = previousCrankAngle = 0;
    crankDocked = previousCrankDocked = true;

    heap.reset(); // Reset heap only after objects are freed
}

void Cranked::update() {
    // Todo: Handle `Stopping` state and call trigger corresponding events

    if (state != State::Running) {
        updateInternals();
        return;
    }

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
            if (nativeEngine.isLoaded()) {
                for (int i = 0; i < 6; i++) {
                    auto bit = 1 << i;
                    if (pressedInputs & bit)
                        nativeEngine.invokeEventCallback(PDSystemEvent::KeyPressed, (uint32_t)bit);
                    if (releasedInputs & bit)
                        nativeEngine.invokeEventCallback(PDSystemEvent::KeyReleased, (uint32_t)bit);
                }
            }

            // Call Lua input events
            if (luaEngine.isLoaded()) {
                if (pressedInputs & int(PDButtons::A))
                    luaEngine.invokeLuaInputCallback("AButtonDown");
                if (pressedInputs & int(PDButtons::B))
                    luaEngine.invokeLuaInputCallback("BButtonDown");
                if (pressedInputs & int(PDButtons::Down))
                    luaEngine.invokeLuaInputCallback("downButtonDown");
                if (pressedInputs & int(PDButtons::Left))
                    luaEngine.invokeLuaInputCallback("leftButtonDown");
                if (pressedInputs & int(PDButtons::Right))
                    luaEngine.invokeLuaInputCallback("rightButtonDown");
                if (pressedInputs & int(PDButtons::Up))
                    luaEngine.invokeLuaInputCallback("upButtonDown");

                if (releasedInputs & int(PDButtons::A))
                    luaEngine.invokeLuaInputCallback("AButtonUp");
                if (releasedInputs & int(PDButtons::B))
                    luaEngine.invokeLuaInputCallback("BButtonUp");
                if (releasedInputs & int(PDButtons::Down))
                    luaEngine.invokeLuaInputCallback("downButtonUp");
                if (releasedInputs & int(PDButtons::Left))
                    luaEngine.invokeLuaInputCallback("leftButtonUp");
                if (releasedInputs & int(PDButtons::Right))
                    luaEngine.invokeLuaInputCallback("rightButtonUp");
                if (releasedInputs & int(PDButtons::Up))
                    luaEngine.invokeLuaInputCallback("upButtonUp");

                if (buttonDownTimeA.time_since_epoch() != std::chrono::system_clock::time_point::duration::zero() and currentTime > buttonDownTimeA + std::chrono::seconds(1)) {
                    luaEngine.invokeLuaInputCallback("AButtonHeld");
                    buttonDownTimeA = {};
                }
                if (buttonDownTimeB.time_since_epoch() != std::chrono::system_clock::time_point::duration::zero() and currentTime > buttonDownTimeB + std::chrono::seconds(1)) {
                    luaEngine.invokeLuaInputCallback("BButtonHeld");
                    buttonDownTimeB = {};
                }

                if (crankAngle != previousCrankAngle) {
                    auto change = crankAngle - previousCrankAngle;
                    if (luaEngine.invokeLuaInputCallback("cranked", {change, change})) // Todo: `Acceleration`
                        previousCrankAngle = crankAngle;
                }

                if (crankDocked and not previousCrankDocked)
                    luaEngine.invokeLuaInputCallback("crankDocked");
                if (previousCrankDocked and not crankDocked)
                    luaEngine.invokeLuaInputCallback("crankUndocked");
            }

            if (!disableUpdateLoop and currentTime > suspendUpdateLoopUntil) {
                lastFrameTime = currentTime;
                if (nativeEngine.hasUpdateCallback()) {
                    if (nativeEngine.invokeUpdateCallback())
                        graphics.flushDisplayBuffer();
                } else if (luaEngine.isLoaded()) {
                    luaEngine.runUpdateLoop();
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

void Cranked::start() {
    if (!rom)
        throw std::runtime_error("Rom not loaded");
    state = State::Running;
    if (!initialized)
        init();
}

void Cranked::stop() {
    if (state != State::Running)
        return;
    state = State::Stopping;
    // Todo: Stop from UC and Lua hooks
    // Todo: Actually set state to Stopped, probably in `update`
}

void Cranked::terminate() {
    state = State::Stopped; // Todo: Maybe a different flag
}

void Cranked::updateInternals() {
    // Todo: Update audio (When running)
    debugger.update();
    if (config.updateCallback)
        config.updateCallback(*this);
}
