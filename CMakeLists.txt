cmake_minimum_required(VERSION 3.20)
project(ccraft C)

# GLAD
add_library(glad libs/glad/src/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Executable
add_executable(${PROJECT_NAME} src/main.c)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE glad
    PRIVATE glfw
    PRIVATE OpenGL::GL
)

