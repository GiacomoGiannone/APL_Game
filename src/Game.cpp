#pragma once

#include <iostream>

#include "Game.h"
#include "Scene.h"

Game* Game::instance = nullptr;

Game* Game::getInstance(sf::RenderWindow* window)
{
    if(instance == nullptr)
    {
        instance = new Game(window);
        return instance;
    }
    else
    {
        std::cerr << "Game already instantiated" << std::endl;
    }
}

Game::Game(sf::RenderWindow* window):window(window)
{
}

void Game::update(float dt)
{
    currentScene->setDt(dt);
    currentScene->update();
}

void Game::setScene(Scene* newScene)
{
    currentScene = newScene;
}