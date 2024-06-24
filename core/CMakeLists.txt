project(core)

find_package (Python3)

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
add_subdirectory(libs/asio)
add_subdirectory(libs/dynarmic)
add_subdirectory(libs/libzip)
add_subdirectory(libs/libzippp)
add_subdirectory(libs/nlohmann_json)
add_subdirectory(libs/ffi)
add_subdirectory(libs/magic_enum)

file(GLOB lua_sources libs/lua54/*.c)
list(FILTER lua_sources EXCLUDE REGEX ".*luac?\\.c$")
foreach(source IN LISTS lua_sources)
    set_source_files_properties(${source} PROPERTIES LANGUAGE CXX)
endforeach()

file(GLOB cranked_sources
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
        NativeResource.cpp
        Rom.cpp
        Runtime.cpp
        Utils.cpp
        gen/PlaydateFunctionMappings.cpp
)

add_library(cranked STATIC ${cranked_sources} ${lua_sources})
target_include_directories(cranked PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} gen libs libs/unicorn/include libs/lua54 libs/dynarmic/src libs/capstone/include libs/asio/include libs/zlib ${CMAKE_CURRENT_BINARY_DIR}/libs/ffi/include)
# Todo: Make debugger libs optional (capstone)
target_link_libraries(cranked unicorn libzippp nlohmann_json::nlohmann_json capstone libffi dynarmic magic_enum::magic_enum ${ZLIB_LIBRARIES})

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
