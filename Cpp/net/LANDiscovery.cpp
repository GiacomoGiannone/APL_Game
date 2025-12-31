#include "LANDiscovery.h"
#include <iostream>
#include <cstring>

LANDiscovery::LANDiscovery() : running(false), isHost(false), gamePort(8080) {}

LANDiscovery::~LANDiscovery() {
    stop();
}

bool LANDiscovery::startHostBroadcast(unsigned short port, const std::string& name) {
    if (running) return false;
    
    isHost = true;
    gamePort = port;
    serverName = name;
    
    // Il socket UDP per broadcasting
    socket.setBlocking(false);
    
    // Abilita il broadcast
    running = true;
    broadcastThread = std::thread(&LANDiscovery::broadcastLoop, this);
    
    std::cout << "[LAN] Host broadcast avviato sulla porta " << LAN_DISCOVERY_PORT << std::endl;
    return true;
}

bool LANDiscovery::startClientListen() {
    if (running) return false;
    
    isHost = false;
    
    // Bind sulla porta di discovery per ricevere i broadcast
    // Usa sf::IpAddress::Any per ricevere da qualsiasi interfaccia
    if (socket.bind(LAN_DISCOVERY_PORT, sf::IpAddress::Any) != sf::Socket::Done) {
        std::cerr << "[LAN] Impossibile fare bind sulla porta " << LAN_DISCOVERY_PORT << std::endl;
        std::cerr << "[LAN] Potrebbe essere gia' in uso da un altro programma" << std::endl;
        return false;
    }
    
    socket.setBlocking(false);
    running = true;
    listenThread = std::thread(&LANDiscovery::listenLoop, this);
    
    std::cout << "[LAN] Client in ascolto sulla porta UDP " << LAN_DISCOVERY_PORT << std::endl;
    std::cout << "[LAN] Assicurati che il firewall permetta UDP sulla porta " << LAN_DISCOVERY_PORT << std::endl;
    return true;
}

void LANDiscovery::stop() {
    running = false;
    
    if (broadcastThread.joinable()) {
        broadcastThread.join();
    }
    if (listenThread.joinable()) {
        listenThread.join();
    }
    
    socket.unbind();
}

void LANDiscovery::broadcastLoop() {
    sf::UdpSocket broadcastSocket;
    broadcastSocket.setBlocking(true);
    
    ServerAnnouncement announcement;
    announcement.gamePort = gamePort;
    std::strncpy(announcement.serverName, serverName.c_str(), sizeof(announcement.serverName) - 1);
    announcement.serverName[sizeof(announcement.serverName) - 1] = '\0';
    
    while (running) {
        // Manda il broadcast all'indirizzo di broadcast (255.255.255.255)
        sf::Socket::Status status = broadcastSocket.send(
            &announcement, 
            sizeof(announcement), 
            sf::IpAddress::Broadcast, 
            LAN_DISCOVERY_PORT
        );
        
        if (status != sf::Socket::Done) {
            std::cerr << "[LAN] Errore invio broadcast" << std::endl;
        }
        
        // Aspetta 2 secondi prima del prossimo broadcast
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void LANDiscovery::listenLoop() {
    char buffer[sizeof(ServerAnnouncement)];
    std::size_t received;
    sf::IpAddress sender;
    unsigned short senderPort;
    
    while (running) {
        sf::Socket::Status status = socket.receive(
            buffer, 
            sizeof(buffer), 
            received, 
            sender, 
            senderPort
        );
        
        if (status == sf::Socket::Done && received == sizeof(ServerAnnouncement)) {
            ServerAnnouncement* announcement = reinterpret_cast<ServerAnnouncement*>(buffer);
            
            // Verifica il magic number
            if (announcement->magic[0] == 'A' && 
                announcement->magic[1] == 'P' && 
                announcement->magic[2] == 'L' && 
                announcement->magic[3] == 'G') 
            {
                FoundServer server;
                server.ip = sender.toString();
                server.port = announcement->gamePort;
                server.name = announcement->serverName;
                
                // Aggiungi alla lista se non esiste già
                std::lock_guard<std::mutex> lock(serversMutex);
                bool exists = false;
                for (const auto& s : foundServers) {
                    if (s.ip == server.ip && s.port == server.port) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    foundServers.push_back(server);
                    std::cout << "[LAN] Trovato server: " << server.name 
                              << " @ " << server.ip << ":" << server.port << std::endl;
                }
            }
        }
        
        // Piccola pausa per non sovraccaricare la CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::vector<FoundServer> LANDiscovery::getFoundServers() {
    std::lock_guard<std::mutex> lock(serversMutex);
    return foundServers;
}

void LANDiscovery::clearServers() {
    std::lock_guard<std::mutex> lock(serversMutex);
    foundServers.clear();
}
