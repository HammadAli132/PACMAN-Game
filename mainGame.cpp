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
#define PLAYERPOSX 1
#define PLAYERPOSY 15
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
pthread_mutex_t ghostHomeLeavingSynchronizor;
int currentGhostToLeave = 0;

class GHOST {
private:
    Sprite sprite; // Ghost sprite
    int turn; // Ghost's turn
    int mode; // Ghost mode (e.g., scatter, chase)

public:
    GHOST(Texture &text, int t, int m = 0) { // Constructor
        this->sprite.setTexture(text);
        this->sprite.setScale(0.3f, 0.3f);
        this->sprite.setPosition(GHOSTHOMEX * CELLSize + 101, GHOSTHOMEY * CELLSize + 100);
        this->turn = t;
        this->mode = m;
    } 
    // Getter for sprite
    Sprite& getSprite() { return sprite; }
    // Setter for sprite
    void setSprite(Sprite sprite) { this->sprite = sprite; }
    // Getter for turn
    int getTurn() const { return turn; }
    // Setter for turn
    void setTurn(int turn) { this->turn = turn; }
    // Getter for mode
    int getMode() const { return mode; }
    // Setter for mode
    void setMode(int mode) { this->mode = mode; }
};

class PLAYER{
private:
    Sprite sprite;
    
public:
    bool moveLeft = false; // boolean for left movement
    bool moveRight = true; // boolean for right movement
    bool moveUp = false; // boolean for up movement
    bool moveDown = false; // boolean for down movement
    PLAYER(Texture& texture){
        this->sprite.setTexture(texture);
        this->sprite.setScale(0.8f, 0.8f);
        this->sprite.setPosition(PLAYERPOSX * CELLSize + 105, PLAYERPOSY * CELLSize + 100);
    }
    // Getter for sprite
    Sprite& getSprite() { return this->sprite; }
    // Setter for sprite
    void setSprite(Sprite sprite) { this->sprite = sprite; }
    //to change the face of pacman
    void changeTexture(Texture& tex){
        this->sprite.setTexture(tex);
        this->sprite.setScale(0.8f, 0.8f);
    }
};

void *PLAYERTHREAD(void *arg){
    PLAYER *player = (PLAYER*) arg;
    
    bool collisionDetected = false; // boolean for collision detection

    while(!threadExit){
        pthread_mutex_lock(&ghostMovementSynchronizor);
        FloatRect playerBounds = player->getSprite().getGlobalBounds();
        // Defining collision rectangles for each side of the player
        FloatRect leftRect(playerBounds.left - 1, playerBounds.top, 1, playerBounds.height); // getting left rect of player
        FloatRect rightRect(playerBounds.left + playerBounds.width, playerBounds.top, 1, playerBounds.height); // getting right rect of player
        FloatRect topRect(playerBounds.left, playerBounds.top - 1, playerBounds.width, 1); // getting top rect of player
        FloatRect bottomRect(playerBounds.left, playerBounds.top + playerBounds.height, playerBounds.width, 1); // getting bottom rect of player

        // checking collisions with walls
        for(int i = 0; i<gridRows; i++){
            for(int j = 0; j<gridCols; j++){
                if (maze1[i][j] == 1) {
                    mazeBox.setPosition(j * CELLSize + 100, i * CELLSize + 100); // placing temporary mazeBox at current location
                    if(player->moveUp && topRect.intersects(mazeBox.getGlobalBounds())){
                        player->moveUp = false;
                    }

                    if(player->moveDown && bottomRect.intersects(mazeBox.getGlobalBounds())){
                        player->moveDown = false;
                    }

                    if(player->moveLeft && leftRect.intersects(mazeBox.getGlobalBounds())){
                        player->moveLeft = false;
                    }

                    if(player->moveRight && rightRect.intersects(mazeBox.getGlobalBounds())){
                        player->moveRight = false;
                    }
                }
            }            
        }
        if (player->moveUp)
            player->getSprite().move(0.0f, -1.0f);
        else if (player->moveLeft)
            player->getSprite().move(-1.0f, 0.0f);
        else if (player->moveRight)
            player->getSprite().move(1.0f, 0.0f);
        else if (player->moveDown)
            player->getSprite().move(0.0f, 1.0f);
        pthread_mutex_unlock(&ghostMovementSynchronizor);
        sleep(milliseconds(5));
    }
    pthread_exit(NULL);    
}

void *GHOSTTHREAD(void *arg) { // this is the ghost thread
    GHOST *ghost = (GHOST *) arg;
    bool moveLeft = false; // boolean for left movement
    bool moveRight = false; // boolean for right movement
    bool moveUp = true; // boolean for up movement
    bool moveDown = false; // boolean for down movement
    bool collisionDetected = false; // boolean for collision detection
    bool leftHome = false;
    // initially only up ward movement is allowed because the ghost has to move up to come out of the home

    while (!threadExit) { // loop iterate until threadExit becomes true
        if (currentGhostToLeave == ghost->getTurn() || leftHome) {
            pthread_mutex_lock(&ghostMovementSynchronizor);
            // Getting the global bounds of the ghost
            FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();

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
                            if (!leftHome) {
                                currentGhostToLeave++;
                                leftHome = true;
                                cout << "Current ghost: " << currentGhostToLeave << endl;
                            }
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
                ghost->getSprite().move(0.0f, -1.0f);
            else if (moveLeft)
                ghost->getSprite().move(-1.0f, 0.0f);
            else if (moveRight)
                ghost->getSprite().move(1.0f, 0.0f);
            else if (moveDown)
                ghost->getSprite().move(0.0f, 1.0f);
            pthread_mutex_unlock(&ghostMovementSynchronizor);
            sleep(milliseconds(5));
        }
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
    GHOST redGhostObj(redGhostTex, 0, 0); // creating a ghost obj

    Texture greenGhostTex;
    greenGhostTex.loadFromFile("sprites/greenGhost.png"); // loading a red ghost png
    GHOST greenGhostObj(greenGhostTex, 1, 0); // creating a ghost obj

    Texture yellowGhostTex;
    yellowGhostTex.loadFromFile("sprites/yellowGhost.png"); // loading a red ghost png
    GHOST yellowGhostObj(yellowGhostTex, 2, 0); // creating a ghost obj

    Texture pinkGhostTex;
    pinkGhostTex.loadFromFile("sprites/pinkGhost.png"); // loading a red ghost png
    GHOST pinkGhostObj(pinkGhostTex, 3, 0); // creating a ghost obj

    Texture blueGhostTex;
    blueGhostTex.loadFromFile("sprites/blueGhost.png"); // loading a red ghost png
    GHOST blueGhostObj(blueGhostTex, 4, 0); // creating a ghost obj

    Texture playerTexLeft;
    playerTexLeft.loadFromFile("sprites/mouthOpenLeft.png"); // loading player png
    Texture playerTexRight;
    playerTexRight.loadFromFile("sprites/mouthOpenRight.png"); // loading player png
    Texture playerTexUp;
    playerTexUp.loadFromFile("sprites/mouthOpenUp.png"); // loading player png
    Texture playerTexDown;
    playerTexDown.loadFromFile("sprites/mouthOpenDown.png"); // loading player png
    PLAYER playerObj(playerTexRight); // creating player obj

    pthread_attr_t detachProp; // setting detachable property
    pthread_attr_init(&detachProp); // initializing that property
    pthread_attr_setdetachstate(&detachProp, PTHREAD_CREATE_DETACHED); // making it detachable
    pthread_t ghostThread[4]; 
    pthread_create(&ghostThread[0], &detachProp, GHOSTTHREAD, (void **) &redGhostObj); // creating a detachable ghost thread
    pthread_create(&ghostThread[1], &detachProp, GHOSTTHREAD, (void **) &greenGhostObj); // creating a detachable ghost thread
    pthread_create(&ghostThread[2], &detachProp, GHOSTTHREAD, (void **) &pinkGhostObj); // creating a detachable ghost thread
    pthread_create(&ghostThread[3], &detachProp, GHOSTTHREAD, (void **) &yellowGhostObj); // creating a detachable ghost thread

    pthread_t playerThread; 
    pthread_create(&playerThread, &detachProp, PLAYERTHREAD, (void **) &playerObj); // creating a detachable player thread
    pthread_attr_destroy(&detachProp);

    while (gameWindow.isOpen()) {
        Event event;
        while (gameWindow.pollEvent(event)) { // checking for window close command
            if (event.type == Event::Closed)
                gameWindow.close();
        }
        // taking user input
        if(Keyboard::isKeyPressed(Keyboard::A)){
            playerObj.changeTexture(playerTexLeft);
            playerObj.moveLeft = true;
            playerObj.moveRight = false;
            playerObj.moveUp = false;
            playerObj.moveDown = false;
        }
        if(Keyboard::isKeyPressed(Keyboard::W)){
            playerObj.changeTexture(playerTexUp);
            playerObj.moveLeft = false;
            playerObj.moveRight = false;
            playerObj.moveUp = true;
            playerObj.moveDown = false;
        }
        if(Keyboard::isKeyPressed(Keyboard::S)){
            playerObj.changeTexture(playerTexDown);
            playerObj.moveLeft = false;
            playerObj.moveRight = false;
            playerObj.moveUp = false;
            playerObj.moveDown = true;
        }
        if(Keyboard::isKeyPressed(Keyboard::D)){
            playerObj.changeTexture(playerTexRight);
            playerObj.moveLeft = false;
            playerObj.moveRight = true;
            playerObj.moveUp = false;
            playerObj.moveDown = false;
        }
        gameWindow.clear(); // clearing the buffer window
        DRAWMAZE(gameWindow, Food, mazeBox); // Drawing the maze with food and mazeBoxes
        gameWindow.draw(redGhostObj.getSprite());
        gameWindow.draw(greenGhostObj.getSprite());
        gameWindow.draw(pinkGhostObj.getSprite());
        gameWindow.draw(yellowGhostObj.getSprite());
        gameWindow.draw(playerObj.getSprite());
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
    pthread_mutex_destroy(&ghostMovementSynchronizor);
    return 0;
}