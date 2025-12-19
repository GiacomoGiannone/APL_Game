#include "Block.h"
#include <iostream>

Block::Block(float x, float y, const std::string& texturePath)
{
    if (!texture.loadFromFile(texturePath))
    {
        std::cerr << "Could not load texture from " << texturePath << std::endl;
    }

    sprite.setTexture(texture);
    sprite.setPosition(x, y);

    // sf::IntRect rect({50,30,16,16});
    // sprite.setTextureRect(rect);
}

sf::FloatRect Block::getBounds() const
{
    return sprite.getGlobalBounds();
}

void Block::draw(sf::RenderWindow& window)
{
    window.draw(sprite);
}

void Block::update(const Scene& scene)
{

}

