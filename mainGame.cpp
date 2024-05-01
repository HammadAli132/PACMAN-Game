#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
using namespace sf;

#define GRIDHEIGHT 800
#define GRIDWIDTH 1000
#define CELLSize 25
const int gridRows = GRIDHEIGHT / CELLSize;
const int gridCols = GRIDWIDTH / CELLSize;

void *GAMEINIT(void *arg) { // main game thread
    RenderWindow gameWindow(VideoMode(GRIDWIDTH, GRIDHEIGHT), "PACMAN Game", Style::Default);
    Texture maze; // creating a texture for maze.png
    maze.loadFromFile("sprites/grid.png"); // loading the texture with maze.png
    Sprite gameGrid; // creating a sprite for Game Grid
    gameGrid.setTexture(maze); // setting the Game Grid Sprite to maze texture
    gameGrid.scale(CELLSize, CELLSize); // scaling the sprite accoring to the cell size to fit in the screen
    while (gameWindow.isOpen()) {
        Event event;
        while (gameWindow.pollEvent(event)) { // checking for window close command
            if (event.type == Event::Closed)
                gameWindow.close();
        }
        gameWindow.clear(); // clearing the buffer window
        gameWindow.draw(gameGrid); // drawing the sprite to buffer window
        gameWindow.display(); // swapping the buffer window with main window
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t startGame;
    pthread_create(&startGame, NULL, GAMEINIT, NULL); // initiating main game thread
    pthread_join(startGame, NULL); // waiting for game thread to exit
    return 0;
}