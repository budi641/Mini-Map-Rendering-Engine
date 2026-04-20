cmake_minimum_required(VERSION 3.22)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(MiniMapEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MINIMAP_BUILD_DESKTOP "Build desktop sample app" ON)
option(MINIMAP_BUILD_ANDROID "Build Android JNI library" ON)
option(MINIMAP_ENABLE_STRICT_WARNINGS "Treat warnings as errors" ON)

include(FetchContent)
find_package(OpenGL REQUIRED)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG glew-cmake-2.2.0
)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glew)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

add_library(minimap_engine STATIC
    src/Core/Application.cpp
    src/Core/Window.cpp
    src/Core/InputState.cpp
    src/Renderer/RenderEngine.cpp
    src/Renderer/Batcher.cpp
    src/Renderer/TileRenderer.cpp
    src/Renderer/UiRenderer.cpp
    src/GPU/Buffer.cpp
    src/GPU/VertexArray.cpp
    src/GPU/Shader.cpp
    src/GPU/Texture2D.cpp
    src/Map/Projection.cpp
    src/Map/TileSystem.cpp
    src/Map/Camera.cpp
    src/Map/TileManager.cpp
    src/Map/RoadNetwork.cpp
    src/Map/Distance.cpp
    src/Geometry/Simplifier.cpp
    src/Geometry/Triangulator.cpp
    src/Geometry/PolylineMeshBuilder.cpp
    src/Geometry/PolygonMeshBuilder.cpp
    src/Math/Mat4.cpp
    src/Utils/Logger.cpp
    src/Utils/Timer.cpp
)

target_include_directories(minimap_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${stb_SOURCE_DIR}
)

target_link_libraries(minimap_engine PUBLIC glfw OpenGL::GL libglew_static)

target_compile_definitions(minimap_engine PUBLIC
    MINIMAP_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

if(MINIMAP_ENABLE_STRICT_WARNINGS)
    if(MSVC)
        target_compile_options(minimap_engine PRIVATE /W4 /WX)
    else()
        target_compile_options(minimap_engine PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

if(MINIMAP_BUILD_DESKTOP)
    add_executable(minimap_demo src/main.cpp)
    target_link_libraries(minimap_demo PRIVATE minimap_engine)
    if(MINIMAP_ENABLE_STRICT_WARNINGS)
        if(MSVC)
            target_compile_options(minimap_demo PRIVATE /W4 /WX)
        else()
            target_compile_options(minimap_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
        endif()
    endif()
endif()

if(ANDROID AND MINIMAP_BUILD_ANDROID)
    add_subdirectory(android)
endif()

enable_testing()
add_executable(minimap_verify tests/verify_main.cpp)
target_link_libraries(minimap_verify PRIVATE minimap_engine)
if(MINIMAP_ENABLE_STRICT_WARNINGS)
    if(MSVC)
        target_compile_options(minimap_verify PRIVATE /W4 /WX)
    else()
        target_compile_options(minimap_verify PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()
add_test(NAME minimap_verify COMMAND minimap_verify)
set_tests_properties(minimap_verify PROPERTIES
    ENVIRONMENT "MINIMAP_VECTOR_SOURCE=synthetic;MINIMAP_RASTER_SOURCE=synthetic"
    LABELS "deterministic;core"
)

if(MINIMAP_BUILD_DESKTOP)
    add_test(NAME minimap_render_self_test COMMAND minimap_demo --self-test)
    set_tests_properties(minimap_render_self_test PROPERTIES
        ENVIRONMENT "MINIMAP_VECTOR_SOURCE=synthetic;MINIMAP_RASTER_SOURCE=synthetic"
        LABELS "deterministic;render"
    )
    add_test(
        NAME minimap_local_osm_verify
        COMMAND ${CMAKE_COMMAND}
            -DWORK_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DVERIFY_EXE=$<TARGET_FILE:minimap_verify>
            -DSELF_TEST_EXE=$<TARGET_FILE:minimap_demo>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunLocalOsmVerify.cmake
    )
    set_tests_properties(minimap_local_osm_verify PROPERTIES
        SKIP_REGULAR_EXPRESSION "SKIP: map.osm missing"
        LABELS "local_osm;render"
    )
endif()
