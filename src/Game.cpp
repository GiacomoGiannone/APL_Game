#include <iostream>

#include "Game.h"
#include "Scene.h"

// Inizializzazione membro statico
Game* Game::instance = nullptr;

Game* Game::getInstance(sf::RenderWindow* window)
{
    if(instance == nullptr)
    {
        if (window != nullptr) {
            instance = new Game(window);
        } else {
            std::cerr << "ERRORE: Tentativo di accedere a Game::getInstance() prima dell'inizializzazione!" << std::endl;
        }
    }
    return instance; 
}

Game::Game(sf::RenderWindow* window) : window(window)
{
}

void Game::update(float dt)
{
    if (currentScene != nullptr) 
    {
        currentScene->setDt(dt);
        currentScene->update();
    }
}

void Game::setScene(Scene* newScene)
{
    currentScene = newScene;
}

void Game::setLocalPlayerId(int id) {
    this->localPlayerId = id;
}

int Game::getLocalPlayerId() const {
    return this->localPlayerId;
}

void Game::setFocus(bool focus) {
    this->isWindowFocused = focus;
}

bool Game::hasFocus() const {
    return this->isWindowFocused;
}