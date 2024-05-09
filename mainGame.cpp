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
RectangleShape intelligentGhostTarget(Vector2f(50.0f, 50.0f));
pthread_mutex_t objectMovementSynchronisor;
pthread_mutex_t pathFinder;
int PlayerX = 1;
int PlayerY = 15;
int totalThreads = 1;
int exitedThread = 0;
int currentGhostToLeave = 0;
#define SPEEDBOOST 0.2f
bool playerAndGhostHaveCollided = false;

struct Point {
    int row, col;
};

class GHOST {
private:
    Sprite sprite; // Ghost sprite
    int ID; // Ghost's turn
    int mode; // Ghost mode (e.g., scatter, chase)
    float speed; // this is ghost's speed used for speed boost
    pair<int, int> target; // this is the ghost's target having x and y co-ordinates

public:
    bool moveLeft = false; // boolean for left movement
    bool moveRight = false; // boolean for right movement
    bool moveUp = true; // boolean for up movement
    bool moveDown = false; // boolean for down movement
    vector<Point> ghostPath;
    int currentTarget = 0;

    GHOST(Texture &text, int t, int m = 0, float sp = 0) { // Constructor
        this->sprite.setTexture(text);
        this->sprite.setScale(1.0f, 1.0f);
        this->sprite.setPosition(GHOSTHOMEX * CELLSIZE + 100, GHOSTHOMEY * CELLSIZE + 100);
        this->ID = t;
        this->mode = m;
        this->speed = sp;
        target = make_pair(10, 6);
    } 
    // Getter for sprite
    Sprite& getSprite() { return sprite; }
    // Setter for sprite
    void setSprite(Sprite sprite) { this->sprite = sprite; }
    // Getter for turn
    int getID() const { return ID; }
    // Setter for turn
    void setID(int ID) { this->ID = ID; }
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
    char currentDir;
    char prevDir;

    PLAYER(Texture& texture){
        this->sprite.setTexture(texture);
        this->sprite.setScale(1.0f, 1.0f);
        this->sprite.setPosition(PLAYERPOSX * CELLSIZE + 100, PLAYERPOSY * CELLSIZE + 100);
        this->score = 0;
        this->currentDir = '-';
        this->prevDir = '-';
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

struct GHOSTARGS {
    PLAYER *player;
    GHOST *ghost;
};

void *PLAYERTHREAD(void *arg){
    PLAYER *player = (PLAYER*) arg;
    
    bool collisionDetected = false; // boolean for collision detection

    while(!threadExit){
        collisionDetected = false;
        pthread_mutex_lock(&objectMovementSynchronisor);
        if (playerAndGhostHaveCollided) {
            player->getSprite().setPosition(PLAYERPOSX * CELLSIZE + 100, PLAYERPOSY * CELLSIZE + 100);
            playerAndGhostHaveCollided = false;
        }
        FloatRect playerBounds = player->getSprite().getGlobalBounds();
        // Defining collision rectangles for each side of the player
        FloatRect leftRect(playerBounds.left - 1, playerBounds.top, 1, playerBounds.height); // getting left rect of player
        FloatRect rightRect(playerBounds.left + playerBounds.width, playerBounds.top, 1, playerBounds.height); // getting right rect of player
        FloatRect topRect(playerBounds.left, playerBounds.top - 1, playerBounds.width, 1); // getting top rect of player
        FloatRect bottomRect(playerBounds.left, playerBounds.top + playerBounds.height, playerBounds.width, 1); // getting bottom rect of player

        // checking collisions with walls
        for(int i = 0; i < gridRows; i++){
            for(int j = 0; j < gridCols; j++){
                if (maze1[i][j] == 1) { // checking if maze can be placed here or not
                    mazeBox.setPosition(j * CELLSIZE + 100, i * CELLSIZE + 100); // placing temporary mazeBox at current location
                    if (player->currentDir == 'W'){ // if player is moving rightwards and it collides with walls 
                        if (topRect.intersects(mazeBox.getGlobalBounds())) {
                            player->currentDir = player->prevDir;
                            player->prevDir = '-';
                            collisionDetected = true;
                        }
                    }
                    else if(player->currentDir == 'S'){ // if player is moving downwards and it collides with walls
                        if (bottomRect.intersects(mazeBox.getGlobalBounds())) {
                            player->currentDir = player->prevDir;
                            player->prevDir = '-';
                            collisionDetected = true;
                        }
                    }
                    else if(player->currentDir == 'A'){ // if player is moving leftwards and it collides with walls
                        if (leftRect.intersects(mazeBox.getGlobalBounds())) {
                            player->currentDir = player->prevDir;
                            player->prevDir = '-';
                            collisionDetected = true;
                        }
                    }
                    else if(player->currentDir == 'D'){ // if player is moving upwards and it collides with walls
                        if (rightRect.intersects(mazeBox.getGlobalBounds())) {
                            player->currentDir = player->prevDir;
                            player->prevDir = '-';
                            collisionDetected = true;
                        }   
                    }
                }
                //detecting player's collision with food
                else if(maze1[i][j] == 0){
                    Food.setPosition((j * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100, (i * CELLSIZE + ((CELLSIZE / 2) - 5)) + 100); //placing temporary Food at current position
                    FloatRect FoodBounds = Food.getGlobalBounds();
                    if(playerBounds.intersects(FoodBounds)){
                        maze1[i][j] = -99;
                        int score = player->getScore();
                        player->setScore(score += 1); //increasing score as the player eats food
                    }
                }
            }           
        }
        if (!collisionDetected)
            player->prevDir = '-';
        // moving the player accordingly
        if (player->currentDir == 'W') {
            player->getSprite().move(0.0f, -1.0f);
            PlayerX = (player->getSprite().getPosition().x - 100) / CELLSIZE;
            PlayerY = (player->getSprite().getPosition().y - 150) / CELLSIZE;
            PlayerY == 0 ? PlayerY = 1 : PlayerY = PlayerY;
            intelligentGhostTarget.setPosition(player->getSprite().getPosition().x, player->getSprite().getPosition().y);
        }
        else if (player->currentDir == 'A') {
            player->getSprite().move(-1.0f, 0.0f);
            PlayerX = (player->getSprite().getPosition().x - 150) / CELLSIZE;
            PlayerY = (player->getSprite().getPosition().y - 100) / CELLSIZE;
            PlayerX == 0 ? PlayerX = 1 : PlayerX = PlayerX;
            intelligentGhostTarget.setPosition(player->getSprite().getPosition().x, player->getSprite().getPosition().y);
        }
        else if (player->currentDir == 'D') {
            player->getSprite().move(1.0f, 0.0f);
            PlayerX = (player->getSprite().getPosition().x - 50) / CELLSIZE;
            PlayerY = (player->getSprite().getPosition().y - 100) / CELLSIZE;
            PlayerX == 20 ? PlayerX = 19 : PlayerX = PlayerX;
            intelligentGhostTarget.setPosition(player->getSprite().getPosition().x, player->getSprite().getPosition().y);
        }
        else if (player->currentDir == 'S') {
            player->getSprite().move(0.0f, 1.0f);
            PlayerX = (player->getSprite().getPosition().x - 100) / CELLSIZE;
            PlayerY = (player->getSprite().getPosition().y - 50) / CELLSIZE;
            PlayerY == 16 ? PlayerY = 15 : PlayerY = PlayerY;
            intelligentGhostTarget.setPosition(player->getSprite().getPosition().x, player->getSprite().getPosition().y);
        }
        pthread_mutex_unlock(&objectMovementSynchronisor);
        sleep(milliseconds(5));
    }
    exitedThread++;
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
                        int randomDirection = abs(rand() % 2); // we get random direction for our ghost
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
                        }
                    }
                    ghost->moveUp = false;
                }
            }
        }
    }
    return;
}

// Function to check if a point is valid (within the bounds of the grid)
bool isValid(int x, int y) {
    return (x >= 0 && x < gridRows && y >= 0 && y < gridCols);
}

// Function to perform BFS and return the path to the target
void GETPATHTOTARGET(GHOST *ghost, Point start, Point target) {
    vector<vector<bool>> visited(gridRows, vector<bool>(gridCols, false));
    vector<vector<Point>> parent(gridRows, vector<Point>(gridCols, { -1, -1 }));

    queue<Point> q;
    q.push(start);
    visited[start.row][start.col] = true;

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        // Check if the current point is the target
        if (curr.row == target.row && curr.col == target.col) {
            ghost->ghostPath.clear();
            while (!(curr.row == start.row && curr.col == start.col)) {
                ghost->ghostPath.push_back(curr);
                curr = parent[curr.row][curr.col];
            }
            ghost->ghostPath.push_back(start);
            reverse(ghost->ghostPath.begin(), ghost->ghostPath.end());
        }

        // Possible directions: up, down, left, right
        int dx[] = { -1, 1, 0, 0 };
        int dy[] = { 0, 0, -1, 1 };

        // Explore neighbors
        for (int i = 0; i < 4; i++) {
            int newX = curr.row + dx[i];
            int newY = curr.col + dy[i];

            if (isValid(newX, newY) && maze1[newX][newY] != 1 && !visited[newX][newY]) {
                visited[newX][newY] = true;
                parent[newX][newY] = curr;
                q.push({ newX, newY });
            }
        }
    }
    return; // No path found
}

void *GHOSTTHREAD(void *arg) { // this is the ghost thread
    GHOST *ghost = (GHOST *) arg;
    bool collisionDetected = false; // boolean for collision detection
    bool leftHome = false;
    bool foundPath = false;
    // initially only up ward movement is allowed because the ghost has to move up to come out of the home
    Clock clock;
    float elapsedTime = 0, pathFindingInterval = 300;
    clock.restart();
    while (!threadExit) { // loop iterate until threadExit becomes true
        if (currentGhostToLeave == ghost->getID() || leftHome) {
            pthread_mutex_lock(&objectMovementSynchronisor);
            if (ghost->getMode() == 0) { // this is for simple ghost
                // MOVESIMPLEGHOST(collisionDetected, leftHome, ghost);
                srand(time(0));
                if (!leftHome)
                    LEAVEHOME(collisionDetected, leftHome, ghost);
                else {
                    if (!foundPath) {
                        pthread_mutex_lock(&pathFinder);
                        vector<vector<int>> corners = {{1, 1}, {19, 1}, {1, 15}, {19, 15}};
                        int newCorner = rand() % 4;
                        ghost->setTarget(make_pair(corners[newCorner][0], corners[newCorner][1])); // setting new target for ghost
                        Point ghostPosition, ghostTarget;
                        ghostPosition.row = (ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                        ghostTarget.row = ghost->getTarget().second, ghostTarget.col = ghost->getTarget().first;
                        GETPATHTOTARGET(ghost, ghostPosition, ghostTarget);
                        foundPath = true;
                        ghost->moveDown = ghost->moveUp = ghost->moveRight = ghost->moveLeft = false;
                        pthread_mutex_unlock(&pathFinder);
                    }
                    else {
                        RectangleShape tempTarget(Vector2f(50.0f, 50.0f));
                        if (!ghost->moveDown && !ghost->moveUp && !ghost->moveRight && !ghost->moveLeft) {
                            ghost->currentTarget++;
                            int ghostPosX = (ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            int ghostPosY = (ghost->getSprite().getPosition().y - 100) / CELLSIZE;
                            if (ghostPosY < ghost->ghostPath[ghost->currentTarget].row) { // target is below
                                ghost->moveDown = true;
                                ghost->moveLeft = ghost->moveRight = ghost->moveUp = false;
                            }
                            else if (ghostPosY > ghost->ghostPath[ghost->currentTarget].row) { // target is above
                                ghost->moveUp = true;
                                ghost->moveLeft = ghost->moveRight = ghost->moveDown = false;
                            }
                            else if (ghostPosX < ghost->ghostPath[ghost->currentTarget].col) { // target is on the right side
                                ghost->moveRight = true;
                                ghost->moveDown = ghost->moveLeft = ghost->moveUp = false;
                            }
                            else if (ghostPosX > ghost->ghostPath[ghost->currentTarget].col) { // target is on the left side
                                ghost->moveLeft = true;
                                ghost->moveDown = ghost->moveRight = ghost->moveUp = false;
                            }
                        }
                        tempTarget.setPosition(ghost->ghostPath[ghost->currentTarget].col * CELLSIZE + 100, ghost->ghostPath[ghost->currentTarget].row * CELLSIZE + 100);
                        FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();
                        // Defining collision rectangles for each side of the ghost
                        FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height); // getting left rect of ghost
                        FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height); // getting right rect of ghost
                        FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1); // getting top rect of ghost
                        FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1); // getting bottom rect of ghost

                        FloatRect tempTargetBounds = tempTarget.getGlobalBounds();
                        // Defining collision rectangles for each side of the ghost
                        FloatRect leftRectTG(tempTargetBounds.left - 1, tempTargetBounds.top, 1, tempTargetBounds.height); // getting left rect of ghost
                        FloatRect rightRectTG(tempTargetBounds.left + tempTargetBounds.width, tempTargetBounds.top, 1, tempTargetBounds.height); // getting right rect of ghost
                        FloatRect topRectTG(tempTargetBounds.left, tempTargetBounds.top - 1, tempTargetBounds.width, 1); // getting top rect of ghost
                        FloatRect bottomRectTG(tempTargetBounds.left, tempTargetBounds.top + tempTargetBounds.height, tempTargetBounds.width, 1); // getting bottom rect of ghost

                        if (ghost->moveUp && topRect.intersects(topRectTG)) {
                            ghost->moveUp = false;
                        }
                        else if (ghost->moveLeft && leftRect.intersects(leftRectTG)) {
                            ghost->moveLeft = false;
                        }
                        else if (ghost->moveRight && rightRect.intersects(rightRectTG)) {
                            ghost->moveRight = false;
                        }
                        else if (ghost->moveDown && bottomRect.intersects(bottomRectTG)) {
                            ghost->moveDown = false;
                        }
                        if (ghost->currentTarget == ghost->ghostPath.size()) {
                            // cout << "Current Target length: " << ghost->currentTarget << endl << endl;
                            ghost->currentTarget = 0;
                            foundPath = false;
                            ghost->moveDown = ghost->moveUp = ghost->moveRight = ghost->moveLeft = false;
                        }
                        if (ghostBounds.intersects(intelligentGhostTarget.getGlobalBounds())) {
                            cout << "Collision detected" << endl << endl;
                            playerAndGhostHaveCollided = true;
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
            }
            else if (ghost->getMode() == 1) { // this is for a semi intelligent ghost
                srand(time(0));
                if (!leftHome)
                    LEAVEHOME(collisionDetected, leftHome, ghost);
                else {
                    if (!foundPath) {
                        pthread_mutex_lock(&pathFinder);
                        int newX, newY;
                        do {
                            newX = rand() % gridCols; // Exclude border cells (0th row and column)
                            newY = rand() % gridRows; // Exclude border cells (0th row and column)
                        } while(maze1[newY][newX] == 1 || newY == 0 || newX == 0); // Check if the new position is valid
                        ghost->setTarget(make_pair(newX, newY)); // setting new target for ghost
                        Point ghostPosition, ghostTarget;
                        ghostPosition.row = (ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                        ghostTarget.row = ghost->getTarget().second, ghostTarget.col = ghost->getTarget().first;
                        GETPATHTOTARGET(ghost, ghostPosition, ghostTarget);
                        foundPath = true;
                        ghost->moveDown = ghost->moveUp = ghost->moveRight = ghost->moveLeft = false;
                        pthread_mutex_unlock(&pathFinder);
                    }
                    else {
                        RectangleShape tempTarget(Vector2f(50.0f, 50.0f));
                        if (!ghost->moveDown && !ghost->moveUp && !ghost->moveRight && !ghost->moveLeft) {
                            ghost->currentTarget++;
                            int ghostPosX = (ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            int ghostPosY = (ghost->getSprite().getPosition().y - 100) / CELLSIZE;
                            if (ghostPosY < ghost->ghostPath[ghost->currentTarget].row) { // target is below
                                ghost->moveDown = true;
                                ghost->moveLeft = ghost->moveRight = ghost->moveUp = false;
                            }
                            else if (ghostPosY > ghost->ghostPath[ghost->currentTarget].row) { // target is above
                                ghost->moveUp = true;
                                ghost->moveLeft = ghost->moveRight = ghost->moveDown = false;
                            }
                            else if (ghostPosX < ghost->ghostPath[ghost->currentTarget].col) { // target is on the right side
                                ghost->moveRight = true;
                                ghost->moveDown = ghost->moveLeft = ghost->moveUp = false;
                            }
                            else if (ghostPosX > ghost->ghostPath[ghost->currentTarget].col) { // target is on the left side
                                ghost->moveLeft = true;
                                ghost->moveDown = ghost->moveRight = ghost->moveUp = false;
                            }
                        }
                        tempTarget.setPosition(ghost->ghostPath[ghost->currentTarget].col * CELLSIZE + 100, ghost->ghostPath[ghost->currentTarget].row * CELLSIZE + 100);
                        FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();
                        // Defining collision rectangles for each side of the ghost
                        FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height); // getting left rect of ghost
                        FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height); // getting right rect of ghost
                        FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1); // getting top rect of ghost
                        FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1); // getting bottom rect of ghost

                        FloatRect tempTargetBounds = tempTarget.getGlobalBounds();
                        // Defining collision rectangles for each side of the ghost
                        FloatRect leftRectTG(tempTargetBounds.left - 1, tempTargetBounds.top, 1, tempTargetBounds.height); // getting left rect of ghost
                        FloatRect rightRectTG(tempTargetBounds.left + tempTargetBounds.width, tempTargetBounds.top, 1, tempTargetBounds.height); // getting right rect of ghost
                        FloatRect topRectTG(tempTargetBounds.left, tempTargetBounds.top - 1, tempTargetBounds.width, 1); // getting top rect of ghost
                        FloatRect bottomRectTG(tempTargetBounds.left, tempTargetBounds.top + tempTargetBounds.height, tempTargetBounds.width, 1); // getting bottom rect of ghost

                        if (ghost->moveUp && topRect.intersects(topRectTG)) {
                            ghost->moveUp = false;
                        }
                        else if (ghost->moveLeft && leftRect.intersects(leftRectTG)) {
                            ghost->moveLeft = false;
                        }
                        else if (ghost->moveRight && rightRect.intersects(rightRectTG)) {
                            ghost->moveRight = false;
                        }
                        else if (ghost->moveDown && bottomRect.intersects(bottomRectTG)) {
                            ghost->moveDown = false;
                        }
                        if (ghost->currentTarget == ghost->ghostPath.size()) {
                            // cout << "Current Target length: " << ghost->currentTarget << endl << endl;
                            ghost->currentTarget = 0;
                            foundPath = false;
                            ghost->moveDown = ghost->moveUp = ghost->moveRight = ghost->moveLeft = false;
                        }
                        if (ghostBounds.intersects(intelligentGhostTarget.getGlobalBounds())) {
                            cout << "Collision detected" << endl << endl;
                            playerAndGhostHaveCollided = true;
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
            }
            else if (ghost->getMode() == 2) { // this is for a fully intelligent ghost
                elapsedTime += clock.getElapsedTime().asSeconds();
                srand(time(0));
                if (!leftHome)
                    LEAVEHOME(collisionDetected, leftHome, ghost);
                else {
                    if (!foundPath) {
                        pthread_mutex_lock(&pathFinder);
                        ghost->setTarget(make_pair(PlayerX, PlayerY)); // setting new target for ghost
                        Point ghostPosition, ghostTarget;
                        ghostPosition.row = (ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                        ghostTarget.row = ghost->getTarget().second, ghostTarget.col = ghost->getTarget().first;
                        GETPATHTOTARGET(ghost, ghostPosition, ghostTarget);
                        foundPath = true;
                        ghost->moveDown = ghost->moveUp = ghost->moveRight = ghost->moveLeft = false;
                        pthread_mutex_unlock(&pathFinder);
                    }
                    else {
                        RectangleShape tempTarget(Vector2f(50.0f, 50.0f));
                        if (!ghost->moveDown && !ghost->moveUp && !ghost->moveRight && !ghost->moveLeft) {
                            ghost->currentTarget++;
                            int ghostPosX = (ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            int ghostPosY = (ghost->getSprite().getPosition().y - 100) / CELLSIZE;
                            if (ghostPosY < ghost->ghostPath[ghost->currentTarget].row) { // target is below
                                ghost->moveDown = true;
                                ghost->moveLeft = ghost->moveRight = ghost->moveUp = false;
                            }
                            else if (ghostPosY > ghost->ghostPath[ghost->currentTarget].row) { // target is above
                                ghost->moveUp = true;
                                ghost->moveLeft = ghost->moveRight = ghost->moveDown = false;
                            }
                            else if (ghostPosX < ghost->ghostPath[ghost->currentTarget].col) { // target is on the right side
                                ghost->moveRight = true;
                                ghost->moveDown = ghost->moveLeft = ghost->moveUp = false;
                            }
                            else if (ghostPosX > ghost->ghostPath[ghost->currentTarget].col) { // target is on the left side
                                ghost->moveLeft = true;
                                ghost->moveDown = ghost->moveRight = ghost->moveUp = false;
                            }
                        }
                        tempTarget.setPosition(ghost->ghostPath[ghost->currentTarget].col * CELLSIZE + 100, ghost->ghostPath[ghost->currentTarget].row * CELLSIZE + 100);
                        FloatRect ghostBounds = ghost->getSprite().getGlobalBounds();
                        // Defining collision rectangles for each side of the ghost
                        FloatRect leftRect(ghostBounds.left - 1, ghostBounds.top, 1, ghostBounds.height); // getting left rect of ghost
                        FloatRect rightRect(ghostBounds.left + ghostBounds.width, ghostBounds.top, 1, ghostBounds.height); // getting right rect of ghost
                        FloatRect topRect(ghostBounds.left, ghostBounds.top - 1, ghostBounds.width, 1); // getting top rect of ghost
                        FloatRect bottomRect(ghostBounds.left, ghostBounds.top + ghostBounds.height, ghostBounds.width, 1); // getting bottom rect of ghost

                        FloatRect tempTargetBounds = tempTarget.getGlobalBounds();
                        // Defining collision rectangles for each side of the ghost
                        FloatRect leftRectTG(tempTargetBounds.left - 1, tempTargetBounds.top, 1, tempTargetBounds.height); // getting left rect of ghost
                        FloatRect rightRectTG(tempTargetBounds.left + tempTargetBounds.width, tempTargetBounds.top, 1, tempTargetBounds.height); // getting right rect of ghost
                        FloatRect topRectTG(tempTargetBounds.left, tempTargetBounds.top - 1, tempTargetBounds.width, 1); // getting top rect of ghost
                        FloatRect bottomRectTG(tempTargetBounds.left, tempTargetBounds.top + tempTargetBounds.height, tempTargetBounds.width, 1); // getting bottom rect of ghost

                        if (ghost->moveUp && topRect.intersects(topRectTG)) {
                            ghost->moveUp = false;
                        }
                        else if (ghost->moveLeft && leftRect.intersects(leftRectTG)) {
                            ghost->moveLeft = false;
                        }
                        else if (ghost->moveRight && rightRect.intersects(rightRectTG)) {
                            ghost->moveRight = false;
                        }
                        else if (ghost->moveDown && bottomRect.intersects(bottomRectTG)) {
                            ghost->moveDown = false;
                        }
                        if (elapsedTime >= pathFindingInterval || ghost->currentTarget == ghost->ghostPath.size()) {
                            ghost->currentTarget = 0;
                            foundPath = false;
                            elapsedTime = 0;
                            clock.restart();
                            ghost->moveDown = ghost->moveUp = ghost->moveRight = ghost->moveLeft = false;
                        }
                        if (ghostBounds.intersects(intelligentGhostTarget.getGlobalBounds())) {
                            cout << "Collision detected" << endl << endl;
                            playerAndGhostHaveCollided = true;
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
            }
            pthread_mutex_unlock(&objectMovementSynchronisor);
            sleep(milliseconds(5));
        }
    }
    exitedThread++;
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

    Texture ghostTextures[5];
    ghostTextures[0].loadFromFile("sprites/redGhost.png");
    ghostTextures[1].loadFromFile("sprites/greenGhost.png");
    ghostTextures[2].loadFromFile("sprites/yellowGhost.png");
    ghostTextures[3].loadFromFile("sprites/pinkGhost.png");
    ghostTextures[4].loadFromFile("sprites/blueGhost.png");

    int initialTotalGhost = 3;

    GHOST *ghosts[initialTotalGhost];
    for (int i = 0; i < initialTotalGhost; i++) {
        ghosts[i] = new GHOST(ghostTextures[i], i, i);
    }

    playerTexLeft.loadFromFile("sprites/mouthOpenLeft.png"); // loading left side player png
    playerTexRight.loadFromFile("sprites/mouthOpenRight.png"); // loading default player png
    playerTexUp.loadFromFile("sprites/mouthOpenUp.png"); // loading upwards player png
    playerTexDown.loadFromFile("sprites/mouthOpenDown.png"); // loading downwards player png

    playerTexLeftClose.loadFromFile("sprites/leftMouthClose.png"); // loading left side player png
    playerTexRightClose.loadFromFile("sprites/rightMouthClose.png"); // loading default player png
    playerTexUpClose.loadFromFile("sprites/upMouthClose.png"); // loading upwards player png
    playerTexDownClose.loadFromFile("sprites/downMouthClose.png"); // loading downwards player png

    PLAYER playerObj(playerTexRight); // creating player obj,mkmlkjj

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
    pthread_t ghostThread[initialTotalGhost]; 
    for (int i = 0; i < initialTotalGhost; i++) {
        pthread_create(&ghostThread[i], &detachProp, GHOSTTHREAD, ghosts[i]); // creating a detachable ghost thread
        totalThreads++;
    }

    pthread_t playerThread; 
    pthread_create(&playerThread, &detachProp, PLAYERTHREAD, (void **) &playerObj); // creating a detachable player thread
    totalThreads++;
    pthread_attr_destroy(&detachProp);

    bool mouthOpened = true;
    Clock clock;
    float changeMouthTimer = 0.2f;
    clock.restart();
    while (gameWindow.isOpen()) {
        Event event;
    
        while (gameWindow.pollEvent(event)) { // checking for window close command
            if (event.type == Event::Closed)
                gameWindow.close();
        }
        // taking user input
        pthread_mutex_lock(&objectMovementSynchronisor);
        if(Keyboard::isKeyPressed(Keyboard::W) && playerObj.currentDir != 'W') {
            playerObj.prevDir = playerObj.currentDir;
            playerObj.currentDir = 'W';
        }
        else if(Keyboard::isKeyPressed(Keyboard::S) && playerObj.currentDir != 'S') {
            playerObj.prevDir = playerObj.currentDir;
            playerObj.currentDir = 'S';
        }
        else if(Keyboard::isKeyPressed(Keyboard::A) && playerObj.currentDir != 'A') {
            playerObj.prevDir = playerObj.currentDir;
            playerObj.currentDir = 'A';
        }
        else if(Keyboard::isKeyPressed(Keyboard::D) && playerObj.currentDir != 'D') {
            playerObj.prevDir = playerObj.currentDir;
            playerObj.currentDir = 'D';
        }
        pthread_mutex_unlock(&objectMovementSynchronisor);
        if(clock.getElapsedTime().asSeconds() >= changeMouthTimer && mouthOpened){
            if(playerObj.currentDir == 'W'){
                playerObj.changeTexture(playerTexUpClose);   
            }
            else if(playerObj.currentDir == 'S'){
                playerObj.changeTexture(playerTexDownClose);   
            }
            else if(playerObj.currentDir == 'A'){
                playerObj.changeTexture(playerTexLeftClose);    
            }
            else if(playerObj.currentDir == 'D'){
                playerObj.changeTexture(playerTexRightClose);    
            }
            mouthOpened = false;
            clock.restart();
        }
        else if(clock.getElapsedTime().asSeconds() >= changeMouthTimer && !mouthOpened){
            if(playerObj.currentDir == 'W'){
                playerObj.changeTexture(playerTexUp);
            }
            else if(playerObj.currentDir == 'S'){
                playerObj.changeTexture(playerTexDown);
            }
            else if(playerObj.currentDir == 'A'){
                playerObj.changeTexture(playerTexLeft);
            }
            else if(playerObj.currentDir == 'D'){
                playerObj.changeTexture(playerTexRight);
            }
            mouthOpened = true;
            clock.restart();
        }
        
        gameWindow.clear(); // clearing the buffer window
        DRAWMAZE(gameWindow, Food, mazeBox); // Drawing the maze with food and mazeBoxes
        for (int i = 0; i < initialTotalGhost; i++)
            gameWindow.draw(ghosts[i]->getSprite());
        gameWindow.draw(playerObj.getSprite());
        gameWindow.draw(displayScoreString);
        displayScore.setString(to_string(playerObj.getScore())); //converting score to string so that it can be displayed
        gameWindow.draw(displayScore);        
        gameWindow.display(); // swapping the buffer window with main window
    }

    threadExit = true;
    exitedThread++;
    pthread_exit(NULL);
}

int main()
{
    pthread_t startGame;
    pthread_attr_t detachProp; // setting detachable property
    pthread_attr_init(&detachProp); // initializing that property
    pthread_attr_setdetachstate(&detachProp, PTHREAD_CREATE_DETACHED); // making it detachable
    pthread_create(&startGame, &detachProp, GAMEINIT, NULL); // initiating main game thread
    pthread_attr_destroy(&detachProp);
    while (true) {
        if (threadExit && exitedThread == totalThreads)
            break;
    }
    pthread_mutex_destroy(&objectMovementSynchronisor);
    pthread_mutex_destroy(&pathFinder);
    return 0;
}