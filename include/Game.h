#pragma once

#include <SFML/Graphics.hpp>

#include "GameObject.h"

class Scene;

class Game
{   
    //Implements the singleton pattern
    private:
        Scene* currentScene;
        static Game* instance;
        sf::RenderWindow* window;
        Game(sf::RenderWindow* window);

    public:
        static Game* getInstance(sf::RenderWindow* window);
        void update();
        void setScene(Scene* newScene);
};