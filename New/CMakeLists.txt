cmake_minimum_required(VERSION 3.10)
project(hydro CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# one line per folder so it is easy to see where everything lives
file(GLOB CORE_SOURCES    ${CMAKE_SOURCE_DIR}/src/core/*.cpp)      # math, mesh, shader, camera, screenshot
file(GLOB RENDER_SOURCES  ${CMAKE_SOURCE_DIR}/src/render/*.cpp)    # renderer, particles
file(GLOB OBJECT_SOURCES  ${CMAKE_SOURCE_DIR}/src/objects/*.cpp)   # one file per 3D object
file(GLOB SCENE_SOURCES   ${CMAKE_SOURCE_DIR}/src/scenes/*.cpp)    # one file per scene of the video
file(GLOB UI_SOURCES      ${CMAKE_SOURCE_DIR}/src/ui/*.cpp)        # font, HUD drawing

add_executable(hydro
    src/main.cpp
    src/App.cpp
    src/glad.c
    ${CORE_SOURCES} ${RENDER_SOURCES} ${OBJECT_SOURCES} ${SCENE_SOURCES} ${UI_SOURCES})

target_include_directories(hydro PRIVATE include src)
# GLFW (window + input) and OpenGL come with the MSYS2 MinGW-w64 toolchain
target_link_libraries(hydro glfw3 opengl32 gdi32)

# shaders are read at run time from "shaders/" next to the executable
add_custom_command(TARGET hydro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:hydro>/shaders)
