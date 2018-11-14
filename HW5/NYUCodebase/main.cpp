#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>

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
using namespace std;
SDL_Window* displayWindow;
ShaderProgram program;


glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);

class soundBar{
public:
    Mix_Chunk* hit;
    Mix_Chunk* miss;
    Mix_Music* music;
};


void drawBordersAnd(){
    float vertices[] = {-2.0f,-.1f, 2.0f,-0.1f, 2.0f,0.1f, -2.0f,-0.1f,2.0f,0.1f,-2.0f,0.1f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    //Up
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(0.0f,1.0f,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Down
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(0.0f,-1.0f,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Dotted Lines
    float middle[] = {-0.04f,-0.04f, 0.04f,-0.04f, 0.04f,0.04f, -0.04f,-0.04f, 0.04f,0.04f, -0.04f,0.04f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,middle);
    glEnableVertexAttribArray(program.positionAttribute);
    
    float y = -0.9f;
    for(int x = 0; x < 13; x++){
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix,glm::vec3(0.0f,y,0.0f));
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        y += .15;
    }
}

SDL_Event event;
bool done = false;
float lastFrameTicks = 0.0f;
float movement = 0.0f;
float ticks = 0;
float elapsed = 0;
float ballPositionX = 0;
float ballPositionY = 0;
float rightY = 0;
float leftY = 0;
float collisionX = 0;
float collisionY = 0;
float modifierX = 1;
float modifierY = 1;
#define FIXED_TIMESTEP 0.01666666f
#define MAX_TIMESTEPS 6
float accumulator = 0.0f;
soundBar sound;

void Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("HW1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    #ifdef _WINDOWS
        glewInit();
    #endif
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    sound.hit = Mix_LoadWAV("hit.wav");
    sound.miss = Mix_LoadWAV("miss.wav");
    sound.music =Mix_LoadMUS("Background.mp3");
    
    Mix_VolumeChunk(sound.hit, 10);
    Mix_VolumeMusic(30);
    Mix_PlayMusic(sound.music, -1);
    
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Update(float elapsed){
    glClear(GL_COLOR_BUFFER_BIT);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    // Up/Down to move right
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_UP] and rightY < 0.68){
        rightY += elapsed * 0.7;
    }
    if(keys[SDL_SCANCODE_DOWN] and rightY > -0.68){
        rightY -= elapsed * 0.7;
    }
    // W/S to move left
    if(keys[SDL_SCANCODE_W] and leftY < 0.68){
        leftY += elapsed * 0.7;
    }
    if(keys[SDL_SCANCODE_S] and leftY >  -0.68){
        leftY -= elapsed * 0.7;
    }
    
    // Top or Bottom Collision
    if ((abs(ballPositionY - 1.0f) - (0.1f + 0.2f)/2) <= 0 or (abs(ballPositionY + 1.0f) - (0.1f + 0.2f)/2) <= 0){
        modifierY *= -1;
        Mix_PlayChannel(-1, sound.hit, 0);
    }
    
    // MovingBox Collision and End Game Collision
    if (abs(ballPositionY - rightY) - (0.3f) <= 0 and abs(ballPositionX - 1.73f) - (0.1f) <= 0){
        modifierX *= -1;
        Mix_PlayChannel( -1, sound.hit, 0);
    }
    else if (abs(ballPositionY - leftY) - (0.3f) <= 0 and abs(ballPositionX + 1.73f) - (0.1f) <= 0){
        modifierX *= -1;
        Mix_PlayChannel( -1, sound.hit, 0);
    }
    else if ((abs(ballPositionX - 1.8f) - 0.1f) <= 0 or (abs(ballPositionX + 1.8f) - (0.1f) <= 0)){
        ballPositionY = 0;
        ballPositionX = 0;
        modifierX = 1;
        modifierY = 1;
        Mix_PlayChannel( -1, sound.miss, 0);
    }
    
    ballPositionX += modifierX * elapsed * 0.85;
    ballPositionY += modifierY * elapsed * 0.85;
}


void Render(){
    
    drawBordersAnd();
    
    //Moving Blocks
    float boxes[] = {-0.05f,-0.25f, 0.05f,-0.25f, 0.05f,0.25f, -0.05f,-0.25f, 0.05f,0.25f, -0.05f,0.25f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,boxes);
    glEnableVertexAttribArray(program.positionAttribute);
    
    //Right
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(1.73f,rightY,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Left
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(-1.73f,leftY,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Pong Ball
    float ball[] = {-0.05f,-0.05f, 0.05f,-0.05f, 0.05f,0.05f, -0.05f,-0.05f, 0.05f,0.05f, -0.05f,0.05f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,ball);
    glEnableVertexAttribArray(program.positionAttribute);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(ballPositionX,ballPositionY,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
    Setup();
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP){
            accumulator = elapsed;
            continue;
        }
        while(elapsed >= FIXED_TIMESTEP){
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        Render();
    }
    Mix_FreeChunk(sound.hit);
    Mix_FreeMusic(sound.music);
    SDL_Quit();
    return 0;
}
