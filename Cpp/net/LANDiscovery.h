#pragma once
#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

// Porta usata per il discovery UDP
constexpr unsigned short LAN_DISCOVERY_PORT = 8888;

// Messaggio di broadcast del server
struct ServerAnnouncement {
    char magic[4] = {'A', 'P', 'L', 'G'};  // Magic number per identificare il nostro gioco
    unsigned short gamePort;               // Porta TCP del server di gioco
    char serverName[32];                   // Nome del server/host
};

// Informazioni su un server trovato
struct FoundServer {
    std::string ip;
    unsigned short port;
    std::string name;
};

class LANDiscovery {
private:
    sf::UdpSocket socket;
    std::thread broadcastThread;
    std::thread listenThread;
    std::atomic<bool> running;
    
    std::mutex serversMutex;
    std::vector<FoundServer> foundServers;
    
    bool isHost;
    unsigned short gamePort;
    std::string serverName;

public:
    LANDiscovery();
    ~LANDiscovery();
    
    // Modalità Host: inizia a mandare broadcast sulla LAN
    bool startHostBroadcast(unsigned short gamePort, const std::string& serverName);
    
    // Modalità Client: ascolta per trovare server sulla LAN
    bool startClientListen();
    
    // Ferma il discovery
    void stop();
    
    // Ottieni la lista dei server trovati (per i client)
    std::vector<FoundServer> getFoundServers();
    
    // Pulisci la lista dei server
    void clearServers();
    
private:
    void broadcastLoop();
    void listenLoop();
};
