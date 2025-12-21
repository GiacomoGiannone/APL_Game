#include "Scene.h"
#include <typeinfo>

#include "Block.h"
#include "Player.h"

#include "NetworkClient.h"
#include "NetMessages.h"

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
    /* Prima di aggiornare la scena, controlliamo se ci sono messaggi di rete in arrivo
       dal server, e applichiamo gli aggiornamenti di stato ai giocatori remoti.*/
    // Creiamo un buffer per ricevere l'header del pacchetto e verificarne il tipo
    PacketHeader header;
    size_t received;
    // Proviamo a ricevere l'header (in modalità non bloccante)
    auto status = NetworkClient::getInstance()->receive(&header, sizeof(header), received);
    if (status == sf::Socket::Done && received == sizeof(header))
    {
        // Abbiamo ricevuto un pacchetto! Vediamo se è di movimento.
        if (header.type == PacketType::MOVE)
        {
            // È di tipo MOVE, riceviamo il resto del pacchetto, ma prima creiamo un nuovo pacchetto e copiamo l'header già letto
            PacketMove movePacket;
            movePacket.header = header;      
            // Per leggere il resto del pacchetto, calcoliamo prima la dimensione del body, e troviamo il puntatore al body
            size_t remainingSize = sizeof(PacketMove) - sizeof(PacketHeader);
            char* buffer = (char*)&movePacket + sizeof(PacketHeader); // Puntatore al body calcolato come indirizzo di mem in cui inizia il move packet + sizeof header
            // Riceviamo il resto del pacchetto
            NetworkClient::getInstance()->receive(buffer, remainingSize, received);
            
            // Ora abbiamo il pacchetto completo in movePacket, cerchiamo il giocatore corrispondente e applichiamo l'aggiornamento.            
            auto players = getPlayers();
            for (auto* player : players)
            {
                // Se il player ha l'ID corrispondente e NON è quello locale, applichiamo l'aggiornamento
                if (player->getId() == movePacket.playerId && !player->isLocal())
                {
                    player->syncFromNetwork(
                        movePacket.x, 
                        movePacket.y, 
                        movePacket.velocityX, 
                        movePacket.velocityY, 
                        movePacket.isFacingRight, 
                        movePacket.isGrounded
                    );
                    break;
                }
            }
        }
    }

    // Ora aggiorniamo tutti gli oggetti nella scena
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

void Scene::setDt(float dt)
{
    this->dt = dt;
}

Player* Scene::getLocalPlayerInScene()
{
    std::vector<Player*> playersInScene = getPlayers();
    for(auto& player : playersInScene)
    {
        if(player->isLocal())
        {
            return player;
        }
    }
    return nullptr;
}