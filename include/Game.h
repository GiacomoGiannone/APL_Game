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
        int localPlayerId;

    public:
        static Game* getInstance(sf::RenderWindow* window);
        void update(float dt);
        void setScene(Scene* newScene);
        void setLocalPlayerId(int id);
        int getLocalPlayerId() const;
};