cmake_minimum_required(VERSION 3.5)

project(core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_TOOLCHAIN_FILE MATCHES "conan_toolchain.cmake")
    message(STATUS "Using Conan toolchain")
    set(USING_CONAN ON)
else()
    message(STATUS "Not using Conan toolchain")
    set(USING_CONAN OFF)
endif()

find_package(Python3)
find_package(PythonInterp)

find_package(ZLIB)
find_package(unicorn)
find_package(libffi)
find_package(libzip)
find_package(libzippp)
find_package(nlohmann_json)
find_package(SDL2)
find_package(magic_enum)
find_package(zlib)
find_package(asio)
find_package(Tracy)

option(USE_TRACY "Use Tracy profiling" OFF)
option(USE_CAPSTONE "Use Capstone disassembly" OFF)

# Hack to skip FindLIBZIP.cmake in libzippp
set(LIBZIP_INCLUDE_DIR libs/libzip)
set(LIBZIP_LIBRARY libzip)
set(_libzip_pkgcfg " ")

set(CRANKED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} gen libs libs/lua54)
set(CRANKED_LIBS magic_enum::magic_enum nlohmann_json::nlohmann_json)
set(CRANKED_DEFINES LUA_COMPAT_MATHLIB -DBOOST_ASIO_STANDALONE=true SDL_MAIN_HANDLED)

if (NOT TARGET magic_enum::magic_enum)
    add_subdirectory(libs/magic_enum)
endif ()

if (NOT TARGET libzip::zip)
    add_subdirectory(libs/libzip)
endif ()

if (TARGET libzippp::libzippp)
    list(APPEND CRANKED_LIBS libzippp::libzippp)
else ()
    add_subdirectory(libs/libzippp)
    list(APPEND CRANKED_LIBS libzippp)
endif ()

if (NOT TARGET nlohmann_json::nlohmann_json)
    add_subdirectory(libs/nlohmann_json)
endif ()

if (TARGET libffi::libffi)
    list(APPEND CRANKED_LIBS libffi::libffi)
else ()
    add_subdirectory(libs/ffi)
    list(APPEND CRANKED_LIBS libffi)
    list(APPEND CRANKED_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/libs/ffi/include)
endif ()

if (TARGET ZLIB::ZLIB)
    list(APPEND CRANKED_LIBS ZLIB::ZLIB)
else ()
    add_subdirectory(libs/zlib)
    list(APPEND CRANKED_LIBS ${ZLIB_LIBRARIES})
    list(APPEND CRANKED_INCLUDES libs/zlib)
endif ()

if (TARGET unicorn::unicorn)
    list(APPEND CRANKED_LIBS unicorn::unicorn)
else ()
    add_subdirectory(libs/unicorn)
    list(APPEND CRANKED_LIBS unicorn)
endif ()

if (TARGET asio::asio)
    list(APPEND CRANKED_LIBS asio::asio)
else ()
    add_subdirectory(libs/asio)
    list(APPEND CRANKED_INCLUDES libs/asio/include)
endif ()

if (TARGET Tracy::TracyClient)
    list(APPEND CRANKED_LIBS Tracy::TracyClient)
else ()
    add_subdirectory(libs/tracy)
    list(APPEND CRANKED_LIBS TracyClient)
    list(APPEND CRANKED_INCLUDES libs/tracy/public)
endif ()

if (USE_CAPSTONE)
    if (TARGET capstone::capstone)
        list(APPEND CRANKED_LIBS capstone::capstone)
    else ()
        add_subdirectory(libs/capstone)
        list(APPEND CRANKED_LIBS capstone)
    endif ()
    list(APPEND CRANKED_DEFINES USE_CAPSTONE)
endif ()

if (USE_TRACY)
    set(TRACY_ENABLE ON)
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:__cplusplus")
    list(APPEND CRANKED_LIBS ws2_32 wsock32)
    list(APPEND CRANKED_DEFINES _WIN32_WINNT=0x0A00)
endif ()

file(GLOB lua_sources libs/lua54/*.c)
list(FILTER lua_sources EXCLUDE REGEX ".*luac?\\.c$")
foreach (source IN LISTS lua_sources)
    set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
endforeach ()

set(cranked_sources
        Audio.cpp
        Bump.cpp
        Cranked.cpp
        Debugger.cpp
        Files.cpp
        Graphics.cpp
        HeapAllocator.cpp
        LuaEngine.cpp
        LuaRuntime.cpp
        Menu.cpp
        NativeEngine.cpp
        Networking.cpp
        PathFinding.cpp
        Rom.cpp
        Runtime.cpp
        Utils.cpp
        gen/PlaydateFunctionMappings.cpp
)

add_library(cranked STATIC ${cranked_sources} ${lua_sources})
target_include_directories(cranked PUBLIC ${CRANKED_INCLUDES})
target_link_libraries(cranked ${CRANKED_LIBS})
target_compile_definitions(cranked PUBLIC ${CRANKED_DEFINES})

function(embed_resource resource_file header_file variable_name args needed_for)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gen/${header_file}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/embed_resource.py ${CMAKE_CURRENT_SOURCE_DIR}/resources/${resource_file} ${CMAKE_CURRENT_SOURCE_DIR}/gen/${header_file} ${variable_name} ${args}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${resource_file}
            COMMENT Processing '${resource_file}'
    )
    set_property(SOURCE ${needed_for} APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen/${header_file}) # This is a hack to add dependencies on generated files without a dummy target
endfunction()

embed_resource("Asheville-Sans-14-Light.pft" "AshevilleSans14Light.hpp" "ASHEVILLE_SANS_14_LIGHT" "" "Rom.cpp")
embed_resource("Asheville-Sans-14-Bold.pft" "AshevilleSans14Bold.hpp" "ASHEVILLE_SANS_14_BOLD" "" "Rom.cpp")
embed_resource("Asheville-Sans-14-Light-Oblique.pft" "AshevilleSans14LightOblique.hpp" "ASHEVILLE_SANS_14_LIGHT_OBLIQUE" "" "Rom.cpp")
embed_resource("Asheville-Sans-24-Light.pft" "AshevilleSans24Light.hpp" "ASHEVILLE_SANS_24_LIGHT" "" "Rom.cpp")
embed_resource("crank-frames.pdt" "CrankFrames.hpp" "CRANK_FRAMES" "" "Rom.cpp")
embed_resource("crank-frames-2x.pdt" "CrankFrames2X.hpp" "CRANK_FRAMES_2X" "" "Rom.cpp")
embed_resource("crank-frames-4x.pdt" "CrankFrames4X.hpp" "CRANK_FRAMES_4X" "" "Rom.cpp")
embed_resource("crank-frames-8x.pdt" "CrankFrames8X.hpp" "CRANK_FRAMES_8X" "" "Rom.cpp")
embed_resource("crank-notice-bubble.pdi" "CrankNoticeBubble.hpp" "CRANK_NOTICE_BUBBLE" "" "Rom.cpp")
embed_resource("crank-notice-bubble-2x.pdi" "CrankNoticeBubble2X.hpp" "CRANK_NOTICE_BUBBLE_2X" "" "Rom.cpp")
embed_resource("crank-notice-bubble-4x.pdi" "CrankNoticeBubble4X.hpp" "CRANK_NOTICE_BUBBLE_4X" "" "Rom.cpp")
embed_resource("crank-notice-bubble-8x.pdi" "CrankNoticeBubble8X.hpp" "CRANK_NOTICE_BUBBLE_8X" "" "Rom.cpp")
embed_resource("crank-notice-text.pdi" "CrankNoticeText.hpp" "CRANK_NOTICE_TEXT" "" "Rom.cpp")
embed_resource("crank-notice-text-2x.pdi" "CrankNoticeText2X.hpp" "CRANK_NOTICE_TEXT_2X" "" "Rom.cpp")
embed_resource("menu-cancel.pdi" "MenuCancel.hpp" "MENU_CANCEL" "" "Rom.cpp")
embed_resource("menu-del.pdi" "MenuDel.hpp" "MENU_DEL" "" "Rom.cpp")
embed_resource("menu-ok.pdi" "MenuOk.hpp" "MENU_OK" "" "Rom.cpp")
embed_resource("menu-space.pdi" "MenuSpace.hpp" "MENU_SPACE" "" "Rom.cpp")
embed_resource("bounce.pda" "BounceSound.hpp" "BOUNCE_SOUND" "" "Rom.cpp")
embed_resource("runtime.lua" "LuaRuntimeSource.hpp" "LUA_RUNTIME_SOURCE" "-t" "LuaRuntime.cpp")
embed_resource("patches.lua" "LuaPatchesSource.hpp" "LUA_PATCHES_SOURCE" "-t" "LuaEngine.cpp")
