#include "Cranked.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <SDL.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

using namespace cranked;

struct Userdata {
    SDL_AudioDeviceID audioDevice;
};

int main(int argc, const char *args[]) {
    if (argc != 2) {
        printf("Usage: emulator_main <pdx_path>");
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER))
        throw std::runtime_error(std::string("Failed to init SDL: ") + SDL_GetError());

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
    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 440, 480, windowFlags);
    if (!window)
        throw std::runtime_error(std::string("Failed to create SDL window: ") + SDL_GetError());
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer)
        throw std::runtime_error(std::string("Failed to create SDL renderer: ") + SDL_GetError());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsLight();

    auto backgroundColor = ImVec4(1.0f, 0.718f, 0.22f, 1.0f);
    auto highlightColor = ImVec4(1.0f, 0.788f, 0.416f, 1.0f);
    auto accentColor = ImVec4(1.0f, 1.0, 1.0f, 1.0f);
    auto bezelColor = ImVec4(0.145f, 0.141f, 0.118f, 1.0f);

    auto &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = bezelColor;
    style.Colors[ImGuiCol_TitleBg] = backgroundColor;
    style.Colors[ImGuiCol_TitleBgActive] = highlightColor;
    style.Colors[ImGuiCol_TitleBgCollapsed] = backgroundColor;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    Userdata userdata{
        .audioDevice = audioDevice
    };
    auto callback = [](Cranked *cranked){
        auto device = ((Userdata *)cranked->internalUserdata)->audioDevice;
        if (device) {
            int toQueue = 1024 - (int) SDL_GetQueuedAudioSize(device) / 4;
            int16_t buffer[1024 * 2];
            cranked->audio.sampleAudio(buffer, toQueue);
            SDL_QueueAudio(device, buffer, toQueue * 4);
        }
    };
    Cranked cranked(callback);
    cranked.internalUserdata = &userdata;
    cranked.load(args[1]);
    cranked.start();

    SDL_Texture* displayTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!displayTexture)
        throw std::runtime_error(std::string("Failed to create SDL display texture: ") + SDL_GetError());

    auto keyboardState = SDL_GetKeyboardState(nullptr);
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT or (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)))
                done = true;
        }

        constexpr int keys[] { SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_B, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_X };
        cranked.currentInputs = 0;
        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
            if (keyboardState[keys[i]])
                cranked.currentInputs |= (1 << i);

        cranked.update();

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        SDL_UpdateTexture(displayTexture, nullptr, cranked.graphics.displayBufferRGBA, 4 * DISPLAY_WIDTH);

        ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::Image((ImTextureID) (intptr_t) displayTexture, ImVec2(DISPLAY_WIDTH, DISPLAY_HEIGHT));
        ImGui::End();

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(backgroundColor.x * 255), (Uint8)(backgroundColor.y * 255), (Uint8)(backgroundColor.z * 255), (Uint8)(backgroundColor.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
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
