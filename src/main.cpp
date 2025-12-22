#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>   // Per time()
#include <cstdlib> // Per rand() e srand()

#include "Player.h"
#include "Block.h"
#include "Scene.h"
#include "Game.h"
#include "NetworkClient.h"

int main()
{
    // -----------------------------------------------------------
    // 1. INIZIALIZZAZIONE DATI E RETE
    // -----------------------------------------------------------
    
    // Inizializza il generatore casuale per l'ID
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int myPlayerId = std::rand() % 9000 + 1000;
    
    std::string playerName;
    std::cout << "Inserisci il tuo nome: ";
    std::cin >> playerName;
    std::cout << "Benvenuto " << playerName << "! Il tuo Player ID è: " << myPlayerId << std::endl;

    // Connessione al Server
    if (NetworkClient::getInstance()->connect("127.0.0.1", 8080))
    {
        std::cout << "CONNESSO AL SERVER!" << std::endl;
    }
    else
    {
        std::cout << "IMPOSSIBILE CONNETTERSI (Offline Mode)." << std::endl;
    }

    // -----------------------------------------------------------
    // 2. CREAZIONE FINESTRA E GIOCO
    // -----------------------------------------------------------
    sf::RenderWindow window(sf::VideoMode(800, 600), "Platformer Game");
    window.setVerticalSyncEnabled(true); 
    window.setFramerateLimit(60);

    // Inizializza il Singleton Game
    Game* game = Game::getInstance(&window);
    
    // Passiamo l'ID al Game (utile se serve globalmente)
    game->setLocalPlayerId(myPlayerId);

    // -----------------------------------------------------------
    // 3. COSTRUZIONE DELLA SCENA
    // -----------------------------------------------------------
    Scene scene;
    
    // Passiamo l'ID alla scena (fondamentale per filtrare i pacchetti)
    scene.setLocalPlayerId(myPlayerId); 

    // Creazione del Player Locale
    auto localPlayer = std::make_unique<Player>("assets/pp1", playerName, true);
    localPlayer->setId(myPlayerId); // Assegniamo l'ID al nostro player così sa chi è quando invia i pacchetti
    scene.addEntity(std::move(localPlayer));

    // Aggiunta Blocchi (Mappa)
    for(int i=0; i < 40; i++)
    {
        scene.addEntity(std::make_unique<Block>(15.0f*i, 500.0f, "assets/pp1/Blocks/block1.png"));
        if(i % 3 == 0)
            scene.addEntity(std::make_unique<Block>(15.0f*i, 400.0f, "assets/pp1/Blocks/block1.png"));
    }

    // Impostiamo la scena attiva nel gioco
    game->setScene(&scene);

    // -----------------------------------------------------------
    // 4. GAME LOOP
    // -----------------------------------------------------------
    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f; // Limite per evitare glitch fisici se il gioco lagga

        // Update Logica (Input, Fisica, Rete)
        game->update(dt);

        // Render
        window.clear(sf::Color::Cyan);
        
        // Disegna tutto quello che c'è nella scena
        scene.draw(window);
        
        window.display();
    }

    // Pulizia finale
    NetworkClient::getInstance()->disconnect();
    return 0;
}