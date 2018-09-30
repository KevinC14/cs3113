#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filepath){
    int w,h,comp;
    unsigned char* image = stbi_load(filepath,&w,&h,&comp,STBI_rgb_alpha);
    
    if(image == NULL){
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1,&retTexture);
    glBindTexture(GL_TEXTURE_2D,retTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    
    stbi_image_free(image);
    return retTexture;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("HW1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint plane = LoadTexture(RESOURCE_FOLDER"roadTile5.png");
    GLuint road = LoadTexture(RESOURCE_FOLDER"roadTile6.png");
    GLuint actualPlane = LoadTexture(RESOURCE_FOLDER"playerShip1_blue.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_Event event;
    bool done = false;
    
    float lastFrameTicks = 0.0f;
    float movement = 0.0f;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        //FIRST
        glBindTexture(GL_TEXTURE_2D, plane);
        modelMatrix = glm::mat4(1.0f);
        
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float textCoords[] = {0.0,1.0,1.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0};
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,textCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        
        modelMatrix = glm::translate(modelMatrix,glm::vec3(-1.0f,0.0f,0.0f));
        program.SetModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES,0,6);
        
        
        //SECOND
        glBindTexture(GL_TEXTURE_2D,road);
        
        //modelMatrix = glm::translate(modelMatrix,glm::vec3(-1.0f,0.0f,0.0f));
        modelMatrix = glm::mat4(1.0f);
        program.SetModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        //THIRD
        glBindTexture(GL_TEXTURE_2D,actualPlane);
        modelMatrix = glm::mat4(1.0f);
        
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.5f,0.0f,0.0f));
        modelMatrix = glm::scale(modelMatrix,glm::vec3(0.2f,0.2f,1.0f));
        
        movement += elapsed;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(movement,0.0f,0.0f));
        program.SetModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES,0, 6);
        
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
