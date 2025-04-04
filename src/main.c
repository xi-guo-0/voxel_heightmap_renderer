#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLFWwindow *create_window(int width, int height, const char *title) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return NULL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return NULL;
  }

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return NULL;
  }

  return window;
}

void cleanup_glfw_window(GLFWwindow **window) {
  if (*window) {
    glfwDestroyWindow(*window);
    *window = NULL;
  }
}

int main(void) {
  int window_width = 640;
  int window_height = 480;

  __attribute__((cleanup(cleanup_glfw_window))) GLFWwindow *window =
      create_window(window_width, window_height, "Voxel Heightmap Renderer");
  if (!window) {
    return EXIT_FAILURE;
  }

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return EXIT_SUCCESS;
}
