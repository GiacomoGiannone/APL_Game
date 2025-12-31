#include "Scene.h"
#include <typeinfo>
#include <iostream>
#include <algorithm>

#include "Block.h"
#include "Player.h"
#include "Enemy.h"
#include "Hittable.h"
#include "Game.h"

#include "NetworkClient.h"
#include "NetMessages.h"

Scene::Scene() : isHost(false) {}

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

std::vector<Enemy*> Scene::getEnemies() const
{
    std::vector<Enemy*> enemies;
    for (const auto& entity : entities)
    {
        if (Enemy* e = dynamic_cast<Enemy*>(entity.get()))
        {
            enemies.push_back(e);
        }
    }
    return enemies;
}

float Scene::getDt() const
{
    return dt;
}

void Scene::addEntity(std::unique_ptr<GameObject> entity)
{
    entities.push_back(std::move(entity));
}

// Implementazione della funzione helper definita in Scene.h
void Scene::addRemotePlayer(int id)
{
    // Creiamo il player remoto (false = non controllato da tastiera)
    auto remotePlayer = std::make_unique<Player>("PM1", "Nemico", false);
    remotePlayer->setId(id);
    addEntity(std::move(remotePlayer));
    std::cout << "🌐 Connesso nuovo giocatore remoto: ID " << id << std::endl;
}

void Scene::update()
{
    // --------------------------------------------------------
    // GESTIONE RETE
    // --------------------------------------------------------
    PacketHeader header;
    size_t received;

    // Processiamo TUTTI i pacchetti arrivati (non solo uno alla volta).
    while (NetworkClient::getInstance()->receive(&header, sizeof(header), received) == sf::Socket::Done)
    {
        // Controllo validità header
        if (received != sizeof(header)) break;

        // Pacchetto LOGIN: il server ci comunica il nostro ID reale
        if (header.type == PacketType::LOGIN)
        {
            uint32_t serverAssignedId;
            if (NetworkClient::getInstance()->receive(&serverAssignedId, sizeof(serverAssignedId), received) != sf::Socket::Done)
                break;
            
            std::cout << "🆔 Server ci ha assegnato ID: " << serverAssignedId << std::endl;
            
            // Aggiorna l'ID del player locale
            for (auto* player : getPlayers())
            {
                if (player->isLocal())
                {
                    player->setId(serverAssignedId);
                    std::cout << "   Player locale aggiornato con ID " << serverAssignedId << std::endl;
                    break;
                }
            }
            
            // Aggiorna anche localPlayerId nella scena
            localPlayerId = serverAssignedId;
            
            // Aggiorna nel Game
            Game::getInstance()->setLocalPlayerId(serverAssignedId);
            
            continue;
        }

        if (header.type == PacketType::MOVE)
        {
            PacketMove movePacket;
            movePacket.header = header;      
            
            size_t remainingSize = sizeof(PacketMove) - sizeof(PacketHeader);
            char* buffer = (char*)&movePacket + sizeof(PacketHeader); 
            
            // Riceviamo il corpo del messaggio
            if (NetworkClient::getInstance()->receive(buffer, remainingSize, received) != sf::Socket::Done)
                break; // Errore di lettura, usciamo

            // Se il pacchetto è mio (del local player), lo ignoro.
            // (Il server me lo rimanda indietro, ma io so già dove sono)
            if (movePacket.playerId == localPlayerId) continue;

            bool found = false;
            auto players = getPlayers(); 

            // 1. Aggiornamento Player Esistente
            for (auto* player : players)
            {
                if (player->getId() == movePacket.playerId)
                {
                    player->syncFromNetwork(
                        movePacket.x, movePacket.y, 
                        movePacket.velocityX, movePacket.velocityY, 
                        movePacket.isFacingRight, movePacket.isGrounded
                    );
                    found = true;
                    break;
                }
            }

            // 2. Creazione Nuovo Player (se non trovato)
            if (!found)
            {
                // Usiamo la funzione helper per pulizia
                addRemotePlayer(movePacket.playerId);

                // E dobbiamo sincronizzarlo SUBITO per evitare che appaia a (0,0) per un frame
                // Cerchiamo l'ultimo elemento aggiunto (che è il nostro nuovo player)
                // Nota: sappiamo che è un Player perché lo abbiamo appena aggiunto.
                if (Player* newP = dynamic_cast<Player*>(entities.back().get()))
                {
                    newP->syncFromNetwork(
                        movePacket.x, movePacket.y, 
                        movePacket.velocityX, movePacket.velocityY, 
                        movePacket.isFacingRight, movePacket.isGrounded
                    );
                }
            }
        }
        else if (header.type == PacketType::ENEMY_SPAWN)
        {
            PacketEnemySpawn spawnPacket;
            spawnPacket.header = header;
            
            size_t remainingSize = sizeof(PacketEnemySpawn) - sizeof(PacketHeader);
            char* buffer = (char*)&spawnPacket + sizeof(PacketHeader);
            
            if (NetworkClient::getInstance()->receive(buffer, remainingSize, received) != sf::Socket::Done)
                break;
            
            // Controlla se il nemico esiste già
            bool found = false;
            for (auto* enemy : getEnemies())
            {
                if (enemy->getId() == spawnPacket.enemyId)
                {
                    found = true;
                    break;
                }
            }
            
            // Se non esiste, crealo (nemico controllato dall'host, noi siamo client)
            if (!found)
            {
                auto remoteEnemy = std::make_unique<Enemy>("PM2", spawnPacket.enemyId, false); // false = non controlliamo
                remoteEnemy->setInitialPosition(spawnPacket.x, spawnPacket.y);
                addEntity(std::move(remoteEnemy));
                
                // Aggiorna il contatore di nemici da sconfiggere
                Game::getInstance()->incrementEnemiesToDefeat();
                
                std::cout << "👾 Nemico spawnato da host: ID " << spawnPacket.enemyId 
                          << " a (" << spawnPacket.x << ", " << spawnPacket.y << ")" << std::endl;
            }
        }
        else if (header.type == PacketType::ENEMY_UPDATE)
        {
            PacketEnemyUpdate enemyPacket;
            enemyPacket.header = header;
            
            size_t remainingSize = sizeof(PacketEnemyUpdate) - sizeof(PacketHeader);
            char* buffer = (char*)&enemyPacket + sizeof(PacketHeader);
            
            if (NetworkClient::getInstance()->receive(buffer, remainingSize, received) != sf::Socket::Done)
                break;
            
            // Trova il nemico e aggiornalo
            bool found = false;
            for (auto* enemy : getEnemies())
            {
                if (enemy->getId() == enemyPacket.enemyId)
                {
                    enemy->syncFromNetwork(
                        enemyPacket.x, enemyPacket.y,
                        enemyPacket.velocityX, enemyPacket.velocityY,
                        enemyPacket.isFacingRight, enemyPacket.isGrounded,
                        enemyPacket.isAttacking, enemyPacket.currentHealth
                    );
                    found = true;
                    break;
                }
            }
            
            // Se non esiste, crealo (nemico remoto)
            if (!found)
            {
                auto remoteEnemy = std::make_unique<Enemy>("PM2", enemyPacket.enemyId, false);
                remoteEnemy->syncFromNetwork(
                    enemyPacket.x, enemyPacket.y,
                    enemyPacket.velocityX, enemyPacket.velocityY,
                    enemyPacket.isFacingRight, enemyPacket.isGrounded,
                    enemyPacket.isAttacking, enemyPacket.currentHealth
                );
                addEntity(std::move(remoteEnemy));
                std::cout << "👾 Nemico remoto creato: ID " << enemyPacket.enemyId << std::endl;
            }
        }
        else if (header.type == PacketType::ENEMY_DAMAGE)
        {
            PacketEnemyDamage damagePacket;
            damagePacket.header = header;
            
            size_t remainingSize = sizeof(PacketEnemyDamage) - sizeof(PacketHeader);
            char* buffer = (char*)&damagePacket + sizeof(PacketHeader);
            
            if (NetworkClient::getInstance()->receive(buffer, remainingSize, received) != sf::Socket::Done)
                break;
            
            // Applica il danno al nemico
            for (auto* enemy : getEnemies())
            {
                if (enemy->getId() == damagePacket.enemyId)
                {
                    enemy->takeDamage(damagePacket.damage);
                    std::cout << "👾 Nemico " << damagePacket.enemyId << " ha subito " << damagePacket.damage << " danni!" << std::endl;
                    break;
                }
            }
        }
        else if (header.type == PacketType::PLAYER_ATTACK)
        {
            PacketPlayerAttack attackPacket;
            attackPacket.header = header;
            
            size_t remainingSize = sizeof(PacketPlayerAttack) - sizeof(PacketHeader);
            char* buffer = (char*)&attackPacket + sizeof(PacketHeader);
            
            if (NetworkClient::getInstance()->receive(buffer, remainingSize, received) != sf::Socket::Done)
                break;
            
            // Ignora pacchetti del nostro player
            if (attackPacket.playerId == localPlayerId)
                continue;
            
            // Trova il player e attiva l'animazione di attacco
            for (auto* player : getPlayers())
            {
                if (player->getId() == attackPacket.playerId)
                {
                    player->triggerAttackAnimation();
                    break;
                }
            }
        }
        else if (header.type == PacketType::PLAYER_DAMAGE)
        {
            PacketPlayerDamage damagePacket;
            damagePacket.header = header;
            
            size_t remainingSize = sizeof(PacketPlayerDamage) - sizeof(PacketHeader);
            char* buffer = (char*)&damagePacket + sizeof(PacketHeader);
            
            if (NetworkClient::getInstance()->receive(buffer, remainingSize, received) != sf::Socket::Done)
                break;
            
            // Trova il player e applica il danno
            for (auto* player : getPlayers())
            {
                if (player->getId() == damagePacket.playerId)
                {
                    if (player->isLocal())
                    {
                        // Se siamo l'host, ignoriamo - l'host ha già applicato il danno al momento dell'invio
                        if (!isHost)
                        {
                            player->applyDamageFromHost(damagePacket.damage);
                        }
                    }
                    else
                    {
                        // Aggiorna il player remoto (questo è il caso dell'host che riceve info sul client)
                        player->syncDamageFromNetwork(damagePacket.damage, damagePacket.currentHealth);
                    }
                    break;
                }
            }
        }
    }

    // --------------------------------------------------------
    // AGGIORNAMENTO GIOCO
    // --------------------------------------------------------
    for(auto& entity : entities)
    {
        entity->update(*this);
    }
    
    // Rimuovi entità morte (dopo il loop per evitare crash)
    // E notifica il Game per ogni nemico sconfitto
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::unique_ptr<GameObject>& entity) {
                if (Hittable* hittable = dynamic_cast<Hittable*>(entity.get()))
                {
                    if (hittable->isDead())
                    {
                        // Se è un nemico, decrementa il contatore
                        if (dynamic_cast<Enemy*>(entity.get()))
                        {
                            Game::getInstance()->enemyDefeated();
                        }
                        // Se è il player locale, game over
                        if (Player* player = dynamic_cast<Player*>(entity.get()))
                        {
                            if (player->isLocal())
                            {
                                Game::getInstance()->setGameOver();
                                return false; // Non rimuovere il player
                            }
                        }
                        return true;
                    }
                }
                return false;
            }),
        entities.end()
    );
}

void Scene::draw(sf::RenderWindow& window) const
{
    for (auto& entity : entities)
    {
        entity->draw(window);
    }
}

void Scene::setDt(float dt)
{
    this->dt = dt;
}

Player* Scene::getLocalPlayerInScene()
{
    auto playersInScene = getPlayers();
    for(auto& player : playersInScene)
    {
        if(player->isLocal())
        {
            return player;
        }
    }
    return nullptr;
}

void Scene::removeAllEnemies()
{
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::unique_ptr<GameObject>& entity) {
                return dynamic_cast<Enemy*>(entity.get()) != nullptr;
            }),
        entities.end()
    );
}

void Scene::respawnLocalPlayer()
{
    Player* player = getLocalPlayerInScene();
    if (player) {
        // Resetta la salute
        player->resetHealth();
        // Respawn completo (posizione, sprite, stato)
        player->respawn();
    }
}