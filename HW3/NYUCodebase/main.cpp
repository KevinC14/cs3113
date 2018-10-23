/*
 Kevin Chen - HW3
 "Village" Invaders
 
 Sorry, I could not figure out how to work out collisions.
 I did everything else, however. 
 */
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
#include <string>
#include <vector>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace glm;

SDL_Window* displayWindow;
ShaderProgram program;
mat4 projectionMatrix = mat4(1.0f);
mat4 modelMatrix = mat4(1.0f);
mat4 viewMatrix = mat4(1.0f);

enum GameMode {TITLE_SCREEN, GAME_SCREEN};
GameMode mode = TITLE_SCREEN;
bool done = false;

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

class SheetSprite{
public:
    SheetSprite(){}
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size, float x, float y) : textureID(textureID), u(u), v(v), width(width), height(height), size(size), x(x), y(y){}
    void Draw(ShaderProgram& program){
        glBindTexture(GL_TEXTURE_2D,textureID);
        
        GLfloat texCoords[] = {u,v+height, u+width, v, u,v, u+width,v, u,v+height, u+width,v+height};
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        float aspect = width/height;
        float vertices[] = {-0.5f*size*aspect, -0.5f*size, 0.5f*size*aspect, 0.5f*size, -0.5f*size*aspect, 0.5f*size, 0.5f*size*aspect, 0.5f*size, -0.5f*size*aspect, -0.5f*size, 0.5f*size*aspect, -0.5f*size};
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x,y,0.0f));
        program.SetModelMatrix(modelMatrix);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    float x;
    float y;
    float u;
    float v;
    float width;
    float height;
    float size;
    unsigned int textureID;
};
void DrawText(ShaderProgram& program, int fontTexture, std::string text, float size, float spacing, float y){
    float character_size = 1.0f/16.0f;
    
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for(int i = 0; i < text.size(); i++){
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.35f * size), 0.35f*size,
            ((size+spacing) * i) + (-0.35f * size), -0.35f*size,
            ((size+spacing) * i) + (0.35f * size), 0.35f*size,
            ((size+spacing) * i) + (0.35f * size), -0.35f*size,
            ((size+spacing) * i) + (0.35f * size), 0.35f*size,
            ((size+spacing) * i) + (-0.35f * size), -0.35f*size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x,texture_y,
            texture_x,texture_y+character_size,
            texture_x + character_size,texture_y,
            texture_x + character_size,texture_y + character_size,
            texture_x + character_size,texture_y,
            texture_x,texture_y + character_size,
        });
    }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    float* v = vertexData.data();
    float* t = texCoordData.data();
    float x = -1.3f;
    
    for(int i = 0; i < text.size(); i++){
        float vx[] = {v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],v[11]};
        float tx[] = {t[0],t[1],t[2],t[3],t[4],t[5],t[6],t[7],t[8],t[9],t[10],t[11]};
        glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,tx);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,vx);
        glEnableVertexAttribArray(program.positionAttribute);
        
        modelMatrix = mat4(1.0f);
        modelMatrix = translate(modelMatrix, glm::vec3(x,y,0.0f));
        program.SetModelMatrix(modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        v += 12;
        t += 12;
        
    }
}
class Entity{
public:
    Entity(){}
    Entity(float x, float y, ShaderProgram& program): x(x), y(y), program(program){}
    float x;
    float y;
    float velocity = 2.0;
    float timeAlive;
    bool alive = true;
    SheetSprite sprite;
    ShaderProgram program;
    
    void Draw(){
        sprite.Draw(program);
    }

};
#define MAX_BULLETS 15
int bulletIndex = 0;
class GameState{
public:
    Entity player;
    Entity enemies[24];
    Entity bullets[MAX_BULLETS];
};

GameState state;
SDL_Event event;
float lastFrameTicks = 0;




void shootBullet(GameState& state){
    state.bullets[bulletIndex].x = state.player.sprite.x;
    state.bullets[bulletIndex].y = state.player.sprite.y + 0.05;
    bulletIndex++;
    if(bulletIndex > MAX_BULLETS - 1){
        bulletIndex = 0;
    }
}

void drawBullets(float x,float y){
    program.SetColor(0,0,0,1.0f);
    float bullet[] = {-0.05f,-0.06f, 0.05f,-0.06f, 0.05f,0.06f, -0.05f,-0.06f, 0.05f,0.06f, -0.05f,0.06f};
    glVertexAttribPointer(program.positionAttribute,2,GL_FLOAT,false,0,bullet);
    glEnableVertexAttribArray(program.positionAttribute);
    
    float textCoords[] = {0.0,1.0,1.0,1.0,1.0,0.0,0.0,1.0,1.0,0.0,0.0,0.0};
    glVertexAttribPointer(program.texCoordAttribute,2,GL_FLOAT,false,0,textCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x,y,0.0f));
    program.SetModelMatrix(modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
bool collide(float x1, float x2, float y1, float y2, float w1, float w2,float h1, float h2,float size){
    float enemyWidth = size * (w2/h2);
    float enemyHeight = size;
    float pW = abs(x1 - x2) - (w1 + enemyWidth)/2;
    float pH = abs(y1-y2) - (h1 + enemyHeight)/2;
    if(pW <= 0 and pH <= 0){return true;}
    return false;
}
void Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("HW3 - Village Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    
    SDL_GL_MakeCurrent(displayWindow, context);
    #ifdef _WINDOWS
        glewInit();
    #endif

    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //Add Initial Invaders Here
    GLuint sprites = LoadTexture(RESOURCE_FOLDER"george_0.png");
    float u = (float)(((int)0) % 4) / (float)4;
    float v = (float)(((int)0) / 4) / (float)4;
    float x = -0.75f;
    float y = 0.8f;
    int count = 0;
    
    for (int i = 0; i < 24; i++){
        Entity enemy(x, y,program);
        SheetSprite newSprite(sprites, u, v, (1.0/4), (1.0/4), 0.30f, enemy.x, enemy.y);
        enemy.sprite = newSprite;
        state.enemies[i] = enemy;
        count += 1;
        if(count == 6){
            x = -0.75f;
            y -= 0.3f;
            count = 0;
        }else{x += .30f;}
    }
    
    GLuint playersprite = LoadTexture(RESOURCE_FOLDER"betty_0.png");
    u = (float)(((int)2) % 4) / (float)4;
    v = (float)(((int)2) / 4) / (float)4;
    Entity player(0.0f, -0.85f,program);
    SheetSprite newPlSprite(playersprite, u, v, (1.0/4), (1.0/4), 0.3f, player.x, player.y);
    player.sprite = newPlSprite;
    state.player = player;
    
    for (int i = 0; i < MAX_BULLETS; i++){
        state.bullets[i].x = -2000.0f;
    }
}

#define FIXED_TIMESTEP 0.01666666f
#define MAX_TIMESTEPS 6
float accumulator = 0.0f;
int direction = -1;

void Update(GameState &state,float elapsed){
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    //Enemy Movement
    for (int i = 0; i < 24; i++){
        if(state.enemies[i].alive == true){
            state.enemies[i].sprite.x += direction * 0.3 * elapsed;
            if(state.enemies[i].sprite.x <= -1.7f or state.enemies[i].sprite.x >= 1.7f){
                direction *= -1;
            }
        }
    }
    //Player Movement
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_RIGHT] and state.player.sprite.x <= 1.7f){
        state.player.sprite.x += elapsed * 0.7;
    }
    if(keys[SDL_SCANCODE_LEFT] and state.player.sprite.x >= -1.7f){
        state.player.sprite.x -= elapsed * 0.7;
    }
    //Bullet Movement
    for(int i = 0; i < MAX_BULLETS; i++){
        state.bullets[i].y += state.bullets[i].velocity * 0.5 * elapsed;
    }
    for(int j = 0; j < 24; j++){
        for(int i = 0; i < MAX_BULLETS; i++){
            float x1 = state.bullets[i].x;
            float y1 = state.bullets[i].y;
            SheetSprite sub = state.enemies[i].sprite;
            
            if (state.enemies[j].alive == true and collide(x1, sub.x, y1, sub.y, 0.01, sub.width,0.01, sub.height,sub.size) == true){
                state.enemies[j].alive = false;
                state.bullets[i].y = -2000.0f;
            }
            
        }
    }
    
}
void Render(GameMode mode,GameState& state){
    if(mode == TITLE_SCREEN){
        //DRAW TITLE
        GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");

        DrawText(program, fontTexture, "The Village", 0.27f, 0.0f,0.6f);
        DrawText(program, fontTexture, "Press Space", 0.25,0,0.3f);
        DrawText(program, fontTexture, "To Shoot",0.25f,0,0.0f);
        DrawText(program, fontTexture, "Left/Right", 0.25f,0,-0.3f);
        DrawText(program,fontTexture, "To Move", 0.25f, 0,-0.6f);
    }
    else if (mode == GAME_SCREEN){
        for (int i = 0; i < 24; i++){
            if(state.enemies[i].alive == true){
                state.enemies[i].Draw();
            }
        }
        state.player.Draw();
        GLuint bulletTexture = LoadTexture(RESOURCE_FOLDER"rpgTile217.png");
        glBindTexture(GL_TEXTURE_2D, bulletTexture);
        for (int i = 0; i < MAX_BULLETS; i++){
            if(state.bullets[i].y >= -1){
                drawBullets(state.bullets[i].x,state.bullets[i].y);
            }
        }
    }
    
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
            }else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && mode == TITLE_SCREEN){
                    mode = GAME_SCREEN;
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && mode == GAME_SCREEN){
                    shootBullet(state);
                }
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
            Update(state, FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }

        Render(mode, state);
    }
    SDL_Quit();
    return 0;
}
