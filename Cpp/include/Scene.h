#pragma once

#include <vector>
#include <memory>

#include "GameObject.h"

class Block;
class Player;
class Enemy;

class Scene
{
private:
    std::vector<std::unique_ptr<GameObject>> entities;
    float dt;
    int localPlayerId;
    bool isHost;  // True se siamo l'host

public:
    Scene();

    std::vector<Block*> getBlocks() const;
    std::vector<Player*> getPlayers() const;
    std::vector<Enemy*> getEnemies() const;
    void setDt(float dt);
    float getDt() const;
    void addEntity(std::unique_ptr<GameObject> entity);
    void update();
    void draw(sf::RenderWindow& window) const;
    Player* getLocalPlayerInScene();

    void setLocalPlayerId(int id) { localPlayerId = id; }
    int getLocalPlayerId() const { return localPlayerId; }
    void setIsHost(bool host) { isHost = host; }
    bool getIsHost() const { return isHost; }
    void addRemotePlayer(int id);
    void removePlayer(uint32_t playerId);  // Rimuove un player dalla scena
    void removeAllEnemies();
    void respawnLocalPlayer();
};
