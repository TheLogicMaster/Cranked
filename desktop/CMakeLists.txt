project(desktop)

find_package(SDL2)

add_subdirectory(libs/cxxopts)

file(GLOB IMGUI_SOURCES
        libs/imgui/*.cpp
        libs/imgui/backends/imgui_impl_sdl2.cpp
        libs/imgui/backends/imgui_impl_sdlrenderer2.cpp
        libs/imgui/misc/cpp/imgui_stdlib.cpp
        libs/ImGuiFileDialog/ImGuiFileDialog.cpp
)

add_executable(cranked_desktop
        main.cpp
        ${IMGUI_SOURCES}
)
target_include_directories(cranked_desktop PUBLIC
        libs/imgui
        libs/imgui/backends
        libs/imgui/misc/cpp/
        libs/imgui_club/imgui_memory_editor
        libs/ImGuiFileDialog/
        ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(cranked_desktop cranked cxxopts ${SDL2_LIBRARIES})
