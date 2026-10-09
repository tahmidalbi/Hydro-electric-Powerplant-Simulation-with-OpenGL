cmake_minimum_required(VERSION 3.10)
project(confluence CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# One line per source folder, so a new folder is never silently left out.
file(GLOB CORE_SOURCES      ${CMAKE_SOURCE_DIR}/src/core/*.cpp)       # mesh, shader, camera, clock, transform
file(GLOB OBJECT_SOURCES    ${CMAKE_SOURCE_DIR}/src/objects/*.cpp)    # the plant's objects
file(GLOB SYSTEM_SOURCES    ${CMAKE_SOURCE_DIR}/src/systems/*.cpp)    # wind, hydro, boat, waves, particles, camera, lights
file(GLOB INPUT_SOURCES     ${CMAKE_SOURCE_DIR}/src/input/*.cpp)      # keys, mouse, command line
file(GLOB RENDERING_SOURCES ${CMAKE_SOURCE_DIR}/src/rendering/*.cpp)  # draws one frame
file(GLOB UI_SOURCES        ${CMAKE_SOURCE_DIR}/src/ui/*.cpp)         # status, gallery
file(GLOB TEST_SOURCES      ${CMAKE_SOURCE_DIR}/src/tests/*.cpp)      # --selftest

add_executable(confluence
    src/main.cpp
    src/glad.c
    ${CORE_SOURCES}
    ${OBJECT_SOURCES}
    ${SYSTEM_SOURCES}
    ${INPUT_SOURCES}
    ${RENDERING_SOURCES}
    ${UI_SOURCES}
    ${TEST_SOURCES}
)

target_include_directories(confluence PRIVATE include src)

# GLFW/GLM/OpenGL are resolved via the MinGW-w64 (MSYS2) toolchain's own
# default sysroot include/lib paths -- no external/ vendoring needed.
target_link_libraries(confluence glfw3 opengl32 gdi32)

# Shaders are read at run time from "shaders/" next to the executable.
add_custom_command(TARGET confluence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:confluence>/shaders)
