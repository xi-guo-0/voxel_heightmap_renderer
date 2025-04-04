#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WINDOW_TITLE "Voxel Heightmap Renderer"
#define FOV 45.0f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f
#define MOVEMENT_SPEED 10.0f
#define MOUSE_SENSITIVITY 0.1f

static const char *CUBE_VERTEX_SHADER =
    "#version 410 core\n"
    "layout (location = 0) in vec3 aPosition;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "out vec3 vFragmentPosition;\n"
    "out vec3 vNormal;\n"
    "uniform mat4 uModel;\n"
    "uniform mat4 uView;\n"
    "uniform mat4 uProjection;\n"
    "void main() {\n"
    "    vFragmentPosition = vec3(uModel * vec4(aPosition, 1.0));\n"
    "    vNormal = mat3(transpose(inverse(uModel))) * aNormal;\n"
    "    gl_Position = uProjection * uView * vec4(vFragmentPosition, 1.0);\n"
    "}\n";

static const char *CUBE_FRAGMENT_SHADER =
    "#version 410 core\n"
    "in vec3 vFragmentPosition;\n"
    "in vec3 vNormal;\n"
    "out vec4 fColor;\n"
    "void main() {\n"
    "    vec3 normalColor = abs(vNormal);\n"
    "    float brightness = (vNormal.x > 0.0 || vNormal.y > 0.0 || vNormal.z > "
    "0.0) ? 1.0 : 0.5;\n"
    "    fColor = vec4(normalColor * brightness, 1.0);\n"
    "}\n";

static const float CUBE_VERTICES[] = {

    -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f,
    0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
    0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
    0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

    -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
    0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,
    0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
    -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
    -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
    1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
    0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
    1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
    0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
    0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,
    0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
    0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
    0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};

typedef struct {
  vec3 position;
  vec3 front;
  vec3 up;
  vec3 right;
  vec3 world_up;

  float yaw;
  float pitch;

  float movement_speed;
  float mouse_sensitivity;
  float fov;
} Camera;

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint shader_program;
  size_t vertex_count;
} Mesh;

static void cleanup_ptr(void *ptr) {
  void **p = (void **)ptr;
  if (*p) {
    free(*p);
    *p = NULL;
  }
}

static void cleanup_glfw_window(GLFWwindow **window) {
  if (*window) {
    glfwDestroyWindow(*window);
    *window = NULL;
  }
}

static void cleanup_shader(GLuint *shader) {
  if (*shader) {
    glDeleteShader(*shader);
    *shader = 0;
  }
}

static void cleanup_program(GLuint *program) {
  if (*program) {
    glDeleteProgram(*program);
    *program = 0;
  }
}

static void cleanup_vbo(GLuint *vbo) {
  if (*vbo) {
    glDeleteBuffers(1, vbo);
    *vbo = 0;
  }
}

static void cleanup_vao(GLuint *vao) {
  if (*vao) {
    glDeleteVertexArrays(1, vao);
    *vao = 0;
  }
}

static void cleanup_mesh(Mesh *mesh) {
  if (mesh->shader_program) {
    cleanup_program(&(mesh->shader_program));
  }
  if (mesh->vbo) {
    cleanup_vbo(&(mesh->vbo));
  }
  if (mesh->vao) {
    cleanup_vao(&(mesh->vao));
  }
}

static void init_camera(Camera *camera, vec3 position, vec3 up, float yaw,
                        float pitch) {
  glm_vec3_copy(position, camera->position);
  glm_vec3_copy(up, camera->world_up);
  camera->yaw = yaw;
  camera->pitch = pitch;
  camera->movement_speed = MOVEMENT_SPEED;
  camera->mouse_sensitivity = MOUSE_SENSITIVITY;
  camera->fov = FOV;

  vec3 front = {cosf(glm_rad(yaw)) * cosf(glm_rad(pitch)), sinf(glm_rad(pitch)),
                sinf(glm_rad(yaw)) * cosf(glm_rad(pitch))};
  glm_vec3_normalize_to(front, camera->front);

  glm_vec3_cross(camera->front, camera->world_up, camera->right);
  glm_vec3_normalize(camera->right);
  glm_vec3_cross(camera->right, camera->front, camera->up);
  glm_vec3_normalize(camera->up);
}

static void get_view_matrix(Camera *camera, mat4 dest) {
  vec3 center;
  glm_vec3_add(camera->position, camera->front, center);
  glm_lookat(camera->position, center, camera->up, dest);
}

static void process_keyboard(Camera *camera, GLFWwindow *window,
                             float delta_time) {
  float velocity = camera->movement_speed * delta_time;
  vec3 delta;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    glm_vec3_scale(camera->front, velocity, delta);
    glm_vec3_add(camera->position, delta, camera->position);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    glm_vec3_scale(camera->front, velocity, delta);
    glm_vec3_sub(camera->position, delta, camera->position);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    glm_vec3_scale(camera->right, velocity, delta);
    glm_vec3_sub(camera->position, delta, camera->position);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    glm_vec3_scale(camera->right, velocity, delta);
    glm_vec3_add(camera->position, delta, camera->position);
  }
}

static void process_mouse_movement(Camera *camera, float xoffset, float yoffset,
                                   bool constrain_pitch) {
  xoffset *= camera->mouse_sensitivity;
  yoffset *= camera->mouse_sensitivity;

  camera->yaw += xoffset;
  camera->pitch += yoffset;

  if (constrain_pitch) {
    if (camera->pitch > 89.0f)
      camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
      camera->pitch = -89.0f;
  }

  vec3 front = {cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch)),
                sinf(glm_rad(camera->pitch)),
                sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch))};
  glm_vec3_normalize_to(front, camera->front);

  glm_vec3_cross(camera->front, camera->world_up, camera->right);
  glm_vec3_normalize(camera->right);
  glm_vec3_cross(camera->right, camera->front, camera->up);
  glm_vec3_normalize(camera->up);
}

static GLuint create_shader(GLenum shader_type, const char *shader_source) {
  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &shader_source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    __attribute__((cleanup(cleanup_ptr))) char *info_log = malloc(log_length);
    glGetShaderInfoLog(shader, log_length, NULL, info_log);
    fprintf(stderr, "Shader compilation error: %s\n", info_log);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

static GLuint create_shader_program(const char *vertex_shader_source,
                                    const char *fragment_shader_source) {
  __attribute__((cleanup(cleanup_shader))) GLuint vertex_shader =
      create_shader(GL_VERTEX_SHADER, vertex_shader_source);
  if (!vertex_shader)
    return 0;

  __attribute__((cleanup(cleanup_shader))) GLuint fragment_shader =
      create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
  if (!fragment_shader) {
    glDeleteShader(vertex_shader);
    return 0;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLint log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    char *info_log = malloc(log_length);
    glGetProgramInfoLog(program, log_length, NULL, info_log);
    fprintf(stderr, "Program linking error: %s\n", info_log);
    free(info_log);
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

static bool create_mesh(Mesh *mesh, const float *vertices, size_t vertex_count,
                        const char *vertex_shader,
                        const char *fragment_shader) {

  glGenVertexArrays(1, &mesh->vao);
  glBindVertexArray(mesh->vao);

  glGenBuffers(1, &mesh->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, vertex_count * 6 * sizeof(float), vertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  mesh->shader_program = create_shader_program(vertex_shader, fragment_shader);
  if (!mesh->shader_program) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    return false;
  }

  mesh->vertex_count = vertex_count;
  return true;
}

static void render_mesh(const Mesh *mesh, const mat4 model, const mat4 view,
                        const mat4 projection) {
  glUseProgram(mesh->shader_program);

  glUniformMatrix4fv(glGetUniformLocation(mesh->shader_program, "uModel"), 1,
                     GL_FALSE, (float *)model);
  glUniformMatrix4fv(glGetUniformLocation(mesh->shader_program, "uView"), 1,
                     GL_FALSE, (float *)view);
  glUniformMatrix4fv(glGetUniformLocation(mesh->shader_program, "uProjection"),
                     1, GL_FALSE, (float *)projection);

  glBindVertexArray(mesh->vao);
  glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  (void)scancode;
  (void)mods;
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  (void)window;
  glViewport(0, 0, width, height);
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  static bool first_mouse = true;
  static float last_x = 0.0f;
  static float last_y = 0.0f;

  if (first_mouse) {
    last_x = (float)xpos;
    last_y = (float)ypos;
    first_mouse = false;
  }

  float xoffset = (float)xpos - last_x;
  float yoffset = last_y - (float)ypos;

  last_x = (float)xpos;
  last_y = (float)ypos;

  Camera *camera = (Camera *)glfwGetWindowUserPointer(window);
  process_mouse_movement(camera, xoffset, yoffset, true);
}

static GLFWwindow *create_window(int width, int height, const char *title) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return NULL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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
  glfwSwapInterval(1);

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return NULL;
  }

  return window;
}

int main(void) {
  __attribute__((cleanup(cleanup_glfw_window))) GLFWwindow *window =
      create_window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    return EXIT_FAILURE;
  }

  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Camera camera;
  vec3 camera_pos = {0.0f, 0.0f, 3.0f};
  vec3 camera_up = {0.0f, 1.0f, 0.0f};
  init_camera(&camera, camera_pos, camera_up, -90.0f, 0.0f);
  glfwSetWindowUserPointer(window, &camera);

  __attribute__((cleanup(cleanup_mesh))) Mesh cube;
  if (!create_mesh(&cube, CUBE_VERTICES, 36, CUBE_VERTEX_SHADER,
                   CUBE_FRAGMENT_SHADER)) {
    fprintf(stderr, "Failed to create cube mesh\n");
    return EXIT_FAILURE;
  }

  glEnable(GL_DEPTH_TEST);

  float delta_time = 0.0f;
  float last_frame = 0.0f;

  while (!glfwWindowShouldClose(window)) {

    float current_frame = (float)glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    process_keyboard(&camera, window, delta_time);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    mat4 view, projection;
    get_view_matrix(&camera, view);
    glm_perspective(glm_rad(camera.fov), (float)width / (float)height,
                    NEAR_PLANE, FAR_PLANE, projection);

    mat4 model;
    glm_mat4_identity(model);

    render_mesh(&cube, model, view, projection);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return EXIT_SUCCESS;
}
