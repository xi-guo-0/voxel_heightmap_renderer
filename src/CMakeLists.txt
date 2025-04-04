add_library(strict_options INTERFACE)
if(MSVC)
  target_compile_options(strict_options INTERFACE /W4 /WX)
else()
  target_compile_options(
    strict_options
    INTERFACE -Wall
              -Wextra
              -Werror
              -pedantic
              -Wconversion
              -Wshadow
              -Wno-implicit-int-conversion
              -Wno-float-equal
              -Wno-implicit-int-float-conversion
              -Wno-sign-conversion)
endif()

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC cglm glfw glew stb
  PRIVATE strict_options)
