#pragma once

#include <SFML/Graphics.hpp>
#include "GameObject.h"

class Block: public GameObject
{
    private:
        //shape already holds x,y,width and height
        //no point in duplicating them here
        sf::RectangleShape shape;
        sf::Texture texture;
    public:
        Block(float x, float y, float width, float height, std::string texturePath);
        sf::FloatRect getBounds() const;
        void draw(sf::RenderWindow& window) override;
        void update(const Scene& scene) override;
};