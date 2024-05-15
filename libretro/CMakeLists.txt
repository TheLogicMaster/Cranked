project(libretro)

# Needed on static libs for the libretro shared library core to compile
set_property(TARGET libzippp PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET cranked PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET capstone PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET libffi PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(cranked_libretro SHARED core.cpp)
target_include_directories(cranked_libretro PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cranked_libretro cranked)
