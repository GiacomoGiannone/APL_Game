#include "NetworkClient.h"
#include "NetMessages.h"

#include "NetworkClient.h"

#include <SFML/System.hpp>

// Inizializzazione membro statico
NetworkClient* NetworkClient::instance = nullptr;

NetworkClient::NetworkClient() : connected(false) 
{
    // Imposta il socket come NON-BLOCCANTE.
    // Questo è vitale: se il server non risponde, il gioco NON deve freezarsi.
    socket.setBlocking(false); 
}

NetworkClient* NetworkClient::getInstance() 
{
    if (instance == nullptr) 
    {
        instance = new NetworkClient();
    }
    return instance;
}

bool NetworkClient::connect(const std::string& ip, unsigned short port) 
{
    // Per connettersi la prima volta, meglio usare il blocking temporaneamente
    // per essere sicuri che la connessione avvenga.
    socket.setBlocking(true); 
    
    sf::Socket::Status status = socket.connect(ip, port, sf::seconds(5));
    
    if (status == sf::Socket::Status::Done) 
    {
        connected = true;
        std::cout << "Connesso al server Go " << ip << ":" << port << std::endl;
        socket.setBlocking(false); // Rimettiamo non-blocking per il gioco
        return true;
    } 
    else 
    {
        std::cerr << "Impossibile connettersi al server!" << std::endl;
        connected = false;
        return false;
    }
}

void NetworkClient::disconnect() 
{
    socket.disconnect();
    connected = false;
}

bool NetworkClient::isConnected() const 
{
    return connected;
}

sf::TcpSocket& NetworkClient::getSocket() 
{
    return socket;
}

sf::Socket::Status NetworkClient::receive(void* data, std::size_t size, std::size_t& received) 
{
    if (!connected) return sf::Socket::Status::Error;
    return socket.receive(data, size, received);
}