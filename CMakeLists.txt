cmake_minimum_required(VERSION 3.22)
project(emulator)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FindPkgConfig)

find_package(PythonInterp)

find_package(ZLIB REQUIRED)
find_package(FFI REQUIRED)
find_package(SDL2 REQUIRED)

file(GLOB IMGUI_SOURCES libs/imgui/*.cpp libs/imgui/backends/imgui_impl_sdl2.cpp libs/imgui/backends/imgui_impl_sdlrenderer2.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC libs/imgui libs/imgui/backends ${SDL2_INCLUDE_DIRS})

add_subdirectory(libs/unicorn)
add_subdirectory(libs/capstone)
add_subdirectory(libs/libzippp)
add_subdirectory(libs/nlohmann_json)

file(GLOB lua_sources libs/lua54/*.c)
list(FILTER lua_sources EXCLUDE REGEX ".*luac?\\.c$")
foreach(source IN LISTS lua_sources) # Compile Lua as C++ for exception support
    set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
endforeach()

file(GLOB emulator_sources src/*.cpp src/gen/*.cpp src/gen/*.hpp) # Bad practice, but it sure is convenient
list(FILTER emulator_sources EXCLUDE REGEX ".*(main|libretro)\\.cpp$")

add_library(emulator STATIC ${emulator_sources} ${lua_sources})
target_include_directories(emulator PUBLIC libs libs/unicorn/include libs/lua54 libs/capstone/include ${ZLIB_INCLUDE_DIRS} ${FFI_INCLUDE_DIRS})
target_link_libraries(emulator stdc++fs imgui unicorn libzippp nlohmann_json::nlohmann_json capstone ${ZLIB_LIBRARIES} ${FFI_LIBRARIES})

add_executable(emulator_main src/main.cpp)
target_include_directories(emulator_main PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(emulator_main emulator ${SDL2_LIBRARIES})

set_property(TARGET libzippp PROPERTY POSITION_INDEPENDENT_CODE ON) # Needed on static libs for the libretro shared library core to compile
set_property(TARGET emulator PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(emulator_libretro SHARED src/libretro.cpp)
target_include_directories(emulator_libretro PUBLIC libs/libretro)
target_link_libraries(emulator_libretro emulator)

function(embed_resource resource_file header_file variable_name args)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/gen/${header_file}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_resource.py ${CMAKE_CURRENT_SOURCE_DIR}/resources/${resource_file} ${CMAKE_CURRENT_SOURCE_DIR}/src/gen/${header_file} ${variable_name} ${args}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${resource_file}
            COMMENT Processing '${resource_file}'
    )
endfunction()

embed_resource("Asheville-Sans-24-Light.pft" "SystemFont.hpp" "SYSTEM_FONT" "")
embed_resource("runtime.lua" "LuaRuntimeSource.hpp" "LUA_RUNTIME_SOURCE" "-t")
embed_resource("bump.lua" "LuaBumpSource.hpp" "LUA_BUMP_SOURCE" "-t")
