project(desktop)

find_package(SDL2)

file(GLOB IMGUI_SOURCES libs/imgui/*.cpp libs/imgui/backends/imgui_impl_sdl2.cpp libs/imgui/backends/imgui_impl_sdlrenderer2.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends ${SDL2_INCLUDE_DIRS})

add_executable(cranked_desktop main.cpp)
target_include_directories(cranked_desktop PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(cranked_desktop cranked imgui ${SDL2_LIBRARIES})
