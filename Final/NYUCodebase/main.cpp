#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <vector>
#include <deque>
#include <random>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#define FIXED_TIMESTEP 0.016666f
#define MAX_TIMESTEPS 6
using namespace std;
using namespace glm;
enum GameMode {TITLE_SCREEN,SENT_BACK,GAME_SCREEN1,ADVANTAGE_SCREEN,GAME_SCREEN2,WINNING_SCREEN,GAME_SCREEN3,GAME_OVER};
GameMode mode = TITLE_SCREEN;

//---------------------------------------------------Variables--------------------------------------------------------------
//Universal Stuff
SDL_Window* displayWindow;
ShaderProgram program;
ShaderProgram textprogram;
GLuint font_texture;

//Matrices
mat4 projectionMatrix = mat4(1.0f);
mat4 viewMatrix = mat4(1.0f);
mat4 modelMatrix = mat4(1.0f);
mat4 tailMatrix = mat4(1.0f);
mat4 pUpMatrix = mat4(1.0f);
mat4 ballMatrix = mat4(1.0f);
mat4 textMatrix = mat4(1.0f);

//Game stats
int p1score = 0;
int p2score = 0;
bool powerUpPlaced = false;

float accumulator = 0.0f;
SDL_Event event;
bool done = false;
float lastFrameTicks = 0.0f;

//Ball movements
float ballPositionX = 0;
float ballPositionY = 0;
float modifierX = 1;
float modifierY = 1;
bool moveBall = false;
float bounce = 1;
int hold = 0;
float angle = -45.0f * (3.1415926f/180.0f);

//Block movements
float rightY = 0;
float leftY = 0;
float upY = 0;
float downY = 0;
float adj = 1;
float prevadj = 0;

float itemX = -10;
float itemY = -10;

//Dimensions for Screen 2, flip for S1 & 3
float boxH = 0.1f;
float boxW = 0.9f;
float borH = 3.45f;
float borW = 0.2f;
float ballSize = 0.1f;
float pSize = 0.2f;

//Sound Variables
Mix_Chunk *endSound;
Mix_Chunk *powSound;
Mix_Chunk *hitSound;
Mix_Music *bgMusic;

//Data
float boxes[] = {-0.05f,-0.25f, 0.05f,-0.25f, 0.05f,0.25f, -0.05f,-0.25f, 0.05f,0.25f, -0.05f,0.25f};
float boxes2[] = {-0.45f,-0.05f, 0.45f,-0.05f, 0.45f,0.05f, -0.45f,-0.05f, 0.45f,0.05f, -0.45f,0.05f};
float ball[] = {-0.05f,-0.05f, 0.05f,-0.05f, 0.05f,0.05f, -0.05f,-0.05f, 0.05f,0.05f, -0.05f,0.05f};
float pUp[] = {-0.1f,-0.1f,0.1f,-0.1f,0.1f,0.1f,-0.1f,-0.1f,0.1f,0.1f,-0.1f,0.1f};
deque<float> prevCords;
random_device rd;
//-----------------------------------------------Functions to Load/Write--------------------------------------------------------
void DrawText(ShaderProgram &program, int fontTexture, string text, float size, float spacing, float x, float y) {
    float char_size = 1.0 / 16.0f;
    
    vector<float> vertexData;
    vector<float> texCoordData;
    
    for (int i = 0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + char_size,
            texture_x + char_size, texture_y,
            texture_x + char_size, texture_y + char_size,
            texture_x + char_size, texture_y,
            texture_x, texture_y + char_size,
        });
    }
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glUseProgram(program.programID);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    textMatrix = mat4(1.0f);
    textMatrix = translate(textMatrix, glm::vec3(x,y,0.0f));
    program.SetModelMatrix(textMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());
}
GLuint LoadTexture(const char* filePath) {
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "unable to load image. Make sure the path is correct \n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

//------------------------------------------------Functions For Drawing------------------------------------------------------
void drawBordersHor(){
    float vertices[] = {-1.78f,-.1f, 1.78f,-0.1f, 1.78f,0.1f, -1.78f,-0.1f,1.78f,0.1f,-1.78f,0.1f};
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
void drawBordersVer(){
    float vertices[] = {-.1f,-3.45f, .1f,-3.45f, 0.1f,3.45f, -0.1f,-3.45f,0.1f,3.45f,-0.1f,3.45f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    
    //Right
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(1.77f,0.0f,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Left
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(-1.77f,0.0f,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Dotted Lines
    float middle[] = {-0.04f,-0.04f, 0.04f,-0.04f, 0.04f,0.04f, -0.04f,-0.04f, 0.04f,0.04f, -0.04f,0.04f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,middle);
    glEnableVertexAttribArray(program.positionAttribute);
    
    float x = -1.57f;
    for(int y = 0; y < 22; y++){
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix,glm::vec3(x,0.0f,0.0f));
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += .15;
    }
}

//For tail affect
void adjustcoords(float x, float y){
    prevCords.push_front(y);
    prevCords.push_front(x);
    if(prevCords.size() > 30){
        //Pops last x,y
        prevCords.pop_back();
        prevCords.pop_back();
    }
}
void reset(){
    ballPositionY = 0;
    ballPositionX = 0;
    upY = 0;
    rightY = 0;
    downY = 0;
    leftY = 0;
    
    modifierX = 1;
    modifierY = 1;
    adj = 1;
    prevadj = 0;
    moveBall = false;
    powerUpPlaced = false;
    
    itemX = -10;
    itemY = -10;
    prevCords.clear();
}
//Removes sounds
void CleanSounds() {
    Mix_FreeChunk(powSound);
    Mix_FreeChunk(endSound);
    Mix_FreeChunk(hitSound);
    
    Mix_FreeMusic(bgMusic);
}
//Randomly Generate Stuff
void generateItem(float xneg, float xpos, float yneg, float ypos){
    uniform_real_distribution<float> dist(xneg,xpos);
    uniform_real_distribution<float> dist2(yneg,ypos);
    itemX = dist(rd);
    itemY = dist2(rd);
}

void generateStarting(){
    uniform_real_distribution<float> dist(0,1);
    uniform_real_distribution<float> dist2(0,1);
    modifierX = dist(rd);
    modifierY = dist2(rd);
}
//Draw Powerups
//Item is twice size of ball
//Will be randomly generated somewhere on the field
void drawPowerUp(){
    program.SetColor(1.0f,0.0f,0.0f,1.0f);
    if(powerUpPlaced == false){
        generateItem(-1.777 + boxH + pSize, 1.777f - boxH - pSize, -1.0 + borW + pSize, 1.0f - borW - pSize);
        powerUpPlaced = true;
    }
    if(itemX != -10.0f and itemY != -10.0f){
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,pUp);
        glEnableVertexAttribArray(program.positionAttribute);
        
        pUpMatrix = mat4(1.0f);
        pUpMatrix = translate(pUpMatrix,vec3(itemX,itemY,1.0f));
        program.SetModelMatrix(pUpMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
//RIGHT & DOWN ARE B, LEFT & UP ARE A
void checkCollisions(){
    // Top or Bottom Collision
    if(mode == GAME_SCREEN1 or mode == GAME_SCREEN3){
        if ((abs(ballPositionY - 1.0f) - (0.1f + 0.2f)/2) <= 0 or (abs(ballPositionY + 1.0f) - (0.1f + 0.2f)/2) <= 0){
            modifierY *= -1;
            adj += 0.05;
            Mix_PlayChannel(-1,hitSound,0);
            bounce = 1.5;
        }
        
        // MovingBox Collision
        if (abs(ballPositionY - rightY) - (0.3f) <= 0 and abs(ballPositionX - 1.73f) - (0.1f) <= 0){
            if(prevadj != 0){adj = prevadj; prevadj = 0;}
            modifierX *= -1;
            adj += 0.05;
            bounce = 1.5;
            Mix_PlayChannel(-1,hitSound,0);
        }
        else if (abs(ballPositionY - leftY) - (0.3f) <= 0 and abs(ballPositionX + 1.73f) - (0.1f) <= 0){
            if(prevadj != 0){adj = prevadj; prevadj = 0;}
            modifierX *= -1;
            adj += 0.05;
            bounce = 1.5;
            Mix_PlayChannel(-1,hitSound,0);
        }
        //power up collision
        else if ((abs(ballPositionX - itemX) - (ballSize + pSize)/2 <=0) and (abs(ballPositionY-itemY) - (ballSize + pSize)/2 <= 0)){
            powerUpPlaced = false;
            Mix_PlayChannel(-1, powSound, 0);
            prevadj = adj;
            adj += 1.0;
            bounce = 1.5;
            cout << adj << endl;
        }
        //End game collision
        //RIGHT or PLAYER 1 WINS
        else if (abs(ballPositionX - 1.777f) - (0.1f) <= 0){
            Mix_PlayChannel(-1,endSound,0);
            if(mode == GAME_SCREEN1){
                if(p1score == 0){
                    p1score = 1;
                    p2score = 0;
                    mode = ADVANTAGE_SCREEN;
                    reset();
                }
            }
            else if (mode == GAME_SCREEN3){
                if(p1score == 2){
                    p1score++;
                    mode = GAME_OVER;
                    reset();
                }
                else if(p2score == 2){
                    p2score = 0;
                    p1score = 0;
                    mode = SENT_BACK;
                    reset();
                }
                else{
                    mode = SENT_BACK;
                }
            }
        }
        //LEFT or PLAYER 2 WINS
        else if (abs(ballPositionX + 1.777f) - (0.1f) <= 0){
            Mix_PlayChannel(-1,endSound,0);
            if(mode == GAME_SCREEN1){
                mode = ADVANTAGE_SCREEN;
                p2score++;
                reset();
            }
            else{
                if(p2score == 2){
                    p2score++;
                    mode = GAME_OVER;
                    reset();
                }
                else if(p1score == 2){
                    p2score = 0;
                    p1score = 0;
                    mode = SENT_BACK;
                    reset();
                }
                else{
                    mode = SENT_BACK;
                }
            }
        }
    }
    else if(mode == GAME_SCREEN2){
        //Borders
        if ((abs(ballPositionX - 1.73f) - (0.1f + 0.2f)/2) <= 0 or (abs(ballPositionX + 1.73f) - (0.1f + 0.2f)/2) <= 0){
            modifierX *= -1;
            adj += 0.05;
            bounce = 1.5;
            Mix_PlayChannel(-1,hitSound,0);
        }
        // Box
        //Up
        if (abs(ballPositionY - 0.95) - (boxH+ballSize)/2 <= 0 and abs(ballPositionX - upY) - (boxW+ballSize)/2 <= 0){
            modifierY *= -1;
            adj += 0.05;
            Mix_PlayChannel(-1,hitSound,0);
        }
        //Down
        else if (abs(ballPositionY + 0.95) - (boxH+ballSize)/2 <= 0 and abs(ballPositionX - downY) - (boxW+ballSize)/2 <= 0){
            modifierY *= -1;
            adj += 0.05;
            bounce = 1.5;
            Mix_PlayChannel(-1,hitSound,0);
        }
        //Misses Block
        //TOP or PLAYER B WINS
        else if ((abs(ballPositionY - 1.0f) - 0.1f) <= 0){
            reset();
            Mix_PlayChannel(-1,endSound,0);
            if(p2score == 1){
                p2score++;
                mode = WINNING_SCREEN;
            }else if (p1score == 1){
                mode = SENT_BACK;
                p2score = 0;
                reset();
            }
        }
        //BOTTOM or PLAYER A WINS
        else if (abs(ballPositionY + 1.0f) - (0.1f) <= 0){
            reset();
            Mix_PlayChannel(-1,endSound,0);
            if(p1score == 1){
                p1score++;
                mode = WINNING_SCREEN;
            }else if(p2score == 1){
                mode = SENT_BACK;
                p1score = 0;
                reset();
            }
        }
    }
}

void Setup(){
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Final Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    #ifdef _WINDOWS
        glewInit();
    #endif
    glClearColor(0.50f,0.50f,0.50f,1.0f);
    
    //programs
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    textprogram.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    textprogram.SetProjectionMatrix(projectionMatrix);
    textprogram.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //SOUND
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    endSound = Mix_LoadWAV("end.wav");
    Mix_VolumeChunk(endSound, 50);
    powSound = Mix_LoadWAV("power.wav");
    Mix_VolumeChunk(powSound, 50);
    hitSound = Mix_LoadWAV("hit.wav");
    Mix_VolumeChunk(hitSound, 50);
    bgMusic = Mix_LoadMUS("Null Sleep.mp3");
    Mix_PlayMusic(bgMusic, -1);
}
void Update(float elapsed){
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    // Up/Down to move right
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    checkCollisions();

    if(mode == GAME_SCREEN1 or mode == GAME_SCREEN3){
        if(keys[SDL_SCANCODE_UP] and rightY < 0.635){
            rightY += elapsed * adj;
        }
        if(keys[SDL_SCANCODE_DOWN] and rightY > -0.635){
            rightY -= elapsed * adj;
        }
        // W/S to move left
        if(keys[SDL_SCANCODE_W] and leftY < 0.635){
            leftY += elapsed * adj;
        }
        if(keys[SDL_SCANCODE_S] and leftY >  -0.635){
            leftY -= elapsed * adj;
        }
    }
    else if (mode == GAME_SCREEN2){
        if(keys[SDL_SCANCODE_RIGHT] and upY < 1.215){
            upY += elapsed * adj;
        }
        if(keys[SDL_SCANCODE_LEFT] and upY > -1.215){
            upY -= elapsed * adj;
        }
        // D/A to move left
        if(keys[SDL_SCANCODE_D] and downY < 1.215){
            downY += elapsed * adj;
        }
        if(keys[SDL_SCANCODE_A] and downY >  -1.215){
            downY -= elapsed * adj;
        }
    }
    else if (mode == TITLE_SCREEN or mode == GAME_OVER){
        reset();
    }
    if (moveBall == true){
        adjustcoords(ballPositionX,ballPositionY);
        ballPositionX += modifierX * elapsed * adj * 0.75;
        ballPositionY += modifierY * elapsed * adj * 0.75;
    }
}
void Render(float elapsed){
    glClear(GL_COLOR_BUFFER_BIT);
    program.SetColor(1.0f,1.0f,1.0f,1.0f);
    font_texture = LoadTexture(RESOURCE_FOLDER"font1.png");
    if(mode == TITLE_SCREEN){
        DrawText(textprogram, font_texture, "Welcome To", .25f, 0.0f,-1.15f,0.5f);
        DrawText(textprogram,font_texture,"Ultimate PONG",.225f,0.0f,-1.35f,-0.25f);
    }
    else if(mode == GAME_SCREEN1 or mode == GAME_SCREEN2 or mode == GAME_SCREEN3){
        if(mode == GAME_SCREEN1 or mode == GAME_SCREEN3){
            drawBordersHor();
            
            glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,boxes);
            glEnableVertexAttribArray(program.positionAttribute);
            
            //Right
            program.SetColor(0.0f,1.0f,0.0f,1.0f);
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix,glm::vec3(1.73f,rightY,0.0f));
            program.SetModelMatrix(modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            //Left
            program.SetColor(0.0f, 0.0f, 1.0f, 1.0f);
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix,glm::vec3(-1.73f,leftY,0.0f));
            program.SetModelMatrix(modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            //introduce items
            if(mode == GAME_SCREEN3){
                drawPowerUp();
            }
        }
        else if (mode == GAME_SCREEN2){
            drawBordersVer();
            //Moving Blocks
            glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,boxes2);
            glEnableVertexAttribArray(program.positionAttribute);
            
            //Up
            program.SetColor(0.0f, 0.0f, 1.0f, 1.0f);
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix,glm::vec3(upY,0.95f,0.0f));
            program.SetModelMatrix(modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            //Down
            program.SetColor(0.0f,1.0f,0.0f,1.0f);
            modelMatrix = mat4(1.0f);
            modelMatrix = translate(modelMatrix,vec3(downY,-0.95f,0.0f));
            program.SetModelMatrix(modelMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        //Pong Ball
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,ball);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float scaling = 0;
        for(int x = 0; x < prevCords.size(); x+=2){
            program.SetColor(1.0f,0.0f+scaling,0.0f,1.0f);
            tailMatrix = mat4(1.0f);
            tailMatrix = translate(tailMatrix, vec3(prevCords[x],prevCords[x+1],0.0f));
            tailMatrix = scale(tailMatrix, vec3(1.0f-scaling,1.0f-scaling,1.0f));
            program.SetModelMatrix(tailMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            scaling += 0.05f;
        }
        program.SetColor(1.0f,1.0f,1.0f,1.0f);
        angle += elapsed * 10;
        ballMatrix = mat4(1.0f);
        ballMatrix = translate(ballMatrix, vec3(ballPositionX,ballPositionY,0.0f));
        ballMatrix = scale(ballMatrix, vec3(bounce,bounce,1.0f));
        ballMatrix = rotate(ballMatrix,angle,vec3(0.0f,0.0f,1.0f));
        hold++;
        if(hold == 12){
            bounce = 1;
            hold = 0;
        }
        program.SetModelMatrix(ballMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else if (mode == ADVANTAGE_SCREEN){
        DrawText(textprogram,font_texture,"ADVANTAGE!!!", 0.25f, 0.0f,-1.35f,0.3f);
        if(p1score == 1){
            DrawText(textprogram,font_texture,"Player 1 Is",0.175f,0.0f,-1.0f,-0.1f);
            DrawText(textprogram,font_texture,"Currently Winning!",0.175f,0.0f,-1.45f,-0.3f);
        }
        else if(p2score == 1){
            DrawText(textprogram,font_texture,"Player 2 Is",0.175f,0.0f,-1.0f,-0.1f);
            DrawText(textprogram,font_texture,"Currently Winning!",0.175f,0.0f,-1.45f,-0.3f);
        }
    }
    else if (mode == WINNING_SCREEN){
        DrawText(textprogram,font_texture,"WE'RE AT THE", 0.25f, 0.0f,-1.35f,0.4f);
        DrawText(textprogram,font_texture,"ENDGAME NOW",0.25f, 0.0f,-1.25f,0.12f);
        if(p1score == 2){
            DrawText(textprogram,font_texture,"Player 1 Is",0.175f,0.0f,-1.0f,-0.15f);
            DrawText(textprogram,font_texture,"Currently Winning!",0.175f,0.0f,-1.45f,-0.35f);
        }
        else if(p2score == 2){
            DrawText(textprogram,font_texture,"Player 2 Is",0.175f,0.0f,-1.0f,-0.15f);
            DrawText(textprogram,font_texture,"Currently Winning!",0.175f,0.0f,-1.45f,-0.35f);
        }
    }
    else if (mode == GAME_OVER){
        DrawText(textprogram, font_texture, "GAME OOOVER!!!!", 0.25f, 0.0f,-1.6f,0.4f);
        if(p1score == 3){
            DrawText(textprogram,font_texture,"Player 1 WON!",0.25f,0.0f,-1.45f,0.0f);
        }
        else if(p2score == 3){
            DrawText(textprogram,font_texture,"Player 2 WON!",0.25f,0.0f,-1.45f,0.0f);
        }
        DrawText(textprogram,font_texture,"Press R to Replay",0.15f,0.0f,-1.20f,-0.5f);
    }
    else if (mode == SENT_BACK){
        DrawText(textprogram, font_texture, "THE SCORE HAS", 0.25f, 0.0f,-1.50f,0.5f);
        DrawText(textprogram, font_texture, "BEEN EQAULIZED!", 0.25f, 0.0f, -1.6f, 0.0f);
        DrawText(textprogram, font_texture, "!!!!!!!!!!!!!!",0.25f,0.0f,-1.6f,-0.5f);
    }
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    SDL_GL_SwapWindow(displayWindow);
}
bool Event(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            return true;
        }else if (event.type == SDL_KEYDOWN){
            if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                return true;
            }
            else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                if(mode == TITLE_SCREEN){
                    glUseProgram(program.programID);
                    mode = GAME_SCREEN1;
                }
                else if(mode == ADVANTAGE_SCREEN){
                    mode = GAME_SCREEN2;
                }
                else if(mode == WINNING_SCREEN){
                    mode = GAME_SCREEN3;
                }
                else if(mode == SENT_BACK){
                    mode = GAME_SCREEN1;
                }
                else if(mode != TITLE_SCREEN and mode != ADVANTAGE_SCREEN and mode != WINNING_SCREEN and moveBall == false){
                    moveBall = true;
                    generateStarting();
                }
            }
            else if(event.key.keysym.scancode == SDL_SCANCODE_R && mode == GAME_OVER){
                mode = TITLE_SCREEN;
            }
        }
    
    }
    return false;
}
//-------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    Setup();
    while (!done) {
        //Get Events
        done = Event();
        //Time Stuff
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
        accumulator = elapsed;
        //Render
        Render(FIXED_TIMESTEP);
    }
    CleanSounds();
    SDL_Quit();
    return 0;
}
