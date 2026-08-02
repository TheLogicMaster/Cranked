cmake_minimum_required(VERSION 3.5)

project(desktop)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2)
find_package(cxxopts)

if (NOT TARGET SDL2::SDL2)
    add_subdirectory(libs/SDL)
endif ()

if (NOT TARGET cxxopts::cxxopts)
    add_subdirectory(libs/cxxopts)
endif ()

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
)
target_link_libraries(cranked_desktop cranked cxxopts::cxxopts SDL2::SDL2)

set_target_properties(cranked_desktop PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:__cplusplus")

    file(GLOB DLL_FILES "${CMAKE_BINARY_DIR}/*.dll")

    add_custom_command(
            TARGET cranked_desktop TARGET POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${DLL_FILES} $<TARGET_FILE_DIR:cranked_desktop>
    )
endif ()
