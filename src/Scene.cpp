#include "Scene.h"
#include <typeinfo>

#include "Block.h"
#include "Player.h"

Scene::Scene() = default;

std::vector<Block*> Scene::getBlocks() const
{
    std::vector<Block*> blocks;
    for (const auto& entity : entities)
    {
        if (Block* b = dynamic_cast<Block*>(entity.get()))
        {
            blocks.push_back(b);
        }
    }
    return blocks;
}

std::vector<Player*> Scene::getPlayers() const
{
    std::vector<Player*> players;
    for (const auto& entity : entities)
    {
        if (Player* p = dynamic_cast<Player*>(entity.get()))
        {
            players.push_back(p);
        }
    }
    return players;
}


float Scene::getDt() const
{
    return dt;
}

void Scene::addEntity(std::unique_ptr<GameObject> entity)
{
    entities.push_back(std::move(entity));
}

void Scene::update()
{
    for(auto& entity : entities)
    {
        entity->update(*this);
    }
}

void Scene::draw(sf::RenderWindow& window) const
{
    for (auto& entity : entities)
    {
        entity->draw(window);
    }
}
