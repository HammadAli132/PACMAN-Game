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
CircleShape Food(5.0f); // this is our circular food
pthread_mutex_t objectMovementSynchronisor;
int currentGhostToLeave = 0;
#define SPEEDBOOST 0.2f

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
    void setSpeed(int sp) { this->speed = sp; }
    // Getter for target
    pair<int, int> getTarget() const { return target; }
    // Setter for target
    void setTarget(pair<int, int> tar) { this->target = tar; }
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
        this->sprite.setPosition(PLAYERPOSX * CELLSIZE + 105, PLAYERPOSY * CELLSIZE + 100);
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
                else if(maze1[i][j] == 0){
                    Food.setPosition((j * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100, (i * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100); //placing temporary Food at current position
                    FloatRect FoodBounds = Food.getGlobalBounds();
                    if(playerBounds.intersects(FoodBounds)){
                        maze1[i][j] = -99;
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
    bool pathFound = false;
    // Getting the global bounds of the ghost
    FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();

    // Defining collision rectangles for each side of the ghost
    FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height);
    FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height);
    FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1);
    FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1);

    // cout << "Ghost hits at X: " << ghost->getSprite().getPosition().x - 100 << " and Y: " << ghost->getSprite().getPosition().y - 100 << endl;


    if (!leftHome)
        LEAVEHOME(collisionDetected, leftHome, ghost);
    else {
        // Check if the ghost has reached its target
        if ((ghost->getSprite().getPosition().x - 100) / CELLSIZE == ghost->getTarget().first && (ghost->getSprite().getPosition().y - 100) / CELLSIZE == ghost->getTarget().second) {
            // Update the ghost's target to a new random position
            int newX = rand() % gridCols;
            int newY = rand() % gridRows;
            ghost->setTarget(make_pair(newX, newY)); // setting new target for ghost
        }

        // Create a visited array to keep track of visited cells
        bool visited[gridRows][gridCols] = {false};

        // Create a queue for BFS
        queue<Cell> q;

        // Starting cell
        int startX = ghostBounds.left / CELLSIZE;
        int startY = ghostBounds.top / CELLSIZE;
        q.push(Cell(startX, startY, 0)); // Add the starting cell to the queue
        visited[startY][startX] = true; // Mark the starting cell as visited

        // Arrays to represent the possible movements (up, down, left, right)
        int dx[] = {0, 0, -1, 1};
        int dy[] = {-1, 1, 0, 0};

        // Perform BFS
        while (!q.empty()) {
            Cell current = q.front(); // Get the front cell from the queue
            q.pop(); // Remove the front cell from the queue

            // Check if the current cell is the target cell
            if (current.x * CELLSIZE + 100 == ghost->getTarget().first && current.y * CELLSIZE + 100 == ghost->getTarget().second) {
                // Move the ghost towards the target
                if (current.x < startX) {
                    ghost->moveLeft = true;
                    ghost->moveRight = ghost->moveUp = ghost->moveDown = false;
                } 
                else if (current.x > startX) {
                    ghost->moveRight = true;
                    ghost->moveLeft = ghost->moveUp = ghost->moveDown = false;
                } 
                else if (current.y < startY) {
                    ghost->moveUp = true;
                    ghost->moveLeft = ghost->moveRight = ghost->moveDown = false;
                } 
                else if (current.y > startY) {
                    ghost->moveDown = true;
                    ghost->moveLeft = ghost->moveRight = ghost->moveUp = false;
                }
                return;
            }

            // Explore adjacent cells
            for (int i = 0; i < 4; ++i) {
                int nextX = current.x + dx[i];
                int nextY = current.y + dy[i];

                // Check if the next cell is within the maze bounds and not visited yet
                if (nextX > 0 && nextX < gridCols && nextY > 0 && nextY < gridRows && !visited[nextY][nextX] && maze1[nextY][nextX] == 0) {
                    q.push(Cell(nextX, nextY, current.distance + 1)); // Add the next cell to the queue
                    visited[nextY][nextX] = true; // Mark the next cell as visited
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

    Texture playerTexLeft;
    playerTexLeft.loadFromFile("sprites/mouthOpenLeft.png"); // loading left side player png
    Texture playerTexRight;
    playerTexRight.loadFromFile("sprites/mouthOpenRight.png"); // loading default player png
    Texture playerTexUp;
    playerTexUp.loadFromFile("sprites/mouthOpenUp.png"); // loading upwards player png
    Texture playerTexDown;
    playerTexDown.loadFromFile("sprites/mouthOpenDown.png"); // loading downwards player png

    Texture playerTexLeftClose;
    playerTexLeftClose.loadFromFile("sprites/leftMouthClose.png"); // loading left side player png
    Texture playerTexRightClose;
    playerTexRightClose.loadFromFile("sprites/rightMouthClose.png"); // loading default player png
    Texture playerTexUpClose;
    playerTexUpClose.loadFromFile("sprites/upMouthClose.png"); // loading upwards player png
    Texture playerTexDownClose;
    playerTexDownClose.loadFromFile("sprites/downMouthClose.png"); // loading downwards player png

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

        
    float switchTime = 0.01f;
    Clock clock;
    while (gameWindow.isOpen()) {
        Event event;
        Time elapsed;
        while (gameWindow.pollEvent(event)) { // checking for window close command
            if (event.type == Event::Closed)
                gameWindow.close();
        }

        elapsed = clock.restart();
        float deltaTime = elapsed.asSeconds();
        // taking user input
        if(Keyboard::isKeyPressed(Keyboard::A)){
            if(deltaTime >= switchTime){
                playerObj.changeTexture(playerTexLeftClose);    
                clock.restart();
            }
            else
                playerObj.changeTexture(playerTexLeft);
            playerObj.moveLeft = true;
            playerObj.moveRight = playerObj.moveUp = playerObj.moveUp = playerObj.moveDown = false;
        }
        if(Keyboard::isKeyPressed(Keyboard::W)){
            if(deltaTime >= switchTime){
                playerObj.changeTexture(playerTexUpClose);    
                clock.restart();
            }
            else
                playerObj.changeTexture(playerTexUp);
            playerObj.moveUp = true;
            playerObj.moveLeft = playerObj.moveRight = playerObj.moveDown = false;
        }
        if(Keyboard::isKeyPressed(Keyboard::S)){
            if(deltaTime >= switchTime){
                playerObj.changeTexture(playerTexDownClose);    
                clock.restart();
            }
            else
                playerObj.changeTexture(playerTexDown);
            playerObj.moveDown = true;
            playerObj.moveLeft = playerObj.moveRight = playerObj.moveUp = false;
        }
        if(Keyboard::isKeyPressed(Keyboard::D)){
            if(deltaTime >= switchTime){
                playerObj.changeTexture(playerTexRightClose);    
                clock.restart();
            }
            else
                playerObj.changeTexture(playerTexRight);
            playerObj.moveRight = true;
            playerObj.moveLeft = playerObj.moveUp = playerObj.moveDown = false;
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
    pthread_mutex_destroy(&objectMovementSynchronisor);
    return 0;
}