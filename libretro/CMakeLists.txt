project(libretro)

# Needed on static libs for the libretro shared library core to compile
set_property(TARGET libzippp PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET emulator PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET capstone PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET libffi PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(emulator_libretro SHARED core.cpp)
target_include_directories(emulator_libretro PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(emulator_libretro emulator)
