#include "state.hpp"


State::State(){
    //Matrices
    projectionMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    tailMatrix = glm::mat4(1.0f);
    pUpMatrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);
    textMatrix = glm::mat4(1.0f);

    //Game stats
    p1score = 0;
    p2score = 0;
    powerUpPlaced = false;
    spikePlaced = false;
    slowPlaced = false;

    accumulator = 0.0f;
    done = false;
    lastFrameTicks = 0.0f;

    //Ball movements
    ballPositionX = 0;
    ballPositionY = 0;
    directX = 1;
    directY = 1;
    moveBall = false;
    angle = -45.0f * (3.1415926f/180.0f);

    //Effects
    colorswap = false;
    adjColor = 0;
    bounce = 1;
    hold = 0;
    show = false;

    //Block movements
    rightY = 0;
    leftY = 0;
    upY = 0;
    downY = 0;
    adj = 1;
    prevadj = 0;

    itemX = {-10,-10,-10};
    itemY = {-10,-10,-10};


    //Dimensions for Screen 2, flip for S1 & 3
    boxH = 0.1f;
    boxW = 0.9f; //Only for S2
    boxW2 = 0.5f; //Only for S1/S3
    borH = 3.45f;
    borW = 0.2f;
    ballSize = 0.1f;
    pSize = 0.2f;
}
