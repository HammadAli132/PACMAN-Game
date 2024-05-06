#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
using namespace sf;

//Globals
#define GRIDHEIGHT 850
#define GRIDWIDTH 1050
#define CELLSIZE 50
#define GHOSTHOMEX 10
#define GHOSTHOMEY 8
#define PLAYERPOSX 1
#define PLAYERPOSY 15
const int gridRows = GRIDHEIGHT / CELLSIZE;
const int gridCols = GRIDWIDTH / CELLSIZE;
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
// these are open mouth textures
Texture playerTexLeft;
Texture playerTexRight;
Texture playerTexUp;
Texture playerTexDown;
// these are closed mouth textures
Texture playerTexLeftClose;
Texture playerTexRightClose;
Texture playerTexUpClose;
Texture playerTexDownClose;
// this is our circular food
CircleShape Food(5.0f); 
pthread_mutex_t objectMovementSynchronisor;
int currentGhostToLeave = 0;
#define SPEEDBOOST 0.2f
bool playerAte = false;

class GHOST {
private:
    Sprite sprite; // Ghost sprite
    int turn; // Ghost's turn
    int mode; // Ghost mode (e.g., scatter, chase)
    float speed; // this is ghost's speed used for speed boost
    pair<int, int> target; // this is the ghost's target having x and y co-ordinates

public:
    bool moveLeft = false; // boolean for left movement
    bool moveRight = false; // boolean for right movement
    bool moveUp = true; // boolean for up movement
    bool moveDown = false; // boolean for down movement
    GHOST(Texture &text, int t, int m = 0, float sp = 0) { // Constructor
        this->sprite.setTexture(text);
        this->sprite.setScale(0.3f, 0.3f);
        this->sprite.setPosition(GHOSTHOMEX * CELLSIZE + 101, GHOSTHOMEY * CELLSIZE + 100);
        this->turn = t;
        this->mode = m;
        this->speed = sp;
        target = make_pair(10, 6);
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
    // Getter for speed
    float getSpeed() const { return speed; }
    // Setter for speed
    void setSpeed(float sp) { this->speed = sp; }
    // Getter for target
    pair<int, int> getTarget() const { return target; }
    // Setter for target
    void setTarget(pair<int, int> tar) { this->target = tar; }
};

class PLAYER{
private:
    Sprite sprite;
    int score;
    
public:

    bool moveLeft = false; // boolean for left movement
    bool moveRight = true; // boolean for right movement
    bool moveUp = false; // boolean for up movement
    bool moveDown = false; // boolean for down movement

    char directionBuffer;

    PLAYER(Texture& texture){
        this->sprite.setTexture(texture);
        this->sprite.setScale(1.0f, 1.0f);
        this->sprite.setPosition(PLAYERPOSX * CELLSIZE + 100, PLAYERPOSY * CELLSIZE + 100);
        this->score = 0;
        this->directionBuffer = 'R';
    }
    // Getter for sprite
    Sprite& getSprite() { return this->sprite; }
    // Setter for sprite
    void setSprite(Sprite sprite) { this->sprite = sprite; }
    //to change the face of pacman
    void changeTexture(Texture& tex){
        this->sprite.setTexture(tex);
        this->sprite.setScale(1.0f, 1.0f);
    }
    //getter for score
    int getScore() { return this->score; }
    //setter for score;
    void setScore(int Score) { this->score = Score; }
};

void *PLAYERTHREAD(void *arg){
    PLAYER *player = (PLAYER*) arg;
    
    bool collisionDetected = false; // boolean for collision detection

    while(!threadExit){
        pthread_mutex_lock(&objectMovementSynchronisor);
        FloatRect playerBounds = player->getSprite().getGlobalBounds();
        // Defining collision rectangles for each side of the player
        FloatRect leftRect(playerBounds.left - 1, playerBounds.top, 1, playerBounds.height); // getting left rect of player
        FloatRect rightRect(playerBounds.left + playerBounds.width, playerBounds.top, 1, playerBounds.height); // getting right rect of player
        FloatRect topRect(playerBounds.left, playerBounds.top - 1, playerBounds.width, 1); // getting top rect of player
        FloatRect bottomRect(playerBounds.left, playerBounds.top + playerBounds.height, playerBounds.width, 1); // getting bottom rect of player

        // checking collisions with walls
        for(int i = 0; i<gridRows; i++){
            for(int j = 0; j<gridCols; j++){
                if (maze1[i][j] == 1) { // checking if maze can be placed here or not
                    mazeBox.setPosition(j * CELLSIZE + 100, i * CELLSIZE + 100); // placing temporary mazeBox at current location
                    if(player->moveUp && topRect.intersects(mazeBox.getGlobalBounds())){ // if player is moving rightwards and it collides with walls 
                        player->moveUp = false;
                    }
                    else if(player->moveDown && bottomRect.intersects(mazeBox.getGlobalBounds())){ // if player is moving downwards and it collides with walls
                        player->moveDown = false;
                    }
                    else if(player->moveLeft && leftRect.intersects(mazeBox.getGlobalBounds())){ // if player is moving leftwards and it collides with walls
                        player->moveLeft = false;
                    }
                    else if(player->moveRight && rightRect.intersects(mazeBox.getGlobalBounds())){ // if player is moving upwards and it collides with walls
                        player->moveRight = false;
                    }
                }
                //detecting player's collision with food
                if(maze1[i][j] == 0){
                    Food.setPosition((j * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100, (i * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100); //placing temporary Food at current position
                    FloatRect FoodBounds = Food.getGlobalBounds();
                    if(playerBounds.intersects(FoodBounds)){
                        maze1[i][j] = -99;
                        int score = player->getScore();
                        player->setScore(score += 1); //increasing score as the player eats food
                        playerAte = true;
                    }
                }
            }            
        }
        // moving the player accordingly
        if (player->moveUp)
            player->getSprite().move(0.0f, -1.0f);
        else if (player->moveLeft)
            player->getSprite().move(-1.0f, 0.0f);
        else if (player->moveRight)
            player->getSprite().move(1.0f, 0.0f);
        else if (player->moveDown)
            player->getSprite().move(0.0f, 1.0f);
        pthread_mutex_unlock(&objectMovementSynchronisor);
        sleep(milliseconds(5));
    }
    pthread_exit(NULL);    
}

void LEAVEHOME(bool &collisionDetected, bool &leftHome, GHOST *ghost) {
    // Getting the global bounds of the ghost
    FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();

    // Defining collision rectangles for each side of the ghost
    FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height); // getting left rect of ghost
    FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height); // getting right rect of ghost
    FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1); // getting top rect of ghost
    FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1); // getting bottom rect of ghost

    for (int i = 0; i < gridRows; ++i) {
        for (int j = 0; j < gridCols; ++j) {
            if (maze1[i][j] == 1) { // if maze[i][j] is 1 then we can place a temporary mazeBox there
                mazeBox.setPosition(j * CELLSIZE + 100, i * CELLSIZE + 100); // placing temporary mazeBox at current location
                // if ghost is moving up wards and collision is detected above the ghost
                if (ghost->moveUp && topRect.intersects(mazeBox.getGlobalBounds())) {
                    // cout << "X: " << (int)((ghost->getSprite().getPosition().x - 100) / CELLSIZE) << " and Y: " << (int)((ghost->getSprite().getPosition().y - 100) / CELLSIZE) << endl;
                    collisionDetected = true;
                    if (!leftHome) {
                        currentGhostToLeave++;
                        leftHome = true;
                    }
                    while (collisionDetected){
                        int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                        // checking if that direction is available or not
                        switch(randomDirection) {
                            case 0: // for left movement
                                if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                    ghost->moveLeft = true;
                                    collisionDetected = false; // collision detected is set to false
                                }
                            break;
                            case 1: // for right movement
                                if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                    ghost->moveRight = true;
                                    collisionDetected = false; // collision detected is set to false
                                }
                            break;
                            case 2: // for down movement
                                if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                    ghost->moveDown = true; 
                                    collisionDetected = false; // collision detected is set to false
                                }
                            break;
                        }
                    }
                    ghost->moveUp = false;
                }
            }
        }
    }
    return;
}

void MOVESIMPLEGHOST(bool &collisionDetected, bool &leftHome, GHOST *ghost) {
    if (!leftHome)
        LEAVEHOME(collisionDetected, leftHome, ghost);
    else {
        // Getting the global bounds of the ghost
        FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();

        // Defining collision rectangles for each side of the ghost
        FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height); // getting left rect of ghost
        FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height); // getting right rect of ghost
        FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1); // getting top rect of ghost
        FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1); // getting bottom rect of ghost

        for (int i = 0; i < gridRows; ++i) { // Iterating through all maze boxes
            for (int j = 0; j < gridCols; ++j) {
                if (maze1[i][j] == 1) { // if maze[i][j] is 1 then we can place a temporary mazeBox there
                    mazeBox.setPosition(j * CELLSIZE + 100, i * CELLSIZE + 100); // placing temporary mazeBox at current location
                    // if ghost is moving up wards and collision is detected above the ghost
                    if (ghost->moveUp && topRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                        ghost->moveLeft = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveRight = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveDown = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        ghost->moveUp = false;
                    }
                    // if ghost is moving left wards and collision is detected on left side of the ghost
                    if (ghost->moveLeft && leftRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!topRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle above, we allow upward movement
                                        ghost->moveUp = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveRight = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveDown = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        ghost->moveLeft = false;
                    }
                    // if ghost is moving right wards and collision is detected on right side of the ghost
                    if (ghost->moveRight && rightRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                        ghost->moveLeft = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!topRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveUp = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveDown = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        ghost->moveRight = false;
                    }
                    // if ghost is moving down wards and collision is detected below the ghost
                    if (ghost->moveDown && bottomRect.intersects(mazeBox.getGlobalBounds())) {
                        collisionDetected = true; // since collision is detected, so we set it to true
                        while (collisionDetected){
                            int randomDirection = abs(rand() % 3); // we get random direction for our ghost
                            // checking if that direction is available or not
                            switch(randomDirection) {
                                case 0: // for left movement
                                    if (!leftRect.intersects(mazeBox.getGlobalBounds())) { // if there is no obstacle on the left side, we allow left movement
                                        ghost->moveLeft = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 1: // for right movement
                                    if (!rightRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveRight = true;
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                                case 2: // for down movement
                                    if (!topRect.intersects(mazeBox.getGlobalBounds())) {
                                        ghost->moveUp = true; 
                                        collisionDetected = false; // collision detected is set to false
                                    }
                                break;
                            }
                        }
                        ghost->moveDown = false;
                    }
                }
            }
        }
    }
    // Moving the ghost according to the position available
    if (ghost->moveUp)
        ghost->getSprite().move(0.0f, -1.0f - ghost->getSpeed());
    else if (ghost->moveLeft)
        ghost->getSprite().move(-1.0f - ghost->getSpeed(), 0.0f);
    else if (ghost->moveRight)
        ghost->getSprite().move(1.0f + ghost->getSpeed(), 0.0f);
    else if (ghost->moveDown)
        ghost->getSprite().move(0.0f, 1.0f + ghost->getSpeed());
    return;
}

struct Cell {
    int x, y; // Coordinates of the cell
    int distance; // Distance from the starting cell
    Cell(int x, int y, int distance) : x(x), y(y), distance(distance) {}
};

void MOVESEMIINTELLIGENTGHOST(bool &collisionDetected, bool &leftHome, GHOST *ghost) {
    if (!leftHome)
        LEAVEHOME(collisionDetected, leftHome, ghost);
    else {
        srand(time(0));
        // Getting the global bounds of the ghost
        FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();

        // Defining collision rectangles for each side of the ghost
        FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height);
        FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height);
        FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1);
        FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1);

        // cout << "Ghost hits at X: " << ghost->getSprite().getPosition().x - 100 << " and Y: " << ghost->getSprite().getPosition().y - 100 << endl;
        // Check if the ghost has reached its target
        if ((int)((ghost->getSprite().getPosition().x - 100) / CELLSIZE) == ghost->getTarget().first && (int)((ghost->getSprite().getPosition().y - 100) / CELLSIZE) == ghost->getTarget().second) {
            // Update the ghost's target to a new random position
            int newX, newY;
            do {
                newX = rand() % (gridCols - 1) + 1; // Exclude border cells (0th row and column)
                newY = rand() % (gridRows - 1) + 1; // Exclude border cells (0th row and column)
            } while(maze1[newY][newX] == 1); // Check if the new position is valid
            ghost->setTarget(make_pair(newX, newY)); // setting new target for ghost
            cout << "New target has X: " << newX << " and Y: " << newY << endl;
        }

        // getting current position of ghost
        pair<int, int> currentPosition = make_pair((ghostBounds.left - 100) / CELLSIZE, (ghostBounds.top - 100) / CELLSIZE);

        // checking the placement of target w.r.t current position
        if (currentPosition.first < ghost->getTarget().first && currentPosition.second > ghost->getTarget().second) { // this means target is on the upper-right side
            // we only have to move up or right
            cout << "Top right" << endl;
            if (maze1[currentPosition.second + 1][currentPosition.first] == 1) { // if there is a block on right side
                ghost->moveUp = true; // we can only move upwards
                ghost->moveDown = ghost->moveLeft = ghost->moveRight = false;
            }
            else if (maze1[currentPosition.second][currentPosition.first - 1] == 1) { // if there is a block on above
                ghost->moveRight = true; // we can only move rightwards
                ghost->moveDown = ghost->moveLeft = ghost->moveUp = false;
            }
        }
        else if (currentPosition.first > ghost->getTarget().first && currentPosition.second > ghost->getTarget().second) { // this means target is on the upper-left side
            // we only have to move up or left
            cout << "Top left" << endl;
            if (maze1[currentPosition.second - 1][currentPosition.first] == 1) { // if there is a block on left side
                ghost->moveUp = true; // we can only move upwards
                ghost->moveDown = ghost->moveLeft = ghost->moveRight = false;
            }
            else if (maze1[currentPosition.second][currentPosition.first - 1] == 1) { // if there is a block on above
                ghost->moveLeft = true; // we can only move leftwards
                ghost->moveDown = ghost->moveUp = ghost->moveRight = false;
            }
        }
        else if (currentPosition.first < ghost->getTarget().first && currentPosition.second < ghost->getTarget().second) { // this means target is on the lower-right side
            // we only have to move down or right
            cout << "Bottom right" << endl;
            if (maze1[currentPosition.second + 1][currentPosition.first] == 1) { // if there is a block on right side
                ghost->moveDown = true; // we can only move downwards
                ghost->moveUp = ghost->moveLeft = ghost->moveRight = false;
            }
            else if (maze1[currentPosition.second][currentPosition.first + 1] == 1) { // if there is a block below
                ghost->moveRight = true; // we can only move rightwards
                ghost->moveDown = ghost->moveLeft = ghost->moveUp = false;
            }
        }
        else if (currentPosition.first > ghost->getTarget().first && currentPosition.second < ghost->getTarget().second) { // this means target is on the lower-left side
            // we only have to move down or left
            cout << "Bottom left" << endl;
            if (maze1[currentPosition.second - 1][currentPosition.first] == 1) { // if there is a block on left side
                ghost->moveDown = true; // we can only move downwards
                ghost->moveUp = ghost->moveLeft = ghost->moveRight = false;
            }
            else if (maze1[currentPosition.second][currentPosition.first + 1] == 1) { // if there is a block below
                ghost->moveLeft = true; // we can only move leftwards
                ghost->moveDown = ghost->moveUp = ghost->moveRight = false;
            }
        }
    }
    // Moving the ghost according to the position available
    if (ghost->moveUp)
        ghost->getSprite().move(0.0f, -1.0f - ghost->getSpeed());
    else if (ghost->moveLeft)
        ghost->getSprite().move(-1.0f - ghost->getSpeed(), 0.0f);
    else if (ghost->moveRight)
        ghost->getSprite().move(1.0f + ghost->getSpeed(), 0.0f);
    else if (ghost->moveDown)
        ghost->getSprite().move(0.0f, 1.0f + ghost->getSpeed());
    return;
}

void MOVEFULLYINTELLIGENTGHOST(bool &collisionDetected, bool &leftHome, GHOST *ghost) {

}

void *GHOSTTHREAD(void *arg) { // this is the ghost thread
    GHOST *ghost = (GHOST *) arg;
    bool collisionDetected = false; // boolean for collision detection
    bool leftHome = false;
    // initially only up ward movement is allowed because the ghost has to move up to come out of the home

    while (!threadExit) { // loop iterate until threadExit becomes true
        if (currentGhostToLeave == ghost->getTurn() || leftHome) {
            pthread_mutex_lock(&objectMovementSynchronisor);

            if (ghost->getMode() == 0) // this is for simple ghost
                MOVESIMPLEGHOST(collisionDetected, leftHome, ghost);
            else if (ghost->getMode() == 1) // this is for a semi intelligent ghost
                MOVESEMIINTELLIGENTGHOST(collisionDetected, leftHome, ghost);
            else if (ghost->getMode() == 2) // this is for a fully intelligent ghost
                MOVEFULLYINTELLIGENTGHOST(collisionDetected, leftHome, ghost);

            pthread_mutex_unlock(&objectMovementSynchronisor);
            sleep(milliseconds(5));
        }
    }
    pthread_exit(NULL);
}

void DRAWMAZE(RenderWindow &window, CircleShape food, Sprite mazeBox) { // this function draws the maze
    for (int i = 0; i < gridRows; i++) {
        for (int j = 0; j < gridCols; j++) {
            if (maze1[i][j] == 1) {
                mazeBox.setPosition((j * CELLSIZE) + 100, (i * CELLSIZE) + 100); // setting position of mazeBox
                window.draw(mazeBox); // drawing the mazeBox on window so that it can be rendered
            }
            else if (maze1[i][j] == 0) {
                food.setPosition((j * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100, (i * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100); // setting position of food such that it appears in the center of its block
                window.draw(food); // drawing the food on window so that it can be rendered
            }
        }
    }
    return;
}

void *GAMEINIT(void *arg) { // main game thread
    RenderWindow gameWindow(VideoMode(GRIDWIDTH + 200, GRIDHEIGHT + 200), "PACMAN Game", Style::Default);

    box.loadFromFile("sprites/box.png"); // loading the texture with maze.png
    mazeBox.setTexture(box); // setting the Game Grid Sprite to maze texture
    mazeBox.scale(CELLSIZE, CELLSIZE); // scaling the sprite accoring to the cell size to fit in the screen

    Texture redGhostTex;
    redGhostTex.loadFromFile("sprites/redGhost.png"); // loading a red ghost png
    GHOST redGhostObj(redGhostTex, 0, 0); // creating a ghost obj with (texture, startingNumber, mode)
    redGhostObj.setSpeed(SPEEDBOOST);

    Texture greenGhostTex;
    greenGhostTex.loadFromFile("sprites/greenGhost.png"); // loading a red ghost png
    GHOST greenGhostObj(greenGhostTex, 1, 0); // creating a ghost obj with (texture, startingNumber, mode)

    Texture yellowGhostTex;
    yellowGhostTex.loadFromFile("sprites/yellowGhost.png"); // loading a red ghost png
    GHOST yellowGhostObj(yellowGhostTex, 2, 0); // creating a ghost obj with (texture, startingNumber, mode)

    Texture pinkGhostTex;
    pinkGhostTex.loadFromFile("sprites/pinkGhost.png"); // loading a red ghost png
    GHOST pinkGhostObj(pinkGhostTex, 3, 0); // creating a ghost obj with (texture, startingNumber, mode)

    Texture blueGhostTex;
    blueGhostTex.loadFromFile("sprites/blueGhost.png"); // loading a red ghost png
    GHOST blueGhostObj(blueGhostTex, 4, 0); // creating a ghost obj with (texture, startingNumber, mode)

    playerTexLeft.loadFromFile("sprites/mouthOpenLeft.png"); // loading left side player png
    playerTexRight.loadFromFile("sprites/mouthOpenRight.png"); // loading default player png
    playerTexUp.loadFromFile("sprites/mouthOpenUp.png"); // loading upwards player png
    playerTexDown.loadFromFile("sprites/mouthOpenDown.png"); // loading downwards player png

    playerTexLeftClose.loadFromFile("sprites/leftMouthClose.png"); // loading left side player png
    playerTexRightClose.loadFromFile("sprites/rightMouthClose.png"); // loading default player png
    playerTexUpClose.loadFromFile("sprites/upMouthClose.png"); // loading upwards player png
    playerTexDownClose.loadFromFile("sprites/downMouthClose.png"); // loading downwards player png

    PLAYER playerObj(playerTexRight); // creating player obj

    //to display the string "Score" on the upper left corner
    Text displayScoreString;
    Font font;
    font.loadFromFile("sprites/The Hoca.ttf");
    displayScoreString.setFont(font);
    displayScoreString.setCharacterSize(24);
    displayScoreString.setString("Score: ");
    displayScoreString.setFillColor(Color::Red);
    displayScoreString.setPosition(7, 13);

    //to display the value of score on screen
    Text displayScore;
    displayScore.setFont(font);
    displayScore.setCharacterSize(24);
    displayScore.setFillColor(Color::Green);
    displayScore.setPosition(95, 13);

    pthread_attr_t detachProp; // setting detachable property
    pthread_attr_init(&detachProp); // initializing that property
    pthread_attr_setdetachstate(&detachProp, PTHREAD_CREATE_DETACHED); // making it detachable
    pthread_t ghostThread[4]; 
    // pthread_create(&ghostThread[0], &detachProp, GHOSTTHREAD, (void **) &redGhostObj); // creating a detachable ghost thread
    // pthread_create(&ghostThread[1], &detachProp, GHOSTTHREAD, (void **) &greenGhostObj); // creating a detachable ghost thread
    // pthread_create(&ghostThread[2], &detachProp, GHOSTTHREAD, (void **) &pinkGhostObj); // creating a detachable ghost thread
    // pthread_create(&ghostThread[3], &detachProp, GHOSTTHREAD, (void **) &yellowGhostObj); // creating a detachable ghost thread

    pthread_t playerThread; 
    pthread_create(&playerThread, &detachProp, PLAYERTHREAD, (void **) &playerObj); // creating a detachable player thread
    pthread_attr_destroy(&detachProp);

    bool mouthOpened = true;
    while (gameWindow.isOpen()) {
        Event event;
    
        while (gameWindow.pollEvent(event)) { // checking for window close command
            if (event.type == Event::Closed)
                gameWindow.close();
        }

        // taking user input
        if(Keyboard::isKeyPressed(Keyboard::W)) {
            playerObj.moveUp = true;
            playerObj.moveDown = playerObj.moveLeft = playerObj.moveRight = false;
        }
        else if(Keyboard::isKeyPressed(Keyboard::S)) {
            playerObj.moveDown = true;
            playerObj.moveUp = playerObj.moveLeft = playerObj.moveRight = false;
        }
        else if(Keyboard::isKeyPressed(Keyboard::A)) {
            playerObj.moveLeft = true;
            playerObj.moveDown = playerObj.moveUp = playerObj.moveRight = false;
        }
        else if(Keyboard::isKeyPressed(Keyboard::D)) {
            playerObj.moveRight = true;
            playerObj.moveDown = playerObj.moveLeft = playerObj.moveUp = false;
        }
        // if(playerAte && mouthOpened){
        //     if(playerObj.moveUp){
        //         playerObj.changeTexture(playerTexUpClose);   
        //     }
        //     else if(playerObj.moveDown){
        //         playerObj.changeTexture(playerTexDownClose);   
        //     }
        //     else if(playerObj.moveLeft){
        //         playerObj.changeTexture(playerTexLeftClose);    
        //     }
        //     else if(playerObj.moveRight){
        //         playerObj.changeTexture(playerTexRightClose);    
        //     }
        //     mouthOpened = false;
        //     playerAte = false;
        // }
        // else if(playerAte && !mouthOpened){
        //     if(playerObj.moveUp){
        //         playerObj.changeTexture(playerTexUp);
        //     }
        //     else if(playerObj.moveDown){
        //         playerObj.changeTexture(playerTexDown);
        //     }
        //     else if(playerObj.moveLeft){
        //         playerObj.changeTexture(playerTexLeft);
        //     }
        //     else if(playerObj.moveRight){
        //         playerObj.changeTexture(playerTexRight);
        //     }
        //     mouthOpened = true;
        //     playerAte = false;
        // }
        
        gameWindow.clear(); // clearing the buffer window
        DRAWMAZE(gameWindow, Food, mazeBox); // Drawing the maze with food and mazeBoxes
        // gameWindow.draw(redGhostObj.getSprite());
        // gameWindow.draw(greenGhostObj.getSprite());
        // gameWindow.draw(pinkGhostObj.getSprite());
        // gameWindow.draw(yellowGhostObj.getSprite());
        gameWindow.draw(playerObj.getSprite());
        gameWindow.draw(displayScoreString);
        displayScore.setString(to_string(playerObj.getScore())); //converting score to string so that it can be displayed
        gameWindow.draw(displayScore);        
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
    pthread_mutex_destroy(&objectMovementSynchronisor);
    return 0;
}