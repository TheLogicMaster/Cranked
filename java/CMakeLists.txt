cmake_minimum_required(VERSION 3.5)

project(java)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cranked_java SHARED java.cpp)
target_link_libraries(cranked_java cranked)

if (ANDROID)
    target_link_libraries(cranked_java log)
endif ()

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:__cplusplus")
endif ()
