project(emulator)

find_package(PythonInterp)

find_package(ZLIB)

if (NOT ZLIB_FOUND)
    add_subdirectory(libs/zlib)
    set(ZLIB_LIBRARIES zlib)
endif ()

# Hack to skip FindLIBZIP.cmake in libzippp
set(LIBZIP_INCLUDE_DIR libs/libzip)
set(LIBZIP_LIBRARY libzip)
set(_libzip_pkgcfg " ")

add_subdirectory(libs/unicorn)
add_subdirectory(libs/capstone)
add_subdirectory(libs/libzip)
add_subdirectory(libs/libzippp)
add_subdirectory(libs/nlohmann_json)
add_subdirectory(libs/ffi)

file(GLOB lua_sources libs/lua54/*.c)
list(FILTER lua_sources EXCLUDE REGEX ".*luac?\\.c$")
foreach(source IN LISTS lua_sources)
    set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
endforeach()

file(GLOB emulator_sources
        Emulator.cpp
        File.cpp
        Graphics.cpp
        HeapAllocator.cpp
        LuaRuntime.cpp
        Menu.cpp
        Rom.cpp
        Runtime.cpp
        Utils.cpp
        gen/PlaydateFunctionMappings.cpp
)

add_library(emulator STATIC ${emulator_sources} ${lua_sources})
target_include_directories(emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} gen libs libs/unicorn/include libs/lua54 libs/capstone/include libs/zlib ${CMAKE_CURRENT_BINARY_DIR}/libs/ffi/include)
target_link_libraries(emulator unicorn libzippp nlohmann_json::nlohmann_json capstone libffi ${ZLIB_LIBRARIES})

function(embed_resource resource_file header_file variable_name args)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gen/${header_file}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/embed_resource.py ${CMAKE_CURRENT_SOURCE_DIR}/resources/${resource_file} ${CMAKE_CURRENT_SOURCE_DIR}/gen/${header_file} ${variable_name} ${args}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/${resource_file}
            COMMENT Processing '${resource_file}'
    )
endfunction()

embed_resource("Asheville-Sans-24-Light.pft" "SystemFont.hpp" "SYSTEM_FONT" "")
embed_resource("runtime.lua" "LuaRuntimeSource.hpp" "LUA_RUNTIME_SOURCE" "-t")
embed_resource("bump.lua" "LuaBumpSource.hpp" "LUA_BUMP_SOURCE" "-t")
