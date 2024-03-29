cmake_minimum_required(VERSION 3.22)

project(cranked)

set(CMAKE_CXX_STANDARD 20)

include(FindPkgConfig)

add_subdirectory(core)

if (NOT SKIP_JAVA)
    add_subdirectory(java)
endif ()

if (NOT SKIP_DESKTOP)
    add_subdirectory(desktop)
endif ()

if (NOT SKIP_LIBRETRO)
    add_subdirectory(libretro)
endif ()
