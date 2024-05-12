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
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 1 is for wall, 0 if for food and path
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 2 is for path only, and 3 is for speedboost
    {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1}, // 4 is for powerUp
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {2, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 2},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};
bool threadExit = false; // this is a boolean to close all detached threads when the game closes
Sprite mazeBox; // creating a sprite for Game Grid
Texture box; // creating a texture for maze.png
Sprite boost;
Sprite power;
Sprite pointer;
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

Texture blueGhostTex;

// this is our circular food
CircleShape Food(5.0f); 
RectangleShape intelligentGhostTarget(Vector2f(50.0f, 50.0f));
pthread_mutex_t objectMovementSynchronisor;
pthread_mutex_t pathFinder;
pthread_mutex_t pathIlluminator;
pthread_mutex_t ghostMover;
int PlayerX = 1;
int PlayerY = 15;
int totalThreads = 1;
int exitedThread = 0;
int currentGhostToLeave = 0;
int powerPelletsCount = 0;
int speedBoostersCount = 0;
#define SPEEDBOOST 0.2f
bool ghostAtePacman = false;
bool playerGotPowerUp = false;
bool menuIsBeingDisplayed = true;
bool isMenu = true, isGamePlay = false, isGameOver = false;
Clock clockForPP, clockForSB;
float elapsedTimeForSB = 0,  elapsedTimeForPP = 0, displayPPAndSBInterval = 5000;
bool moveCursorUp = false, moveCursorDown = false, keyPressed = false, pressedEnter = false;

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
    Texture defaultText;
    bool moveLeft = false; // boolean for left movement
    bool moveRight = false; // boolean for right movement
    bool moveUp = false; // boolean for up movement
    bool moveDown = false; // boolean for down movement
    vector<Point> ghostPath;
    int currentTarget = 0;
    bool ghostCanMove = true;
    bool ghostHasSpeedBoost = false;
    Clock ghostClock;
    float ghostElapsedTime = 0;

    GHOST(Texture &text, int t, int m = 0, float sp = 0) { // Constructor
        this->sprite.setTexture(text);
        this->sprite.setScale(1.0f, 1.0f);
        this->sprite.setPosition(GHOSTHOMEX * CELLSIZE + 100, GHOSTHOMEY * CELLSIZE + 100);
        this->ID = t;
        this->mode = m;
        this->speed = sp;
        target = make_pair(10, 6);
        this->defaultText = text;
    } 
    void changeTexture(Texture& tex){
        this->sprite.setTexture(tex);
        this->sprite.setScale(1.0f, 1.0f);
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

struct MENU {
    Text heading;
    Text option[3];
    Font font[2];
    vector<vector<int>> Position = {{200, -80}, {450, 250}, {450, 350}, {450, 450}};
    MENU() {
        //setting the fonts
        font[0].loadFromFile("fonts/heading.otf");       
        font[1].loadFromFile("fonts/normal.ttf");       
        //setting the heading
        heading.setFont(this->font[0]);
        heading.setString("PACMAN FURY");
        heading.setFillColor(Color::White);
        heading.setCharacterSize(150);
        heading.setPosition(Position[0][0], Position[0][1]);
        //setting up option 1
        option[0].setFont(this->font[1]);
        option[0].setString("START GAME");
        option[0].setFillColor(Color::White);
        option[0].setCharacterSize(30);
        option[0].setPosition(Position[1][0], Position[1][1]);
        //setting up option 2
        option[1].setFont(this->font[1]);
        option[1].setString("INSTRUCTIONS");
        option[1].setFillColor(Color::White);
        option[1].setCharacterSize(30);
        option[1].setPosition(Position[2][0], Position[2][1]);
        //setting up option 3
        option[2].setFont(this->font[1]);
        option[2].setString("EXIT GAME");
        option[2].setFillColor(Color::White);
        option[2].setCharacterSize(30);
        option[2].setPosition(Position[3][0], Position[3][1]);
    }
};

struct POWERPELLETLOCATIONS {
    int x, y;
    bool isDisplayed = false;
};

struct SPEEDBOOSTERLOCATIONS {
    int x, y;
    bool isDisplayed = false;
};

struct PLAYERARGS {
    PLAYER *player;
    vector<POWERPELLETLOCATIONS*> PPL;
    MENU *menu;
};

struct GHOSTARGS {
    GHOST *ghost;
    vector<SPEEDBOOSTERLOCATIONS*> SBL;
};

void *PLAYERTHREAD(void *arg){
    PLAYERARGS *playerArgs = (PLAYERARGS*) arg;

    bool collisionDetected = false; // boolean for collision detection

    float elapsedTime = 0, powerUpTime = 3000;
    Clock clock;
    clockForPP.restart();
    clockForSB.restart();
    while(!threadExit){
        collisionDetected = false;
        pthread_mutex_lock(&objectMovementSynchronisor);
        if (isMenu) {
            if (moveCursorUp) {
                cout << "Cursor should move up" << endl;
                moveCursorUp = false;
                if (pointer.getPosition().y > 250)
                    pointer.setPosition(800, pointer.getPosition().y - 100);
            }
            else if (moveCursorDown) {
                cout << "Cursor should move down" << endl;
                moveCursorDown = false;
                if (pointer.getPosition().y < 450)
                    pointer.setPosition(800, pointer.getPosition().y + 100);
            }
            else if (pressedEnter) {
                if (pointer.getPosition().y == 250) {
                    isMenu = false;
                    isGamePlay = true;
                    clockForPP.restart();
                    clockForSB.restart();
                }
                pressedEnter = false;
            }
        }
        else if (isGamePlay) {
            if (ghostAtePacman && !playerGotPowerUp) {
                playerArgs->player->getSprite().setPosition(PLAYERPOSX * CELLSIZE + 100, PLAYERPOSY * CELLSIZE + 100);
                ghostAtePacman = false;
            }

            if (playerArgs->player->getSprite().getPosition().x == -50) // if player moves out from left portal
                playerArgs->player->getSprite().setPosition(GRIDWIDTH + 200, 500);
            else if (playerArgs->player->getSprite().getPosition().x == GRIDWIDTH + 200) // if player moves out from right portal
                playerArgs->player->getSprite().setPosition(-50, 500);

            if (playerGotPowerUp) {
                elapsedTime += clock.getElapsedTime().asSeconds();
                if (elapsedTime >= powerUpTime) {
                    elapsedTime = 0;
                    playerGotPowerUp = false;
                }
            }

            FloatRect playerBounds = playerArgs->player->getSprite().getGlobalBounds();
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
                        if (playerArgs->player->currentDir == 'W'){ // if player is moving rightwards and it collides with walls 
                            if (topRect.intersects(mazeBox.getGlobalBounds())) {
                                playerArgs->player->currentDir = playerArgs->player->prevDir;
                                playerArgs->player->prevDir = '-';
                                collisionDetected = true;
                            }
                        }
                        else if(playerArgs->player->currentDir == 'S'){ // if player is moving downwards and it collides with walls
                            if (bottomRect.intersects(mazeBox.getGlobalBounds())) {
                                playerArgs->player->currentDir = playerArgs->player->prevDir;
                                playerArgs->player->prevDir = '-';
                                collisionDetected = true;
                            }
                        }
                        else if(playerArgs->player->currentDir == 'A'){ // if player is moving leftwards and it collides with walls
                            if (leftRect.intersects(mazeBox.getGlobalBounds())) {
                                playerArgs->player->currentDir = playerArgs->player->prevDir;
                                playerArgs->player->prevDir = '-';
                                collisionDetected = true;
                            }
                        }
                        else if(playerArgs->player->currentDir == 'D'){ // if player is moving upwards and it collides with walls
                            if (rightRect.intersects(mazeBox.getGlobalBounds())) {
                                playerArgs->player->currentDir = playerArgs->player->prevDir;
                                playerArgs->player->prevDir = '-';
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
                            int score = playerArgs->player->getScore();
                            playerArgs->player->setScore(score += 1); //increasing score as the player eats food
                        }
                    }
                    // detecting player's collision with power up
                    else if(!playerGotPowerUp && maze1[i][j] == 4){
                        power.setPosition((j * CELLSIZE) + 100, (i * CELLSIZE) + 100); //placing temporary Food at current position
                        FloatRect FoodBounds = power.getGlobalBounds();
                        if(playerBounds.intersects(FoodBounds)){
                            maze1[i][j] = -99;
                            playerGotPowerUp = true;
                            for (int k = 0; k < 4; k++)
                                if (playerArgs->PPL[k]->isDisplayed && playerArgs->PPL[k]->x == j && playerArgs->PPL[k]->y == i) {
                                    playerArgs->PPL[k]->isDisplayed = false;
                                    powerPelletsCount--;
                                    elapsedTimeForPP = 0;
                                    clockForPP.restart();
                                    cout << "Player Ate PPL at X: " << playerArgs->PPL[k]->x << " Y: " << playerArgs->PPL[k]->y << endl;
                                    break;
                                }
                            clock.restart();
                        }
                    }
                }           
            }
            // checking collisions with portals        
            if (playerArgs->player->getSprite().getPosition().x <= 100) { // collision with left portal
                if (playerArgs->player->currentDir == 'W' || playerArgs->player->currentDir == 'S') {
                    collisionDetected = true;
                    playerArgs->player->currentDir = playerArgs->player->prevDir;
                    playerArgs->player->prevDir = '-';
                }
            }
            else if (playerArgs->player->getSprite().getPosition().x >= 1100) { // collision with left portal
                if (playerArgs->player->currentDir == 'W' || playerArgs->player->currentDir == 'S') {
                    collisionDetected = true;
                    playerArgs->player->currentDir = playerArgs->player->prevDir;
                    playerArgs->player->prevDir = '-';
                }
            }

            if (!collisionDetected)
                playerArgs->player->prevDir = '-';
            // moving the player accordingly
            if (playerArgs->player->currentDir == 'W') {
                playerArgs->player->getSprite().move(0.0f, -1.0f);
                PlayerX = (playerArgs->player->getSprite().getPosition().x - 100) / CELLSIZE;
                PlayerY = (playerArgs->player->getSprite().getPosition().y - 150) / CELLSIZE;
                PlayerY == 0 ? PlayerY = 1 : PlayerY = PlayerY;
                intelligentGhostTarget.setPosition(playerArgs->player->getSprite().getPosition().x, playerArgs->player->getSprite().getPosition().y);
            }
            else if (playerArgs->player->currentDir == 'A') {
                playerArgs->player->getSprite().move(-1.0f, 0.0f);
                PlayerX = (playerArgs->player->getSprite().getPosition().x - 150) / CELLSIZE;
                PlayerY = (playerArgs->player->getSprite().getPosition().y - 100) / CELLSIZE;
                PlayerX == 0 ? PlayerX = 1 : PlayerX = PlayerX;
                intelligentGhostTarget.setPosition(playerArgs->player->getSprite().getPosition().x, playerArgs->player->getSprite().getPosition().y);
            }
            else if (playerArgs->player->currentDir == 'D') {
                playerArgs->player->getSprite().move(1.0f, 0.0f);
                PlayerX = (playerArgs->player->getSprite().getPosition().x - 50) / CELLSIZE;
                PlayerY = (playerArgs->player->getSprite().getPosition().y - 100) / CELLSIZE;
                PlayerX == 20 ? PlayerX = 19 : PlayerX = PlayerX;
                intelligentGhostTarget.setPosition(playerArgs->player->getSprite().getPosition().x, playerArgs->player->getSprite().getPosition().y);
            }
            else if (playerArgs->player->currentDir == 'S') {
                playerArgs->player->getSprite().move(0.0f, 1.0f);
                PlayerX = (playerArgs->player->getSprite().getPosition().x - 100) / CELLSIZE;
                PlayerY = (playerArgs->player->getSprite().getPosition().y - 50) / CELLSIZE;
                PlayerY == 16 ? PlayerY = 15 : PlayerY = PlayerY;
                intelligentGhostTarget.setPosition(playerArgs->player->getSprite().getPosition().x, playerArgs->player->getSprite().getPosition().y);
            }
        }
        pthread_mutex_unlock(&objectMovementSynchronisor);
        sleep(milliseconds(5));
    }
    exitedThread++;
    pthread_exit(NULL);    
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
}

void ILLUMINATETHEPATHTOTARGET (GHOSTARGS *ghostArgs, bool &foundPath, bool &leftHome, bool &collisionDetected) {
    int ghostPosX = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
    int ghostPosY = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE;
    RectangleShape tempTarget(Vector2f(50.0f, 50.0f));
    if (ghostArgs->ghost->ghostCanMove == false) {
        ghostArgs->ghost->currentTarget++;
        if (ghostPosY < ghostArgs->ghost->ghostPath[ghostArgs->ghost->currentTarget].row) { // target is below
            ghostArgs->ghost->moveDown = true;
            ghostArgs->ghost->moveLeft = ghostArgs->ghost->moveRight = ghostArgs->ghost->moveUp = false;
        }
        else if (ghostPosY > ghostArgs->ghost->ghostPath[ghostArgs->ghost->currentTarget].row) { // target is above
            ghostArgs->ghost->moveUp = true;
            ghostArgs->ghost->moveLeft = ghostArgs->ghost->moveRight = ghostArgs->ghost->moveDown = false;
        }
        else if (ghostPosX < ghostArgs->ghost->ghostPath[ghostArgs->ghost->currentTarget].col) { // target is on the right side
            ghostArgs->ghost->moveRight = true;
            ghostArgs->ghost->moveDown = ghostArgs->ghost->moveLeft = ghostArgs->ghost->moveUp = false;
        }
        else if (ghostPosX > ghostArgs->ghost->ghostPath[ghostArgs->ghost->currentTarget].col) { // target is on the left side
            ghostArgs->ghost->moveLeft = true;
            ghostArgs->ghost->moveDown = ghostArgs->ghost->moveRight = ghostArgs->ghost->moveUp = false;
        }
        ghostArgs->ghost->ghostCanMove = true;
    }
    tempTarget.setPosition(ghostArgs->ghost->ghostPath[ghostArgs->ghost->currentTarget].col * CELLSIZE + 100, ghostArgs->ghost->ghostPath[ghostArgs->ghost->currentTarget].row * CELLSIZE + 100);
    FloatRect ghostBounds = ghostArgs->ghost->getSprite().getGlobalBounds();
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

    if (ghostArgs->ghost->moveUp && topRect.top <= topRectTG.top) {
        ghostArgs->ghost->ghostCanMove = false;
    }
    else if (ghostArgs->ghost->moveLeft && leftRect.left <= leftRectTG.left) {
        ghostArgs->ghost->ghostCanMove = false;
    }
    else if (ghostArgs->ghost->moveRight && (rightRect.left + rightRect.width) >= (rightRectTG.left + rightRectTG.width)) {
        ghostArgs->ghost->ghostCanMove = false;
    }
    else if (ghostArgs->ghost->moveDown && (bottomRect.top + bottomRect.height) >= (bottomRectTG.top + bottomRectTG.height)) {
        ghostArgs->ghost->ghostCanMove = false;
    }
    if (ghostArgs->ghost->currentTarget == ghostArgs->ghost->ghostPath.size()) {
        ghostArgs->ghost->currentTarget = 0;
        foundPath = false;
        if (!leftHome) {
            if (ghostArgs->ghost->getID() >= currentGhostToLeave)
                currentGhostToLeave++;
            leftHome = true;
        }
        ghostArgs->ghost->ghostCanMove = false;
    }
    if (!playerGotPowerUp && ghostBounds.intersects(intelligentGhostTarget.getGlobalBounds())) {
        ghostAtePacman = true;
    }
    else if (playerGotPowerUp && ghostBounds.intersects(intelligentGhostTarget.getGlobalBounds())) {
        ghostArgs->ghost->currentTarget = 0;
        leftHome = false;
        foundPath = false;
        collisionDetected = false;
        ghostArgs->ghost->getSprite().setPosition(GHOSTHOMEX * CELLSIZE + 100, GHOSTHOMEY * CELLSIZE + 100);
    }
    for (int i = 0; i < 2; i++) {
        if (!ghostArgs->ghost->ghostHasSpeedBoost && ghostArgs->SBL[i]->isDisplayed && ghostPosX == ghostArgs->SBL[i]->x && ghostPosY == ghostArgs->SBL[i]->y) {
            if (maze1[ghostPosY][ghostPosX] == 3) {
                maze1[ghostPosY][ghostPosX] = -99;
                cout << "Ghost Ate Speed Booster at X: " << ghostPosX << " Y: " << ghostPosY << endl;
                ghostArgs->ghost->ghostHasSpeedBoost = true;
                ghostArgs->ghost->ghostClock.restart();
                ghostArgs->ghost->ghostElapsedTime = 0;
                ghostArgs->ghost->setSpeed(SPEEDBOOST);
                ghostArgs->SBL[i]->isDisplayed = false;
                clockForSB.restart();
                elapsedTimeForSB = 0;
                speedBoostersCount--;
            }
        }
    }
}

void MOVETHEGHOST (GHOSTARGS *ghostArgs) {
    // Moving the ghost according to the position available
    if (ghostArgs->ghost->ghostCanMove) {
        if (ghostArgs->ghost->moveUp)
            ghostArgs->ghost->getSprite().move(0.0f, -1.0f - ghostArgs->ghost->getSpeed());
        else if (ghostArgs->ghost->moveLeft)
            ghostArgs->ghost->getSprite().move(-1.0f - ghostArgs->ghost->getSpeed(), 0.0f);
        else if (ghostArgs->ghost->moveRight)
            ghostArgs->ghost->getSprite().move(1.0f + ghostArgs->ghost->getSpeed(), 0.0f);
        else if (ghostArgs->ghost->moveDown)
            ghostArgs->ghost->getSprite().move(0.0f, 1.0f + ghostArgs->ghost->getSpeed());
    }
}

void *GHOSTTHREAD(void *arg) { // this is the ghost thread
    GHOSTARGS *ghostArgs = (GHOSTARGS *) arg;
    bool collisionDetected = false; // boolean for collision detection
    bool leftHome = false;
    bool foundPath = false;
    // initially only up ward movement is allowed because the ghost has to move up to come out of the home
    Clock clock;
    float elapsedTime = 0, pathFindingInterval = 400, speedBoostInterval = 3000;
    clock.restart();
    while (!threadExit) { // loop iterate until threadExit becomes true
        if (currentGhostToLeave >= ghostArgs->ghost->getID() || leftHome) {
            pthread_mutex_lock(&objectMovementSynchronisor);
            if (isGamePlay) {
                if (playerGotPowerUp) {
                    ghostArgs->ghost->changeTexture(blueGhostTex);
                }
                else {
                    ghostArgs->ghost->changeTexture(ghostArgs->ghost->defaultText);
                }
                if (ghostArgs->ghost->getMode() == 0) { // this is for simple ghost
                    srand(time(0) ^ pthread_self());
                    if (!leftHome) {
                        if (!collisionDetected) {
                            Point ghostPosition, ghostTarget;
                            ghostPosition.row = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            ghostTarget.row = 6, ghostTarget.col = 10;
                            GETPATHTOTARGET(ghostArgs->ghost, ghostPosition, ghostTarget);
                            collisionDetected = true;
                            ghostArgs->ghost->ghostCanMove = false;
                        }
                        ILLUMINATETHEPATHTOTARGET(ghostArgs, foundPath, leftHome, collisionDetected);
                    }
                    else {
                        if (!foundPath) {
                            vector<vector<int>> corners = {{1, 1}, {19, 1}, {1, 15}, {19, 15}};
                            int newCorner = rand() % 4;
                            Point ghostPosition, ghostTarget;
                            ghostPosition.row = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            ghostTarget.row = corners[newCorner][1], ghostTarget.col = corners[newCorner][0];
                            GETPATHTOTARGET(ghostArgs->ghost, ghostPosition, ghostTarget);
                            foundPath = true;
                            ghostArgs->ghost->ghostCanMove = false;
                        }
                        ILLUMINATETHEPATHTOTARGET(ghostArgs, foundPath, leftHome, collisionDetected);
                    }
                    if (ghostArgs->ghost->ghostHasSpeedBoost) {
                        ghostArgs->ghost->ghostElapsedTime += ghostArgs->ghost->ghostClock.getElapsedTime().asSeconds();
                        if (ghostArgs->ghost->ghostElapsedTime >= speedBoostInterval) {
                            ghostArgs->ghost->setSpeed(0);
                            ghostArgs->ghost->ghostHasSpeedBoost = false;
                        }
                    }
                    MOVETHEGHOST(ghostArgs);
                }
                else if (ghostArgs->ghost->getMode() == 1) { // this is for a semi intelligent ghost
                    srand(time(0) ^ pthread_self());
                    if (!leftHome) {
                        if (!collisionDetected) {
                            Point ghostPosition, ghostTarget;
                            ghostPosition.row = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            ghostTarget.row = 6, ghostTarget.col = 10;
                            GETPATHTOTARGET(ghostArgs->ghost, ghostPosition, ghostTarget);
                            collisionDetected = true;
                            ghostArgs->ghost->ghostCanMove = false;
                        }
                        ILLUMINATETHEPATHTOTARGET(ghostArgs, foundPath, leftHome, collisionDetected);
                    }
                    else {
                        if (!foundPath) {
                            srand(time(0));
                            int newX, newY;
                            do {
                                newX = rand() % gridCols; // Exclude border cells (0th row and column)
                                newY = rand() % gridRows; // Exclude border cells (0th row and column)
                            } while(maze1[newY][newX] == 1 || newY == 0 || newX == 0); // Check if the new position is valid
                            Point ghostPosition, ghostTarget;
                            ghostPosition.row = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                            ghostTarget.row = newY, ghostTarget.col = newX;
                            GETPATHTOTARGET(ghostArgs->ghost, ghostPosition, ghostTarget);
                            foundPath = true;
                            ghostArgs->ghost->ghostCanMove = false;
                        }
                        ILLUMINATETHEPATHTOTARGET(ghostArgs, foundPath, leftHome, collisionDetected);
                    }
                    if (ghostArgs->ghost->ghostHasSpeedBoost) {
                        ghostArgs->ghost->ghostElapsedTime += ghostArgs->ghost->ghostClock.getElapsedTime().asSeconds();
                        if (ghostArgs->ghost->ghostElapsedTime >= speedBoostInterval) {
                            ghostArgs->ghost->setSpeed(0);
                            ghostArgs->ghost->ghostHasSpeedBoost = false;
                        }
                    }
                    MOVETHEGHOST(ghostArgs);
                }
                else if (ghostArgs->ghost->getMode() == 2) { // this is for a fully intelligent ghost
                elapsedTime += clock.getElapsedTime().asSeconds();
                if (!leftHome) {
                    if (!collisionDetected) {
                        Point ghostPosition, ghostTarget;
                        ghostPosition.row = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                        ghostTarget.row = 6, ghostTarget.col = 10;
                        GETPATHTOTARGET(ghostArgs->ghost, ghostPosition, ghostTarget);
                        collisionDetected = true;
                        ghostArgs->ghost->ghostCanMove = false;
                    }
                    ILLUMINATETHEPATHTOTARGET(ghostArgs, foundPath, leftHome, collisionDetected);   
                }
                else {
                    if (!foundPath) {
                        ghostArgs->ghost->setTarget(make_pair(PlayerX, PlayerY)); // setting new target for ghost
                        Point ghostPosition, ghostTarget;
                        ghostPosition.row = (ghostArgs->ghost->getSprite().getPosition().y - 100) / CELLSIZE, ghostPosition.col = (ghostArgs->ghost->getSprite().getPosition().x - 100) / CELLSIZE;
                        ghostTarget.row = PlayerY, ghostTarget.col = PlayerX;
                        GETPATHTOTARGET(ghostArgs->ghost, ghostPosition, ghostTarget);
                        foundPath = true;
                        ghostArgs->ghost->ghostCanMove = false;
                    }
                    ILLUMINATETHEPATHTOTARGET(ghostArgs, foundPath, leftHome, collisionDetected);
                    if (elapsedTime >= pathFindingInterval) {
                        ghostArgs->ghost->currentTarget = 0;
                        foundPath = false;
                        ghostArgs->ghost->ghostCanMove = false;
                        elapsedTime = 0;
                        clock.restart();
                    }
                }
                if (ghostArgs->ghost->ghostHasSpeedBoost) {
                    ghostArgs->ghost->ghostElapsedTime += ghostArgs->ghost->ghostClock.getElapsedTime().asSeconds();
                    if (ghostArgs->ghost->ghostElapsedTime >= speedBoostInterval) {
                        ghostArgs->ghost->setSpeed(0);
                        ghostArgs->ghost->ghostHasSpeedBoost = false;
                    }
                }
                MOVETHEGHOST(ghostArgs);
            }
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
            else if (maze1[i][j] == 3) {
                boost.setPosition((j * CELLSIZE) + 100, (i * CELLSIZE) + 100);
                window.draw(boost);
            }
            else if (maze1[i][j] == 4) {
                power.setPosition((j * CELLSIZE) + 100, (i * CELLSIZE) + 100);
                window.draw(power);
            }
        }
    }
    // these are for the portals
    mazeBox.setPosition(50, 450);
    window.draw(mazeBox);
    mazeBox.setPosition(0, 450);
    window.draw(mazeBox);
    mazeBox.setPosition(50, 550);
    window.draw(mazeBox);
    mazeBox.setPosition(0, 550);
    window.draw(mazeBox);
    mazeBox.setPosition(1150, 450);
    window.draw(mazeBox);
    mazeBox.setPosition(1200, 450);
    window.draw(mazeBox);
    mazeBox.setPosition(1150, 550);
    window.draw(mazeBox);
    mazeBox.setPosition(1200, 550);
    window.draw(mazeBox);
    return;
}

void *GAMEINIT(void *arg) { // main game thread
    RenderWindow gameWindow(VideoMode(GRIDWIDTH + 200, GRIDHEIGHT + 200), "PACMAN FURY", Style::Default);
    
    box.loadFromFile("sprites/box.png"); // loading the texture with maze.png
    mazeBox.setTexture(box); // setting the Game Grid Sprite to maze texture
    mazeBox.scale(5.0f, 5.0f); // scaling the sprite accoring to the cell size to fit in the screen

    Texture menuBG;
    menuBG.loadFromFile("images/BG.jpg");
    Texture mainBG;
    mainBG.loadFromFile("images/menuBackground.jpg");

    Sprite menuBackground;
    menuBackground.setTexture(menuBG);
    Sprite mainBackground;
    mainBackground.setTexture(mainBG);

    // loading up textures for eatables
    Texture booster;
    booster.loadFromFile("sprites/ghostBooster.png");
    boost.setTexture(booster);
    boost.setScale(0.4167f, 0.4167f);
    Texture powerUp;
    powerUp.loadFromFile("sprites/cherry.png");
    power.setTexture(powerUp);
    power.setScale(0.2778f, 0.2778f);

    Texture ghostTextures[4];
    ghostTextures[0].loadFromFile("sprites/redGhost.png");
    ghostTextures[1].loadFromFile("sprites/greenGhost.png");
    ghostTextures[2].loadFromFile("sprites/yellowGhost.png");
    ghostTextures[3].loadFromFile("sprites/pinkGhost.png");

    blueGhostTex.loadFromFile("sprites/blueGhost.png");

    // creating vector of PPL and SBL to pass them in playerArgs
    POWERPELLETLOCATIONS ppl[4];
    ppl[0].x = 1, ppl[0].y = 1; 
    ppl[1].x = 14, ppl[1].y = 3; 
    ppl[2].x = 19, ppl[2].y = 8; 
    ppl[3].x = 6, ppl[3].y = 13; 

    SPEEDBOOSTERLOCATIONS sbl[2];
    sbl[0].x = 19, sbl[0].y = 1;
    sbl[1].x = 4, sbl[1].y = 11;

    int initialTotalGhost = 3;

    GHOSTARGS ghostArgs[initialTotalGhost];
    GHOST *ghosts[initialTotalGhost];

    for (int i = 0; i < initialTotalGhost; i++) {
        ghosts[i] = new GHOST(ghostTextures[i], i, i);
        ghostArgs[i].ghost = ghosts[i];
        ghostArgs[i].SBL.push_back(&sbl[0]);
        ghostArgs[i].SBL.push_back(&sbl[1]);
    }

    playerTexLeft.loadFromFile("sprites/mouthOpenLeft.png"); // loading left side player png
    playerTexRight.loadFromFile("sprites/mouthOpenRight.png"); // loading default player png
    playerTexUp.loadFromFile("sprites/mouthOpenUp.png"); // loading upwards player png
    playerTexDown.loadFromFile("sprites/mouthOpenDown.png"); // loading downwards player png

    playerTexLeftClose.loadFromFile("sprites/leftMouthClose.png"); // loading left side player png
    playerTexRightClose.loadFromFile("sprites/rightMouthClose.png"); // loading default player png
    playerTexUpClose.loadFromFile("sprites/upMouthClose.png"); // loading upwards player png
    playerTexDownClose.loadFromFile("sprites/downMouthClose.png"); // loading downwards player png

    pointer.setTexture(playerTexLeft);
    pointer.setPosition(800, 250);

    PLAYER playerObj(playerTexRight); // creating player obj,mkmlkjj
    MENU menuObj;

    // setting up player's args
    PLAYERARGS playerArgs;
    playerArgs.player = &playerObj;
    for (int i = 0; i < 4; i++)
        playerArgs.PPL.push_back(&ppl[i]);
    playerArgs.menu = &menuObj;

    pthread_attr_t detachProp; // setting detachable property
    pthread_attr_init(&detachProp); // initializing that property
    pthread_attr_setdetachstate(&detachProp, PTHREAD_CREATE_DETACHED); // making it detachable
    pthread_t playerThread; 
    pthread_create(&playerThread, &detachProp, PLAYERTHREAD, &playerArgs); // creating a detachable player thread
    totalThreads++;

    //to display the string "Score" on the upper left corner
    Text displayScoreString;
    Font font;
    font.loadFromFile("fonts/normal.ttf");
    displayScoreString.setFont(font);
    displayScoreString.setCharacterSize(30);
    displayScoreString.setString("Score: ");
    displayScoreString.setFillColor(Color::White);
    displayScoreString.setPosition(100, 50);

    //to display the value of score on screen
    Text displayScore;
    displayScore.setFont(font);
    displayScore.setCharacterSize(30);
    displayScore.setFillColor(Color::White);
    displayScore.setPosition(250, 50);

    pthread_t ghostThread[initialTotalGhost]; 
    for (int i = 0; i < initialTotalGhost; i++) {
        pthread_create(&ghostThread[i], &detachProp, GHOSTTHREAD, &ghostArgs[i]); // creating a detachable ghost thread
        totalThreads++;
    }
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
        if (isMenu) {
            pthread_mutex_lock(&objectMovementSynchronisor);
            if(Keyboard::isKeyPressed(Keyboard::Up)) {
                if (!keyPressed) {
                    moveCursorUp = true;
                    keyPressed = true;
                }
                moveCursorDown = false;
            }
            else if(Keyboard::isKeyPressed(Keyboard::Down)) {
                moveCursorUp = false;
                if (!keyPressed) {
                    moveCursorDown = true;
                    keyPressed = true;
                }
            }
            else if(Keyboard::isKeyPressed(Keyboard::Enter)) {
                pressedEnter = true;
            }
            else if (keyPressed)
                keyPressed = false;
            pthread_mutex_unlock(&objectMovementSynchronisor);
        }
        else if (isGamePlay) {// taking user input
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
            elapsedTimeForPP += clockForPP.getElapsedTime().asSeconds();
            elapsedTimeForSB += clockForSB.getElapsedTime().asSeconds();

            if (powerPelletsCount < 4 && elapsedTimeForPP >= displayPPAndSBInterval) {
                powerPelletsCount++;
                srand(time(0) ^ pthread_self());
                int pplIndex = rand() % 4;
                while (ppl[pplIndex].isDisplayed)
                    pplIndex = rand() % 4;
                ppl[pplIndex].isDisplayed = true;
                maze1[ppl[pplIndex].y][ppl[pplIndex].x] = 4; // setting food for pacman
                clockForPP.restart();
                elapsedTimeForPP = 0;
            }

            if (speedBoostersCount < 2 && elapsedTimeForSB >= displayPPAndSBInterval) {
                speedBoostersCount++;
                srand(time(0) ^ pthread_self());
                int sblIndex = rand() % 2;
                while (sbl[sblIndex].isDisplayed)
                    sblIndex = rand() % 2;
                sbl[sblIndex].isDisplayed = true;
                maze1[sbl[sblIndex].y][sbl[sblIndex].x] = 3; // setting speed booster for ghosts
                clockForSB.restart();
                elapsedTimeForSB = 0;
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
        }
        
        gameWindow.clear(); // clearing the buffer window
        if (isMenu) {
            gameWindow.draw(menuBackground);
            gameWindow.draw(menuObj.heading);
            for (int i = 0; i < 3; i++)
                gameWindow.draw(menuObj.option[i]);
            gameWindow.draw(pointer);
        }
        else if (isGamePlay) {
            gameWindow.draw(mainBackground);
            DRAWMAZE(gameWindow, Food, mazeBox); // Drawing the maze with food and mazeBoxes
            for (int i = 0; i < initialTotalGhost; i++)
                gameWindow.draw(ghosts[i]->getSprite());
            gameWindow.draw(playerObj.getSprite());
            gameWindow.draw(displayScoreString);
            displayScore.setString(to_string(playerObj.getScore())); //converting score to string so that it can be displayed
            gameWindow.draw(displayScore);
        }   
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
    pthread_mutex_destroy(&pathIlluminator);
    pthread_mutex_destroy(&ghostMover);
    return 0;
}