#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
using namespace sf;

void *GAMEINIT(void *arg) {
    RenderWindow gameWindow(VideoMode(500.0f, 500.0f), "PACMAN Game", Style::Default);
    RectangleShape rectangle(Vector2f(50.0f, 100.0f));
    rectangle.setFillColor(Color::Blue);
    while (gameWindow.isOpen()) {
        Event event;
        while (gameWindow.pollEvent(event)) {
            if (event.type == Event::Closed)
                gameWindow.close();
        }
        gameWindow.clear();
        gameWindow.draw(rectangle);
        gameWindow.display();
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t startGame;
    pthread_create(&startGame, NULL, GAMEINIT, NULL);
    pthread_join(startGame, NULL);
    return 0;
}