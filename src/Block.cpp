#include "Block.h"
#include <iostream>

Block::Block(float x, float y, float width, float height, std::string texturePath)
{
    shape.setSize({ width, height });
    shape.setPosition(x, y);
    if(!texture.loadFromFile(texturePath))
    {
        std::cerr << "Could not load texture from path " << texturePath << std::endl;
    }
    texture.setRepeated(true);
    shape.setTexture(&texture);
    shape.setTextureRect(sf::IntRect(0, 0, static_cast<int>(width), static_cast<int>(height)));
}

sf::FloatRect Block::getBounds() const
{
    return shape.getGlobalBounds();
}

void Block::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

void Block::update(const Scene& scene)
{

}

