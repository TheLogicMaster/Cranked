#include "Emulator.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

#include <SDL.h>
#include <GL/glew.h>

int main(int argc, const char *args[]) {
    if (argc != 2) {
        printf("Usage: emulator_main <pdx_path>");
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER))
        throw std::runtime_error(std::string("Failed to init SDL: ") + SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    auto windowFlags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 440, 480, windowFlags);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

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

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL2_Init();

    GLuint displayTexture;
    glGenTextures(1, &displayTexture);

    auto callback = [](Emulator *emulator){

    };
    Emulator emulator(callback);
    emulator.load(std::make_shared<Rom>(args[1]));
    emulator.start();

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
        emulator.currentInputs = 0;
        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
            if (keyboardState[keys[i]])
                emulator.currentInputs |= (1 << i);

        emulator.update();

        glBindTexture(GL_TEXTURE_2D, displayTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, emulator.graphics.displayBufferRGBA);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::Image((ImTextureID) (intptr_t) displayTexture, ImVec2(DISPLAY_WIDTH, DISPLAY_HEIGHT));
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(backgroundColor.x * backgroundColor.w, backgroundColor.y * backgroundColor.w, backgroundColor.z * backgroundColor.w, backgroundColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
