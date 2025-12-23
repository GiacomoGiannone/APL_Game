#pragma once

#include <vector>
#include <memory>

#include "GameObject.h"

class Block;
class Player;

class Scene
{
private:
    std::vector<std::unique_ptr<GameObject>> entities;
    float dt;
    int localPlayerId;

public:
    Scene();

    std::vector<Block*> getBlocks() const;
    std::vector<Player*> getPlayers() const;
    void setDt(float dt);
    float getDt() const;
    void addEntity(std::unique_ptr<GameObject> entity);
    void update();
    void draw(sf::RenderWindow& window) const;
    Player* getLocalPlayerInScene();

    void setLocalPlayerId(int id) { localPlayerId = id; }
    int getLocalPlayerId() const { return localPlayerId; }
    void addRemotePlayer(int id);
};
