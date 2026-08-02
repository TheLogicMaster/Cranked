cmake_minimum_required(VERSION 3.5)

project(libretro)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(libffi)
find_package(libzippp)

# Needed on static libs for the libretro shared library core to compile
set_property(TARGET cranked PROPERTY POSITION_INDEPENDENT_CODE ON)

if (TARGET libzippp::libzippp)
    set_property(TARGET libzippp::libzippp PROPERTY POSITION_INDEPENDENT_CODE ON)
else ()
    set_property(TARGET libzippp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

if (TARGET libffi::libffi)
    set_property(TARGET libffi::libffi PROPERTY POSITION_INDEPENDENT_CODE ON)
else ()
    set_property(TARGET libffi PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

if(${USE_CAPSTONE})
    set_property(TARGET capstone PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

add_library(cranked_libretro SHARED core.cpp)
target_include_directories(cranked_libretro PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cranked_libretro cranked)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:__cplusplus")
endif ()
