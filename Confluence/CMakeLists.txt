cmake_minimum_required(VERSION 3.10)
project(confluence CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB CORE_SOURCES    ${CMAKE_SOURCE_DIR}/src/core/*.cpp)
file(GLOB OBJECT_SOURCES  ${CMAKE_SOURCE_DIR}/src/objects/*.cpp)

add_executable(confluence
    src/main.cpp
    src/glad.c
    ${CORE_SOURCES}
    ${OBJECT_SOURCES}
)

target_include_directories(confluence PRIVATE include src)

# GLFW/GLM/OpenGL are resolved via the MinGW-w64 (MSYS2) toolchain's own
# default sysroot include/lib paths -- no external/ vendoring needed.
target_link_libraries(confluence glfw3 opengl32 gdi32)

add_custom_command(TARGET confluence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:confluence>/shaders)
