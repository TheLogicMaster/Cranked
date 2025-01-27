project(core)

find_package(Python3)
find_package(PythonInterp)

find_package(ZLIB)

if (NOT ZLIB_FOUND)
    add_subdirectory(libs/zlib)
    set(ZLIB_LIBRARIES zlib)
endif ()

option(USE_TRACY "Use Tracy profiling" OFF)
option(USE_CAPSTONE "Use Capstone disassembly" ON)

# Hack to skip FindLIBZIP.cmake in libzippp
set(LIBZIP_INCLUDE_DIR libs/libzip)
set(LIBZIP_LIBRARY libzip)
set(_libzip_pkgcfg " ")

add_subdirectory(libs/unicorn)
add_subdirectory(libs/asio)
add_subdirectory(libs/dynarmic)
add_subdirectory(libs/libzip)
add_subdirectory(libs/libzippp)
add_subdirectory(libs/nlohmann_json)
add_subdirectory(libs/ffi)
add_subdirectory(libs/magic_enum)

if (USE_CAPSTONE)
    add_subdirectory(libs/capstone)
endif()

if(USE_TRACY)
    set(TRACY_ENABLE ON)
    add_subdirectory(libs/tracy)
endif()

file(GLOB lua_sources libs/lua54/*.c)
list(FILTER lua_sources EXCLUDE REGEX ".*luac?\\.c$")
foreach(source IN LISTS lua_sources)
    set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
endforeach()

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
        PathFinding.cpp
        Rom.cpp
        Runtime.cpp
        Utils.cpp
        gen/PlaydateFunctionMappings.cpp
)

add_library(cranked STATIC ${cranked_sources} ${lua_sources})
target_include_directories(cranked PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} gen libs libs/lua54 libs/asio/include libs/zlib libs/tracy/public ${CMAKE_CURRENT_BINARY_DIR}/libs/ffi/include)
target_link_libraries(cranked unicorn libzippp nlohmann_json::nlohmann_json libffi dynarmic magic_enum::magic_enum ${ZLIB_LIBRARIES})
target_compile_definitions(cranked PUBLIC LUA_COMPAT_MATHLIB)

if(USE_CAPSTONE)
    target_link_libraries(cranked capstone)
    target_compile_definitions(cranked PUBLIC USE_CAPSTONE)
endif()

if(USE_TRACY)
    target_link_libraries(cranked TracyClient)
endif()

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
