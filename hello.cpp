#include "SDL.h"
#include <SDL_image.h>
#include <GLES2/gl2.h>
#include "emscripten.h"
#include <iostream>
#include <fstream>
#include <vector>

static bool quitting = false;
static SDL_Window *window = NULL;
static SDL_GLContext gl_context;

GLuint vertexBuffer, vertexArrayObject, shaderProgram;
GLint positionAttribute, uvAttribute;
int textures[4];

void loadBufferData(){
    // vertex position, uv
    float vertexData[24] = {
            -0.5, -0.5, 0.0, 1.0 ,  0.0, 0.0,
            -0.5,  0.5, 0.0, 1.0 ,  0.0, 1.0,
            0.5,  0.5, 0.0, 1.0 ,  1.0, 1.0,
            0.5, -0.5, 0.0, 1.0 ,  1.0, 0.0,
    };
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(float), vertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttribute);
    glEnableVertexAttribArray(uvAttribute);
    int vertexSize =sizeof(float)*6;
    glVertexAttribPointer(positionAttribute, 4, GL_FLOAT, GL_FALSE,vertexSize , (const GLvoid *)0);
    glVertexAttribPointer(uvAttribute  , 2, GL_FLOAT, GL_FALSE, vertexSize, (const GLvoid *)(sizeof(float)*4));
}

void render() {

    SDL_GL_MakeCurrent(window, gl_context);

    glClearColor( 255.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    SDL_GL_SwapWindow(window);
}

void update(){
    SDL_Event event;
    while( SDL_PollEvent(&event) ) {
        if(event.type == SDL_QUIT) {
            quitting = true;
        }
    }

    render();
};

int main(int argc, char *argv[]) {
    SDL_Renderer *renderer = NULL;
    SDL_CreateWindowAndRenderer(512, 512, SDL_WINDOW_OPENGL, &window, &renderer);

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Use a core profile setup.
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL);

    gl_context = SDL_GL_CreateContext(window);

    if (gl_context == NULL) {
        std::cout << SDL_GetError() << '\n';
    }

    std::cout << (gl_context == NULL) << '\n';

    glEnable(GL_DEPTH_TEST);
    loadBufferData();

    emscripten_set_main_loop(update, 0, 1);
}
