project(java)

add_library(cranked_java SHARED java.cpp)
target_link_libraries(cranked_java cranked)

if (ANDROID)
    target_link_libraries(cranked_java log)
endif ()
