cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0091 NEW)

project(cranked)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FindPkgConfig)

add_subdirectory(core)

if (NOT SKIP_TESTING)
    add_subdirectory(testing)
endif ()

if (NOT SKIP_JAVA)
    add_subdirectory(java)
endif ()

if (NOT SKIP_DESKTOP)
    add_subdirectory(desktop)
endif ()

if (NOT SKIP_LIBRETRO)
    add_subdirectory(libretro)
endif ()
