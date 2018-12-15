//
//  state.hpp
//  NYUCodebase
//
//  Created by Kevin Chen on 12/14/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef state_hpp
#define state_hpp

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

class State{
public:
    State();
    
    //Universal Stuff
    SDL_Window* displayWindow;
    ShaderProgram program;
    ShaderProgram textprogram;
    GLuint font_texture;
    GLuint neon2Pic;
    GLuint neon1Pic;
    
    //Matrices
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 tailMatrix;
    glm::mat4 pUpMatrix;
    glm::mat4 ballMatrix;
    glm::mat4 textMatrix;
    
    //Game stats
    int p1score;
    int p2score;
    bool powerUpPlaced;
    bool spikePlaced;
    bool slowPlaced;
    
    float accumulator;
    SDL_Event event;
    bool done;
    float lastFrameTicks;
    
    //Ball movements
    float ballPositionX;
    float ballPositionY;
    float directX;
    float directY;
    bool moveBall;
    float angle;
    
    //Effects
    bool colorswap;
    float adjColor;
    float bounce;
    int hold;
    bool show;
    
    //Block movements
    float rightY;
    float leftY;
    float upY;
    float downY;
    float adj;
    float prevadj;
    
    std::vector<float> itemX;
    std::vector<float> itemY;
    
    
    //Dimensions for Screen 2, flip for S1 & 3
    float boxH;
    float boxW; //Only for S2
    float boxW2; //Only for S1/S3
    float borH;
    float borW;
    float ballSize;
    float pSize;
    
    //Sound Variables
    Mix_Chunk *endSound;
    Mix_Chunk *powSound;
    Mix_Chunk *hitSound;
    Mix_Music *bgMusic;
    
    //Data
    std::deque<float> prevCords;
    std::random_device rd;
};

#endif /* state_hpp */
