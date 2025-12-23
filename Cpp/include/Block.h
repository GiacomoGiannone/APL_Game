#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "GameObject.h"

class Block: public GameObject
{
    private:
        //sprite already holds x,y,width and height
        //no point in duplicating them here
        sf::Sprite sprite;
        sf::Texture texture;
    public:
        Block(float x, float y, const std::string& texturePath);
        sf::FloatRect getBounds() const;
        void draw(sf::RenderWindow& window) override;
        void update(const Scene& scene) override;
};