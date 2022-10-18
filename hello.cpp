#include "SDL.h"
#include "emscripten.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <string>

static bool quitting = false;
static SDL_Window *window = NULL;
static SDL_GLContext gl_context;

GLuint vertexBuffer, vertexArrayObject, vPositionLocation, meshBuffer,
    shaderProgram;
GLint positionAttribute, uvAttribute;
int textures[4];
GLfloat vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

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

void loadShaders() {
  std::string vertexShaderStr =
      "attribute vec4 vPosition;                   \n"
      "void main()                                 \n"
      "{                                           \n"
      "   gl_Position = vPosition;                 \n"
      "}                                           \n";

  std::string fragmentShaderStr =
      "precision mediump float;                            \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = vec4( 0.0, 1.0, 0.0, 1.0 );        \n"
      "}                                                   \n";

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

  vPositionLocation = glGetAttribLocation(shaderProgram, "vPosition");
}

void loadMesh() {
  glGenVertexArrays(1, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);
  glGenBuffers(1, &meshBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, meshBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
}

void render() {
  SDL_GL_MakeCurrent(window, gl_context);

  glClearColor(255.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shaderProgram);
  glBindVertexArray(vertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(GLfloat) / 3);

  SDL_GL_SwapWindow(window);
}

void update() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      quitting = true;
    }
  }

  render();
};

int main(int argc, char *argv[]) {
  SDL_Renderer *renderer = NULL;
  SDL_CreateWindowAndRenderer(512, 512, SDL_WINDOW_OPENGL, &window, &renderer);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return 1;
  }

  // Use a core profile setup.
  window =
      SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED,
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
