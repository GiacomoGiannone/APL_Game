#pragma once

#include <SFML/Graphics.hpp>

class Scene;

class GameObject 
{
    public:
        virtual ~GameObject() = default;
        virtual void update(const Scene& scene) = 0;
        virtual void draw(sf::RenderWindow& window) = 0;
};
