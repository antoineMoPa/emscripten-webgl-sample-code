#include "SDL.h"
#include "emscripten.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <string>


// Initialize some global variables for later use.
// In a production app, you would want to place these in different classes or namespaces.
GLuint vertexBuffer, vertexArrayObject, vertexPositionAttributeLocation,
    meshBuffer, shaderProgram, timeUniformLocation;
GLint positionAttribute, uvAttribute;
GLfloat vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
bool quitting = false;
SDL_Window *window = NULL;
SDL_GLContext gl_context;

// Utility to dump shader errors
void dumpError(GLuint object) {
  GLint infoLen = 0;

  glGetShaderiv(object, GL_INFO_LOG_LENGTH, &infoLen);

  if (infoLen > 1) {
    GLchar *infoLog = new char[infoLen];

    glGetShaderInfoLog(object, infoLen, NULL, infoLog);
    std::cerr << "Error compiling shader:\n%s\n" << infoLog << "\n";
    delete &infoLog;
  }
}

// Utility to compile a shader
GLuint createShader(GLuint type, std::string code) {
  GLint compiled;
  GLuint shader = glCreateShader(type);

  if (shader == 0)
    return 0;

  const char *shaderStr = code.c_str();

  glShaderSource(shader, 1, &shaderStr, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (!compiled) {
    dumpError(shader);
    glDeleteShader(shader);

    return 0;
  }

  return shader;
}

// Compile and link the fragment and vertex shaders into a shader program.
void loadShaders() {
  std::string vertexShaderStr = "attribute vec4 vertexPosition;\n"
                                "varying vec4 position;\n"
                                "void main()\n"
                                "{\n"
                                "   gl_Position = vertexPosition;\n"
                                "   position = vertexPosition;\n"
                                "}\n";

  std::string fragmentShaderStr =
      "precision mediump float;\n"
      "varying vec4 position;\n"
      "uniform float time;\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4(0.4, 0.1, 0.1 + 0.5 * cos(time * 3.1415 * 2.0 + "
      "position.y * 30.0), 1.0);\n"
      "}\n";

  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderStr);
  fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderStr);

  shaderProgram = glCreateProgram();

  if (shaderProgram == 0)
    return;

  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  GLint linked;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

  if (!linked) {
    dumpError(shaderProgram);
  }

  vertexPositionAttributeLocation =
      glGetAttribLocation(shaderProgram, "vertexPosition");
  timeUniformLocation = glGetUniformLocation(shaderProgram, "time");
}

// Load a triangle into a vertext array object.
void loadMesh() {
  glGenVertexArrays(1, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);
  glGenBuffers(1, &meshBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, meshBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
}

// This function is called at every frame.
// It clears the screen, attaches ther shader program and draws a triangle.
void render() {
  glClearColor(0.3f, 0.0f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shaderProgram);

  // Find the current timestamp
  auto timeSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
  int millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch)
          .count();

  // Update the time in the fragment shader, so we can animate the output.
  glUniform1f(timeUniformLocation, millis % 100000 / 1000.0);

  // Use the triangle that we stored earlier in loadMesh
  glBindVertexArray(vertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(GLfloat) / 3);
}

void update() {
  render();
};

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  // Use a core profile setup.
  window =
      SDL_CreateWindow("C++ Emscripten WebGL", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL);

  gl_context = SDL_GL_CreateContext(window);

  if (gl_context == NULL) {
    std::cout << SDL_GetError() << '\n';
  }

  glEnable(GL_DEPTH_TEST);
  loadShaders();
  loadMesh();

  emscripten_set_main_loop(update, 0, 1);
}
