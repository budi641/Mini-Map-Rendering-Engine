cmake_minimum_required(VERSION 3.22)

# Build the core engine from the repository root when Android Studio/Gradle invokes
# this CMake project directly. Disable nested android add_subdirectory recursion.
set(MINIMAP_BUILD_DESKTOP OFF CACHE BOOL "" FORCE)
set(MINIMAP_BUILD_ANDROID OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/minimap_root_build)

add_library(minimap_android SHARED
    src/main/cpp/jni_bridge.cpp
)

target_include_directories(minimap_android PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_link_libraries(minimap_android
    minimap_engine
    log
)
