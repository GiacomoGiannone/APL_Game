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
#include "Enemy.h"
#include "LANDiscovery.h"
#include "NetMessages.h"

// Punti di spawn possibili per i nemici
struct SpawnPoint {
    float x, y;
};

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
    std::cout << "Benvenuto " << playerName << "! Il tuo Player ID e': " << myPlayerId << std::endl;

    // -----------------------------------------------------------
    // LAN DISCOVERY - Cerca server sulla rete
    // -----------------------------------------------------------
    std::cout << "\n=== MODALITA' DI GIOCO ===" << std::endl;
    std::cout << "1. Gioca offline (single player)" << std::endl;
    std::cout << "2. OSPITA PARTITA (tu sei l'host, altri si connettono a te)" << std::endl;
    std::cout << "3. Cerca partita sulla LAN (automatico)" << std::endl;
    std::cout << "4. Connetti a IP specifico (inserisci manualmente)" << std::endl;
    std::cout << "Scelta: ";
    
    int choice;
    std::cin >> choice;
    
    bool connected = false;
    bool isGameHost = false;  // Se siamo l'host controlliamo i nemici
    LANDiscovery lanDiscovery;
    
    if (choice == 2) {
        // OSPITA PARTITA - Siamo l'host
        isGameHost = true;
        std::cout << "\n=== OSPITA PARTITA ===" << std::endl;
        std::cout << "Avvia il server Go (Server.go) in un terminale separato," << std::endl;
        std::cout << "poi premi INVIO per continuare..." << std::endl;
        std::cin.ignore();
        std::cin.get();
        
        if (NetworkClient::getInstance()->connect("127.0.0.1", 8080)) {
            std::cout << "CONNESSO! Sei l'HOST della partita!" << std::endl;
            std::cout << "Altri giocatori possono cercarti sulla LAN." << std::endl;
            connected = true;
        } else {
            std::cout << "ERRORE: Assicurati che Server.go sia in esecuzione!" << std::endl;
            std::cout << "Continuo in modalita' offline..." << std::endl;
            isGameHost = true; // Anche offline siamo host
        }
    } else if (choice == 3) {
        // Cerca server sulla LAN
        std::cout << "Ricerca partite sulla LAN in corso..." << std::endl;
        std::cout << "(Assicurati che il firewall permetta UDP porta 8888)" << std::endl;
        lanDiscovery.startClientListen();
        
        // Aspetta qualche secondo per trovare server
        std::cout << "Attendo 5 secondi per trovare partite..." << std::endl;
        sf::sleep(sf::seconds(5));
        
        auto servers = lanDiscovery.getFoundServers();
        lanDiscovery.stop();
        
        if (servers.empty()) {
            std::cout << "Nessuna partita trovata sulla LAN." << std::endl;
            std::cout << "Prova l'opzione 4 per connetterti con IP manuale." << std::endl;
        } else {
            std::cout << "\nPartite trovate:" << std::endl;
            for (size_t i = 0; i < servers.size(); i++) {
                std::cout << (i + 1) << ". " << servers[i].name 
                          << " @ " << servers[i].ip << ":" << servers[i].port << std::endl;
            }
            std::cout << "Seleziona partita (0 per offline): ";
            
            size_t serverChoice;
            std::cin >> serverChoice;
            
            if (serverChoice > 0 && serverChoice <= servers.size()) {
                const auto& server = servers[serverChoice - 1];
                if (NetworkClient::getInstance()->connect(server.ip, server.port)) {
                    std::cout << "CONNESSO a " << server.name << "!" << std::endl;
                    connected = true;
                    isGameHost = false; // Siamo un client
                }
            }
        }
    } else if (choice == 4) {
        // Connessione con IP manuale
        std::string ipAddress;
        std::cout << "Inserisci l'indirizzo IP dell'host (es: 192.168.1.100): ";
        std::cin >> ipAddress;
        
        std::cout << "Connessione a " << ipAddress << ":8080..." << std::endl;
        if (NetworkClient::getInstance()->connect(ipAddress, 8080)) {
            std::cout << "CONNESSO!" << std::endl;
            connected = true;
            isGameHost = false; // Siamo un client
        } else {
            std::cout << "IMPOSSIBILE CONNETTERSI a " << ipAddress << std::endl;
        }
    } else {
        std::cout << "Modalita' offline selezionata." << std::endl;
        isGameHost = true; // Offline siamo sempre host
    }
    
    if (!connected) {
        std::cout << "Gioco in modalita' OFFLINE" << std::endl;
        isGameHost = true; // Offline = host dei nemici
    }

    // -----------------------------------------------------------
    // 2. CREAZIONE FINESTRA E GIOCO
    // -----------------------------------------------------------
    sf::RenderWindow window(sf::VideoMode(800, 600), "Platformer Game");
    window.setVerticalSyncEnabled(true); 
    window.setFramerateLimit(120);

    // Inizializza il Singleton Game
    Game* game = Game::getInstance(&window);
    
    // Impostiamo se siamo host
    game->setIsHost(isGameHost);
    
    // Passiamo l'ID al Game (utile se serve globalmente)
    game->setLocalPlayerId(myPlayerId);

    // -----------------------------------------------------------
    // 3. COSTRUZIONE DELLA SCENA
    // -----------------------------------------------------------
    Scene scene;
    
    // Passiamo l'ID alla scena (fondamentale per filtrare i pacchetti)
    scene.setLocalPlayerId(myPlayerId); 

    // Creazione del Player Locale
    auto localPlayer = std::make_unique<Player>("PM1", playerName, true);
    localPlayer->setId(myPlayerId); // Assegniamo l'ID al nostro player così sa chi è quando invia i pacchetti
    scene.addEntity(std::move(localPlayer)); // Non serve più al main, lo passiamo alla scena

    // Aggiunta Blocchi (Mappa)
    // Funzione helper (lambda) per creare una fila di blocchi velocemente
    auto createPlatform = [&](float startX, float startY, int numBlocks) {
        for(int i = 0; i < numBlocks; i++) {
            scene.addEntity(std::make_unique<Block>(
                startX + (i * 15.0f),  // X: Si sposta di 15px per ogni blocco
                startY,                // Y: Rimane fissa per la piattaforma
                "assets/pp1/Blocks/block1.png"
            ));
        }
    };
    
    // Funzione helper per creare muri verticali
    auto createWall = [&](float startX, float startY, int numBlocks) {
        for(int i = 0; i < numBlocks; i++) {
            scene.addEntity(std::make_unique<Block>(
                startX,
                startY + (i * 15.0f),  // Y: Si sposta di 15px per ogni blocco
                "assets/pp1/Blocks/block1.png"
            ));
        }
    };
    
    // Piattaforme di gioco
    createPlatform(100.0f, 450.0f, 40);
    createPlatform(130.0f, 325.0f, 10);
    createPlatform(520.0f, 325.0f, 10);
    createPlatform(340.0f, 200.0f, 8);
    
    // -----------------------------------------------------------
    // BLOCCHI AI BORDI (per evitare di cadere fuori mappa)
    // -----------------------------------------------------------
    // Pavimento in basso (tutta la larghezza)
    createPlatform(0.0f, 550.0f, 54);
    // Muro sinistro
    createWall(0.0f, 0.0f, 40);
    // Muro destro
    createWall(785.0f, 0.0f, 40);
    // Soffitto in alto
    createPlatform(0.0f, 0.0f, 54);

    // -----------------------------------------------------------
    // SPAWN NEMICI IN PUNTI RANDOM
    // -----------------------------------------------------------
    // Definiamo i punti di spawn possibili (sulle piattaforme)
    std::vector<SpawnPoint> spawnPoints = {
        {150.0f, 420.0f},   // Piattaforma principale sinistra
        {300.0f, 420.0f},   // Piattaforma principale centro-sinistra
        {450.0f, 420.0f},   // Piattaforma principale centro-destra
        {600.0f, 420.0f},   // Piattaforma principale destra
        {180.0f, 295.0f},   // Piattaforma sinistra alta
        {570.0f, 295.0f},   // Piattaforma destra alta
        {380.0f, 170.0f},   // Piattaforma centrale più alta
    };
    
    // Solo l'HOST controlla i nemici e li sincronizza con i client
    bool isOffline = !NetworkClient::getInstance()->isConnected();
    bool isEnemyHost = isOffline || isGameHost; // Host o offline controlla i nemici
    
    // Lambda per spawmare i nemici del livello corrente (solo per HOST)
    auto spawnEnemiesForLevel = [&](int level) {
        // I CLIENT non spawnano nemici - li riceveranno via rete
        if (!isEnemyHost) {
            std::cout << "Client: aspetto nemici dall'host..." << std::endl;
            return;
        }
        
        // Numero base di nemici + 2 per ogni livello
        int numEnemies = 3 + (level * 2);
        
        // Massimo 15 nemici per evitare sovraccarico
        if (numEnemies > 15) numEnemies = 15;
        
        game->setEnemiesToDefeat(numEnemies);
        
        for (int i = 0; i < numEnemies; i++) {
            // Scegli un punto di spawn random
            int spawnIndex = std::rand() % spawnPoints.size();
            SpawnPoint spawn = spawnPoints[spawnIndex];
            
            // Aggiungi una piccola variazione casuale alla X
            float offsetX = static_cast<float>((std::rand() % 40) - 20);
            
            uint32_t enemyId = static_cast<uint32_t>(i + 1);
            auto enemy = std::make_unique<Enemy>("PM2", enemyId, true); // Host controlla sempre
            
            float spawnX = spawn.x + offsetX;
            float spawnY = spawn.y;
            
            // Impostiamo la posizione iniziale del nemico
            enemy->setInitialPosition(spawnX, spawnY);
            
            // Se online, invia pacchetto spawn ai client
            if (NetworkClient::getInstance()->isConnected()) {
                PacketEnemySpawn spawnPacket;
                spawnPacket.header.type = PacketType::ENEMY_SPAWN;
                spawnPacket.header.packetSize = sizeof(PacketEnemySpawn);
                spawnPacket.enemyId = enemyId;
                spawnPacket.x = spawnX;
                spawnPacket.y = spawnY;
                spawnPacket.maxHealth = 100.f;
                
                NetworkClient::getInstance()->sendPacket(spawnPacket);
                std::cout << "📤 Inviato spawn nemico ID " << enemyId << " a (" << spawnX << ", " << spawnY << ")" << std::endl;
            }
            
            scene.addEntity(std::move(enemy));
        }
        
        std::cout << "Livello " << level << " - Sconfiggi " << numEnemies << " nemici!" << std::endl;
    };
    
    // Spawn iniziale per livello 1
    spawnEnemiesForLevel(1);
    
    if (isOffline)
        std::cout << "Modalità OFFLINE - Controlli i nemici localmente" << std::endl;
    else
        std::cout << "Modalità ONLINE - Controlli i nemici localmente" << std::endl;

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

            if (event.type == sf::Event::GainedFocus) 
                game->setFocus(true);
                
            if (event.type == sf::Event::LostFocus) 
                game->setFocus(false);
            
            // Restart del gioco con R se game over
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
            {
                if (game->isGameOver())
                {
                    game->restartGame();
                    scene.removeAllEnemies();
                    scene.respawnLocalPlayer();
                    spawnEnemiesForLevel(1);
                }
            }
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f; // Limite per evitare glitch fisici se il gioco lagga

        // Controlla se il livello è completato
        if (game->isLevelComplete())
        {
            // Passa al livello successivo
            game->nextLevel();
            
            // Rimuovi tutti i nemici esistenti
            scene.removeAllEnemies();
            
            // Spawn nuovi nemici per il nuovo livello
            spawnEnemiesForLevel(game->getCurrentLevel());
        }

        // Update Logica (Input, Fisica, Rete)
        game->update(dt);

        // Render
        window.clear(sf::Color::Cyan);
        
        // Disegna tutto quello che c'è nella scena
        scene.draw(window);
        
        // Disegna l'interfaccia (contatore nemici, messaggio vittoria)
        game->drawUI();
        
        window.display();
    }

    // Pulizia finale
    NetworkClient::getInstance()->disconnect();
    return 0;
}