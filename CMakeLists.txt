cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
  voxel_heightmap_renderer
  VERSION 0.0.1
  LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
     ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds are not allowed. Please create a separate build directory and run CMake from there."
  )
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_library(OpenGL OpenGL REQUIRED)

include(FetchContent)

# GLEW
FetchContent_Declare(
  glew
  URL "https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.zip"
  URL_HASH
    SHA256=a9046a913774395a095edcc0b0ac2d81c3aacca61787b39839b941e9be14e0d4)
FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
  FetchContent_Populate(glew)
  add_library(glew "${glew_SOURCE_DIR}/src/glew.c")
  target_include_directories(glew PUBLIC "${glew_SOURCE_DIR}/include")
  target_link_libraries(glew PUBLIC ${OpenGL})
endif()

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
  cglm URL "https://github.com/recp/cglm/archive/refs/tags/v0.9.6.zip")
FetchContent_MakeAvailable(cglm)

# stb
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
  FetchContent_Populate(stb)
  add_library(stb INTERFACE)
  target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")
endif()

add_subdirectory(src)
