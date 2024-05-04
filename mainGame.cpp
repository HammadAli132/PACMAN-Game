#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
using namespace sf;

//Globals
#define GRIDHEIGHT 850
#define GRIDWIDTH 1050
#define CELLSize 50
#define GHOSTHOMEX 10
#define GHOSTHOMEY 8
const int gridRows = GRIDHEIGHT / CELLSize;
const int gridCols = GRIDWIDTH / CELLSize;
vector<vector<int>> maze1 = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};
bool threadExit = false; // this is a boolean to close all detached threads when the game closes
Sprite mazeBox; // creating a sprite for Game Grid
Texture box; // creating a texture for maze.png
pthread_mutex_t ghostMovementSynchronizor;

void *GHOSTTHREAD(void *arg) { // this is the ghost thread
    Sprite *ghost = (Sprite *) arg;
    bool moveLeft = false; // boolean for left movement
    bool moveRight = false; // boolean for right movement
    bool moveUp = true; // boolean for up movement
    bool moveDown = false; // boolean for down movement
    bool collisionDetected = false; // boolean for collision detection
    // initially only up ward movement is allowed because the ghost has to move up to come out of the home

    while (!threadExit) { // loop iterate until threadExit becomes true
        pthread_mutex_lock(&ghostMovementSynchronizor);
        // Getting the global bounds of the ghost
        FloatRect ghostBounds = ghost->getGlobalBounds();

        // Defining collision rectangles for each side of the ghost
        FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height); // getting left rect of ghost
        FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height); // getting right rect of ghost
        FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1); // getting top rect of ghost
        FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1); // getting bottom rect of ghost

        // Iterating through all maze boxes
        for (int i = 0; i < gridRows; ++i) {
            for (int j = 0; j < gridCols; ++j) {
                if (maze1[i][j] == 1) { // if maze[i][j] is 1 then we can place a temporary mazeBox there
                    mazeBox.setPosition(j * CELLSize + 100, i * CELLSize + 100); // placing temporary mazeBox at current location
                    // if ghost is moving up wards and collision is detected above the ghost
                    if (moveUp && topRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                        moveLeft = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveRight = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveDown = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        moveUp = false;
                    }
                    // if ghost is moving left wards and collision is detected on left side of the ghost
                    if (moveLeft && leftRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!topRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle above, we allow upward movement
                                        moveUp = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveRight = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveDown = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        moveLeft = false;
                    }
                    // if ghost is moving right wards and collision is detected on right side of the ghost
                    if (moveRight && rightRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                        moveLeft = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!topRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveUp = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveDown = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        moveRight = false;
                    }
                    // if ghost is moving down wards and collision is detected below the ghost
                    if (moveDown && bottomRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                        moveLeft = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveRight = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!topRect.intersects(mazeBox.getGlobalBounds())) {
                                        moveUp = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        moveDown = false;
                    }
                }
            }
        }
        // Moving the ghost according to the position available
        if (moveUp)
            ghost->move(0.0f, -1.0f);
        else if (moveLeft)
            ghost->move(-1.0f, 0.0f);
        else if (moveRight)
            ghost->move(1.0f, 0.0f);
        else if (moveDown)
            ghost->move(0.0f, 1.0f);
        pthread_mutex_unlock(&ghostMovementSynchronizor);
        sleep(milliseconds(5));
    }
    pthread_exit(NULL);
}

void DRAWMAZE(RenderWindow &window, CircleShape food, Sprite mazeBox) { // this function draws the maze
    for (int i = 0; i < gridRows; i++) {
        for (int j = 0; j < gridCols; j++) {
            if (maze1[i][j] == 1) {
                mazeBox.setPosition((j * CELLSize) + 100, (i * CELLSize) + 100); // setting position of mazeBox
                window.draw(mazeBox); // drawing the mazeBox on window so that it can be rendered
            }
            else if (maze1[i][j] == 0) {
                food.setPosition((j * CELLSize + ((CELLSize / 2) - 5)) + 100, (i * CELLSize + ((CELLSize / 2) - 5)) + 100); // setting position of food such that it appears in the center of its block
                window.draw(food); // drawing the food on window so that it can be rendered
            }
        }
    }
    return;
}

void *GAMEINIT(void *arg) { // main game thread
    RenderWindow gameWindow(VideoMode(GRIDWIDTH + 200, GRIDHEIGHT + 200), "PACMAN Game", Style::Default);
    CircleShape Food(5.0f); // this is our circular food

    box.loadFromFile("sprites/box.png"); // loading the texture with maze.png
    mazeBox.setTexture(box); // setting the Game Grid Sprite to maze texture
    mazeBox.scale(CELLSize, CELLSize); // scaling the sprite accoring to the cell size to fit in the screen

    Texture redGhostTex;
    redGhostTex.loadFromFile("sprites/redGhost.png"); // loading a red ghost png
    Sprite redGhost;
    redGhost.setTexture(redGhostTex); // making a red ghost sprite
    redGhost.setPosition(GHOSTHOMEX * CELLSize + 101, GHOSTHOMEY * CELLSize + 100); // setting its initial position
    redGhost.setScale(0.3f, 0.3f); // scaling so that it fits nicely in the maze

    Texture greenGhostTex;
    greenGhostTex.loadFromFile("sprites/greenGhost.png"); // loading a red ghost png
    Sprite greenGhost;
    greenGhost.setTexture(greenGhostTex); // making a red ghost sprite
    greenGhost.setPosition(GHOSTHOMEX * CELLSize + 101, GHOSTHOMEY * CELLSize + 100); // setting its initial position
    greenGhost.setScale(0.3f, 0.3f); // scaling so that it fits nicely in the maze

    Texture blueGhostTex;
    blueGhostTex.loadFromFile("sprites/blueGhost.png"); // loading a red ghost png
    Sprite blueGhost;
    blueGhost.setTexture(blueGhostTex); // making a red ghost sprite
    blueGhost.setPosition(GHOSTHOMEX * CELLSize + 101, GHOSTHOMEY * CELLSize + 100); // setting its initial position
    blueGhost.setScale(0.3f, 0.3f); // scaling so that it fits nicely in the maze

    Texture yellowGhostTex;
    yellowGhostTex.loadFromFile("sprites/yellowGhost.png"); // loading a red ghost png
    Sprite yellowGhost;
    yellowGhost.setTexture(yellowGhostTex); // making a red ghost sprite
    yellowGhost.setPosition(GHOSTHOMEX * CELLSize + 101, GHOSTHOMEY * CELLSize + 100); // setting its initial position
    yellowGhost.setScale(0.3f, 0.3f); // scaling so that it fits nicely in the maze

    Texture pinkGhostTex;
    pinkGhostTex.loadFromFile("sprites/pinkGhost.png"); // loading a red ghost png
    Sprite pinkGhost;
    pinkGhost.setTexture(pinkGhostTex); // making a red ghost sprite
    pinkGhost.setPosition(GHOSTHOMEX * CELLSize + 100, GHOSTHOMEY * CELLSize + 100); // setting its initial position
    pinkGhost.setScale(0.3f, 0.3f); // scaling so that it fits nicely in the maze

    pthread_attr_t ghostProp; // setting detachable property
    pthread_attr_init(&ghostProp); // initializing that property
    pthread_attr_setdetachstate(&ghostProp, PTHREAD_CREATE_DETACHED); // making it detachable
    pthread_t ghostThread[4]; 
    pthread_create(&ghostThread[0], &ghostProp, GHOSTTHREAD, (void **) &redGhost); // creating a detachable ghost thread
    pthread_create(&ghostThread[1], &ghostProp, GHOSTTHREAD, (void **) &greenGhost); // creating a detachable ghost thread
    pthread_create(&ghostThread[2], &ghostProp, GHOSTTHREAD, (void **) &pinkGhost); // creating a detachable ghost thread
    pthread_create(&ghostThread[3], &ghostProp, GHOSTTHREAD, (void **) &yellowGhost); // creating a detachable ghost thread

    while (gameWindow.isOpen()) {
        Event event;
        while (gameWindow.pollEvent(event)) { // checking for window close command
            if (event.type == Event::Closed)
                gameWindow.close();
        }
        gameWindow.clear(); // clearing the buffer window
        DRAWMAZE(gameWindow, Food, mazeBox); // Drawing the maze with food and mazeBoxes
        gameWindow.draw(redGhost);
        gameWindow.draw(greenGhost);
        gameWindow.draw(pinkGhost);
        gameWindow.draw(yellowGhost);
        gameWindow.display(); // swapping the buffer window with main window
    }

    threadExit = true;
    pthread_exit(NULL);
}

int main()
{
    pthread_t startGame;
    pthread_create(&startGame, NULL, GAMEINIT, NULL); // initiating main game thread
    pthread_join(startGame, NULL); // waiting for game thread to exit
    return 0;
}