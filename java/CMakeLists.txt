project(java)

add_library(emulator_java SHARED java.cpp)
target_link_libraries(emulator_java emulator)

if (ANDROID)
    target_link_libraries(emulator_java log)
endif ()
