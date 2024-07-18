#include "Cranked.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "imgui_memory_editor.h"
#include "ImGuiFileDialog.h"
#include "cxxopts.hpp"

#include <SDL.h>

#include "magic_enum_all.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

using namespace cranked;

typedef variant<string, int, float, bool> setting_type;

enum class Windows {
    Debug, Registers, HeapMemory, CodeMemory, Disassembly, Buttons, Crank, Accelerometer
};

struct Userdata {
    Cranked &cranked;
    SDL_Window* window{};
    map<string, setting_type> settings; // Only updated while saving/loading
    float scale = 1;
    magic_enum::containers::array<Windows, bool> windowStates{};

    template<typename T>
    bool tryGetSetting(const string &name, T &out) {
        if (T *value = get_if<T>(&settings[name])) {
            out = *value;
            return true;
        }
        return false;
    }
};

int main(int argc, const char *args[]) {
    cxxopts::Options commandArgOptions("Cranked", "A Playdate console emulator");
    commandArgOptions.add_options()
        ("d,debug", "GDB enable/port", cxxopts::value<int>())
        ("p,pause", "Start paused", cxxopts::value<bool>())
        ("program", "The program to run", cxxopts::value<string>());
    commandArgOptions.parse_positional({"program"});

    auto result = commandArgOptions.parse(argc, args);
    string programPath = result["program"].as<string>();
    int debugPort = result["debug"].as_optional<int>().value_or(0);
    bool startPaused = result["pause"].as_optional<bool>().value_or(false);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER))
        throw CrankedError("Failed to init SDL: {}", SDL_GetError());

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_AudioSpec audioSpec{
            .freq = 44100,
            .format = AUDIO_S16,
            .channels = 2,
            .samples = 1024,
    };
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, false, &audioSpec, nullptr, false);
    if (!audioDevice)
        printf("Failed to initialize audio: %s\n", SDL_GetError());

    auto windowFlags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    // Todo: If window starts small, it squishes saved window positions
    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 1400, windowFlags);
    if (!window)
        throw CrankedError("Failed to create SDL window: {}", SDL_GetError());
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer)
        throw CrankedError("Failed to create SDL renderer: {}", SDL_GetError());

    IMGUI_CHECKVERSION();
    auto imguiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsLight();

    auto backgroundColor = ImVec4(1.0f, 0.718f, 0.22f, 1.0f);
    auto highlightColor = ImVec4(1.0f, 0.788f, 0.416f, 1.0f);
    auto accentColor = ImVec4(1.0f, 1.0, 1.0f, 1.0f);
    auto bezelColor = ImVec4(0.145f, 0.141f, 0.118f, 1.0f);
    auto selectedColor = ImVec4(1.0f, 0.7f, 0.35f, 0.9f);
    auto activeColor = ImVec4(1.0f, 0.773f, 0.0f, 1.0f);

    auto &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = backgroundColor;
    style.Colors[ImGuiCol_TitleBg] = backgroundColor;
    style.Colors[ImGuiCol_TitleBgActive] = highlightColor;
    style.Colors[ImGuiCol_TitleBgCollapsed] = backgroundColor;
    style.Colors[ImGuiCol_MenuBarBg] = backgroundColor;
    style.Colors[ImGuiCol_PopupBg] = highlightColor;
    style.Colors[ImGuiCol_Button] = highlightColor;
    style.Colors[ImGuiCol_ButtonActive] = activeColor;
    style.Colors[ImGuiCol_ButtonHovered] = selectedColor;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    Cranked cranked;
    Userdata userdata { .cranked = cranked, .window = window };
    cranked.config.userdata = &userdata;
    float &scale = userdata.scale;

    { // Todo: This would be good to extract to a header and make more generic
        ImGuiSettingsHandler settingsHandler;
        settingsHandler.TypeName = "Cranked";
        settingsHandler.UserData = &userdata;
        settingsHandler.TypeHash = ImHashStr("Cranked");
        settingsHandler.ClearAllFn = [](ImGuiContext *ctx, ImGuiSettingsHandler *handler) {};
        settingsHandler.ReadOpenFn = [](ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name) {
            return (void *)name; // Just use name as entry value
        };
        settingsHandler.ReadLineFn = [](ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line) {
            auto userdata = (Userdata *)handler->UserData;
            string lineStr(line);
            size_t typeSeparatorIndex = lineStr.find('@');
            size_t valueSeparatorIndex = lineStr.find('=');
            if (typeSeparatorIndex == string::npos or valueSeparatorIndex == string::npos)
                throw CrankedError("Bad settings item in line: `{}`", line);
            auto type = lineStr.substr(0, typeSeparatorIndex);
            auto name = lineStr.substr(typeSeparatorIndex + 1, valueSeparatorIndex - typeSeparatorIndex - 1);
            auto value = lineStr.substr(valueSeparatorIndex + 1);
            try {
                if (type == "String")
                    userdata->settings[name] = value;
                else if (type == "Int")
                    userdata->settings[name] = stoi(value);
                else if (type == "Float")
                    userdata->settings[name] = stof(value);
                else if (type == "Bool")
                    userdata->settings[name] = (bool) stoi(value);
                else
                    throw CrankedError("Bad settings item type in line: `{}`", line);
            } catch (logic_error &ex) {
                throw CrankedError("Bad settings item value in line: `{}` ({})", line, ex.what());
            }
        };
        settingsHandler.ApplyAllFn = [](ImGuiContext *ctx, ImGuiSettingsHandler *handler) {
            auto userdata = (Userdata *)handler->UserData;
            int width, height;
            if (userdata->tryGetSetting("WindowWidth", width) && userdata->tryGetSetting("WindowHeight", height))
                SDL_SetWindowSize(userdata->window, width, height);

            userdata->tryGetSetting("Scale", userdata->scale);
            ImGui::GetStyle().ScaleAllSizes((float)userdata->scale);

            for (size_t i = 0; i < userdata->windowStates.size(); i++) {
                auto value = magic_enum::enum_value<Windows>(i);
                bool shown{};
                userdata->tryGetSetting("ShowWindow" + string(magic_enum::enum_name(value)), shown);
                userdata->windowStates[value] = shown;
            }
        };
        settingsHandler.WriteAllFn = [](ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf) {
            auto userdata = (Userdata *)handler->UserData;

            int width, height;
            SDL_GetWindowSize(userdata->window, &width, &height);
            userdata->settings["WindowWidth"] = width;
            userdata->settings["WindowHeight"] = height;

            userdata->settings["Scale"] = userdata->scale;

            for (size_t i = 0; i < userdata->windowStates.size(); i++) {
                auto value = magic_enum::enum_value<Windows>(i);
                userdata->settings["ShowWindow" + string(magic_enum::enum_name(value))] = userdata->windowStates[value];
            }

            buf->append("[Cranked][Settings]\n");
            for (const auto &entry : userdata->settings) {
                if (holds_alternative<string>(entry.second))
                    buf->appendf("String@%s=%s\n", entry.first.c_str(), get<string>(entry.second).c_str());
                else if (holds_alternative<int>(entry.second))
                    buf->appendf("Int@%s=%d\n", entry.first.c_str(), get<int>(entry.second));
                else if (holds_alternative<float>(entry.second))
                    buf->appendf("Float@%s=%f\n", entry.first.c_str(), get<float>(entry.second));
                else if (holds_alternative<bool>(entry.second))
                    buf->appendf("Bool@%s=%d\n", entry.first.c_str(), get<bool>(entry.second));
                else
                    throw CrankedError("Unexpected settings type variant");
            }
        };
        imguiContext->SettingsHandlers.push_back(settingsHandler);
    }

    SDL_Texture* displayTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!displayTexture)
        throw CrankedError("Failed to create SDL display texture: ", SDL_GetError());

    auto getWindowOpen = [&](Windows window) -> bool & { return userdata.windowStates[window]; };

    MemoryEditor codeMemoryEditor;
    MemoryEditor heapMemoryEditor;

    string newBreakpointText;
    bool disassemblyFollowPc = true;

    auto keyboardState = SDL_GetKeyboardState(nullptr);
    bool exited = false;
    bool wasScaling = false;
    int newScale = 1;
    bool firstFrame = true;
    bool requestedStart = false;

    int buttonInputs{};

    // Todo: Extract the following disaster to separate functions
    auto callback = [&](Cranked &cranked){
        if (audioDevice) {
            int toQueue = 1024 - (int) SDL_GetQueuedAudioSize(audioDevice) / 4;
            int16 buffer[1024 * 2];
            cranked.audio.sampleAudio(buffer, toQueue);
            SDL_QueueAudio(audioDevice, buffer, toQueue * 4);
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT or (event.type == SDL_WINDOWEVENT and event.window.event == SDL_WINDOWEVENT_CLOSE and event.window.windowID == SDL_GetWindowID(window))) {
                exited = true;
                cranked.terminate();
            }
        }

        constexpr int keys[] { SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_Z, SDL_SCANCODE_X };
        cranked.currentInputs = buttonInputs;
        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
            if (keyboardState[keys[i]])
                cranked.currentInputs |= (1 << i);
        buttonInputs = 0;

        io.FontGlobalScale = scale;

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (firstFrame) {
            newScale = (int)scale;
            firstFrame = false;
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                // Todo: Clean this up (Different menu? Termination should not be necessary) (Also just doesn't work correctly at preset)
                if (ImGui::MenuItem(cranked.state == Cranked::State::Stopped ? "Start" : cranked.state == Cranked::State::Stopping ? "Terminate" : "Stop")) {
                    switch (cranked.state) {
                        case Cranked::State::Stopped:
                            requestedStart = true;
                            break;
                        case Cranked::State::Stopping:
                            cranked.terminate();
                            break;
                        case Cranked::State::Running:
                            cranked.stop();
                            break;
                    }
                }
                if (ImGui::MenuItem("Reset"))
                    cranked.reset(); // Todo: This is not safe at present
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::SliderInt("Scale", &newScale, 1, 6, "%dx", ImGuiSliderFlags_NoInput))
                    wasScaling = true;
                ImGui::MenuItem("Show Debug", nullptr, &getWindowOpen(Windows::Debug));
                ImGui::MenuItem("Show Registers", nullptr, &getWindowOpen(Windows::Registers));
                ImGui::MenuItem("Show Code Memory", nullptr, &getWindowOpen(Windows::CodeMemory));
                ImGui::MenuItem("Show Heap Memory", nullptr, &getWindowOpen(Windows::HeapMemory));
                ImGui::MenuItem("Show Disassembly", nullptr, &getWindowOpen(Windows::Disassembly));
                ImGui::MenuItem("Show Buttons", nullptr, &getWindowOpen(Windows::Buttons));
                ImGui::MenuItem("Show Crank", nullptr, &getWindowOpen(Windows::Crank));
                ImGui::EndMenu();
            } else if (wasScaling) {
                style.ScaleAllSizes((float)newScale / scale);
                scale = (float)newScale;
                wasScaling = false;
            }
            ImGui::EndMainMenuBar();
        }

        // ImGui::ShowDemoWindow();

        // Todo: Capture logToConsole/print and write to a "Log" window

        // Todo: Performance/stats window (Heap usage, frame times, program size), or just throw them around

        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, bezelColor);
            ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize);
            SDL_UpdateTexture(displayTexture, nullptr, cranked.graphics.displayBufferRGBA, 4 * DISPLAY_WIDTH);
            ImGui::Image((ImTextureID) (intptr_t) displayTexture, ImVec2(DISPLAY_WIDTH * scale, DISPLAY_HEIGHT * scale));
            ImGui::End();
            ImGui::PopStyleColor();
        }

        if (getWindowOpen(Windows::Debug)) {
            ImGui::Begin("Debug", &getWindowOpen(Windows::Debug), ImGuiWindowFlags_None);

            ImGui::Checkbox("Enabled", &cranked.debugger.isEnabled());

            if (ImGui::Button("Halt"))
                cranked.debugger.halt();
            ImGui::SameLine();
            if (ImGui::Button("Continue"))
                cranked.debugger.resume();
            ImGui::SameLine();
            if (ImGui::Button("Step"))
                cranked.debugger.step();

            ImGui::SeparatorText("Breakpoints");

            ImGui::BeginChild("BreakpointList", ImVec2(ImGui::GetWindowWidth() - 10 * scale, ImGui::GetWindowHeight() - 130 * scale));
            bool oddBreakpoint = false;
            for (cref_t breakpoint: cranked.debugger.getBreakpoints()) {
                ImGui::PushID((int) breakpoint);
                if (oddBreakpoint)
                    ImGui::SameLine();
                if (ImGui::Button("X")) {
                    cranked.debugger.removeBreakpoint(breakpoint);
                    ImGui::PopID();
                    break;
                }
                ImGui::PopID();
                ImGui::SameLine();
                ImGui::TextColored(accentColor, "%08X", breakpoint);
                oddBreakpoint = !oddBreakpoint;
            }
            ImGui::EndChild();

            ImGui::SetNextItemWidth(80 * scale);
            ImGui::InputText("##", &newBreakpointText, ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AlwaysOverwrite | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::SameLine();
            if (ImGui::Button("Add") && !newBreakpointText.empty()) {
                try {
                    cranked.debugger.addBreakpoint((cref_t)stol(newBreakpointText, nullptr, 16));
                    newBreakpointText.clear();
                } catch (exception &) {}
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear"))
                cranked.debugger.clearBreakpoints();

            ImGui::End();
        }

        if (getWindowOpen(Windows::Registers)) {
            ImGui::Begin("Registers", &getWindowOpen(Windows::Registers), ImGuiWindowFlags_None);

            ImGui::TextColored(bezelColor, "PC ");
            ImGui::SameLine();
            ImGui::TextColored(bezelColor, "%08X", cranked.nativeEngine.readRegister(UC_ARM_REG_PC));

            ImGui::SameLine(0.0, 20 * scale);
            ImGui::TextColored(bezelColor, "SP ");
            ImGui::SameLine();
            ImGui::TextColored(bezelColor, "%08x", cranked.nativeEngine.readRegister(UC_ARM_REG_SP));

            ImGui::TextColored(bezelColor, "LR ");
            ImGui::SameLine();
            ImGui::TextColored(bezelColor, "%08X", cranked.nativeEngine.readRegister(UC_ARM_REG_LR));

            ImGui::SameLine(0.0, 20 * scale);
            ImGui::TextColored(bezelColor, "SR ");
            ImGui::SameLine();
            ImGui::TextColored(bezelColor, "%08x", cranked.nativeEngine.readRegister(UC_ARM_REG_XPSR));

            if (ImGui::TreeNodeEx("General Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool oddRegister = false;
                for (int i = 0; i < 13; i++) {
                    if (oddRegister)
                        ImGui::SameLine(0.0, 20 * scale);
                    ImGui::TextColored(bezelColor, i < 10 ? "R%d " : "R%d", i);
                    ImGui::SameLine();
                    ImGui::TextColored(bezelColor, "%08X", cranked.nativeEngine.readRegister(UC_ARM_REG_R0 + i));
                    oddRegister = !oddRegister;
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Float Registers")) {
                ImGui::TextColored(bezelColor, "SR ");
                ImGui::SameLine();
                ImGui::TextColored(bezelColor, "%08X", cranked.nativeEngine.readRegister(UC_ARM_REG_FPSCR));

                bool oddRegister = false;
                for (int i = 0; i < 32; i++) {
                    if (oddRegister)
                        ImGui::SameLine(0.0, 20 * scale);
                    ImGui::TextColored(bezelColor, i < 10 ? "S%d " : "S%d", i);
                    ImGui::SameLine();
                    ImGui::TextColored(bezelColor, "%08f", bit_cast<float>(cranked.nativeEngine.readRegister(UC_ARM_REG_S0 + i)));
                    oddRegister = !oddRegister;
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Double Registers")) {
                bool oddRegister = false;
                for (int i = 0; i < 16; i++) {
                    if (oddRegister)
                        ImGui::SameLine(0.0, 20 * scale);
                    ImGui::TextColored(bezelColor, i < 10 ? "D%d " : "D%d", i);
                    ImGui::SameLine();
                    ImGui::TextColored(bezelColor, "%08f", bit_cast<double>(cranked.nativeEngine.readRegister64(UC_ARM_REG_D0 + i)));
                    oddRegister = !oddRegister;
                }
                ImGui::TreePop();
            }

            ImGui::End();
        }

        auto drawMemoryWindow = [&](Windows windowVal, const char *name, MemoryEditor &editor, void *data, size_t size, size_t base){
            if (!getWindowOpen(windowVal))
                return;
            MemoryEditor::Sizes s;
            editor.CalcSizes(s, size, base);
            ImGui::SetNextWindowSize(ImVec2(s.WindowWidth, s.WindowWidth * 0.60f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.WindowWidth, FLT_MAX));
            ImGui::Begin(name, &getWindowOpen(windowVal), ImGuiWindowFlags_NoScrollbar);
            // Todo: Ensure this is safe when code memory is empty...
            editor.DrawContents(data, size, base);
            if (editor.ContentsWidthChanged)
            {
                editor.CalcSizes(s, size, base);
                ImGui::SetWindowSize(ImVec2(s.WindowWidth, ImGui::GetWindowSize().y));
            }
            ImGui::End();
        };

        drawMemoryWindow(Windows::CodeMemory, "Code Memory", codeMemoryEditor, cranked.nativeEngine.getCodeMemory().data(), cranked.nativeEngine.getCodeMemory().size(), CODE_PAGE_ADDRESS);
        drawMemoryWindow(Windows::HeapMemory, "Heap Memory", heapMemoryEditor, cranked.heap.baseAddress(), HEAP_SIZE, HEAP_ADDRESS);

        if (getWindowOpen(Windows::Disassembly)) {
            ImGui::SetNextWindowSize(ImVec2(250 * scale, 400 * scale), ImGuiCond_FirstUseEver);
            ImGui::Begin("Disassembly", &getWindowOpen(Windows::Disassembly), ImGuiWindowFlags_NoScrollbar);
            ImGui::Checkbox("Follow PC", &disassemblyFollowPc);
            ImGui::BeginChild("DisassemblyView", ImVec2(ImGui::GetWindowWidth() - 10 * scale, ImGui::GetWindowHeight() - 55 * scale));
#if USE_CAPSTONE
            auto disassembly = cranked.debugger.getDisassembly();
            int size = cranked.debugger.getDisassemblySize();
            cref_t pc = cranked.nativeEngine.readRegister(UC_ARM_REG_PC) & ~0x1;
            for (int i = 0; i < size; i++) {
                auto &instruction = disassembly[i];
                auto address = (cref_t)instruction.address;
                ImGui::TextColored(address == pc ? accentColor : bezelColor, "%08X  %s %s", address, instruction.mnemonic, instruction.op_str);
                if (disassemblyFollowPc and address == pc)
                    ImGui::SetScrollHereY();
                if (ImGui::IsItemHovered() and ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                    if (cranked.debugger.hasBreakpoint(address))
                        cranked.debugger.removeBreakpoint(address);
                    else
                        cranked.debugger.addBreakpoint(address);
                }
            }
#endif
            ImGui::EndChild();
            ImGui::End();
        }

        // Todo: Improve aesthetics
        if (getWindowOpen(Windows::Buttons)) {
            ImGui::SetNextWindowSize(ImVec2(180 * scale, 60 * scale), ImGuiCond_FirstUseEver);
            ImGui::Begin("Buttons", &getWindowOpen(Windows::Buttons), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

            ImGui::BeginGroup();
            ImGui::BeginGroup();
            ImGui::Dummy({ 0, 10 * scale });
            buttonInputs |= (ImGui::Button("L"), ImGui::IsItemActive() * (int)PDButtons::Left);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            buttonInputs |= (ImGui::Button("U"), ImGui::IsItemActive() * (int)PDButtons::Up);
            buttonInputs |= (ImGui::Button("D"), ImGui::IsItemActive() * (int)PDButtons::Down);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Dummy({ 0, 10 * scale });
            buttonInputs |= (ImGui::Button("R"), ImGui::IsItemActive() * (int)PDButtons::Right);
            ImGui::EndGroup();
            ImGui::EndGroup();

            ImGui::SameLine();
            buttonInputs |= (ImGui::Button("A", { 40 * scale, 40 * scale }), ImGui::IsItemActive() * (int)PDButtons::A);
            ImGui::SameLine();
            buttonInputs |= (ImGui::Button("B", { 40 * scale, 40 * scale }), ImGui::IsItemActive() * (int)PDButtons::B);
            ImGui::End();
        }

        if (getWindowOpen(Windows::Crank)) {
            float radius = 35 * scale;
            ImGui::SetNextWindowSize(ImVec2(radius * 2 + 2 * scale, radius * 2 + 2 * scale), ImGuiCond_FirstUseEver);
            ImGui::Begin("Crank", &getWindowOpen(Windows::Buttons), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            auto screenPos = ImGui::GetCursorScreenPos();
            auto contentSize = ImGui::GetContentRegionAvail();
            auto drawList = ImGui::GetWindowDrawList();
            ImVec2 center{ radius + 1 * scale + screenPos.x, radius + 1 * scale + screenPos.y };
            ImGui::InvisibleButton("##", contentSize, ImGuiButtonFlags_MouseButtonLeft);
            if (ImGui::IsItemActive())
                cranked.crankAngle = atan2f(io.MousePos.y - center.y, io.MousePos.x - center.x) / numbers::pi_v<float> * 180;
            auto color = ImGui::GetColorU32(accentColor);
            drawList->AddCircle(center, radius, color);
            auto angle = cranked.crankAngle * numbers::pi_v<float> / 180;
            ImVec2 lineEnd{ center.x + cosf(angle) * radius, center.y + sinf(angle) * radius };
            drawList->AddLine(center, lineEnd, color);
            ImGui::End();
        }

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(backgroundColor.x * 255), (Uint8)(backgroundColor.y * 255), (Uint8)(backgroundColor.z * 255), (Uint8)(backgroundColor.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    };
    cranked.config.updateCallback = callback;
    cranked.config.debugPort = debugPort;
    cranked.load(programPath);
    if (!startPaused)
        cranked.start();

    while (!exited) {
        try {
            if (requestedStart) {
                cranked.start();
                requestedStart = false;
            }
            cranked.update();
            // Todo: Sleep for the rest of the frame
        } catch (NativeEngine::NativeExecutionError &ex) {
            printf("Uncaught native execution error: %s\n%s", ex.what(), ex.getDump().c_str());
            exited = true;
        }
    }

    if (audioDevice)
        SDL_CloseAudioDevice(audioDevice);

    SDL_DestroyTexture(displayTexture);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
