#include <iostream>

#include "Game.h"
#include "Scene.h"
#include "Player.h"
#include "NetMessages.h"

// Inizializzazione membro statico
Game* Game::instance = nullptr;

Game* Game::getInstance(sf::RenderWindow* window)
{
    if(instance == nullptr)
    {
        instance = new Game(window);
    }
    else
    {
        std::cerr << "Game already instantiated" << std::endl;
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