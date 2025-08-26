cmake_minimum_required(VERSION 3.20)
project(ccraft C)

# GLAD
add_library(glad libs/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/include)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

# CGLM
add_subdirectory(libs/cglm)

# stb_image.h
include_directories(${CMAKE_SOURCE_DIR}/libs/stb_image)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Executable
add_executable(
    ${PROJECT_NAME}
    src/shader.c
	src/texture.c
    src/chunk.c
    src/world.c
	src/player.c
	src/input.c
	src/game.c
    src/main.c
)

# Link libraries
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE glad
    PRIVATE glfw
    PRIVATE cglm
    PRIVATE OpenGL::GL
)

