#pragma once
#include <SFML/Network.hpp>
#include <iostream>

class NetMessages;

class NetworkClient 
{
    private:
        static NetworkClient* instance;
        sf::TcpSocket socket;
        bool connected;

        // Costruttore privato (Singleton)
        NetworkClient();

    public:
        // Accesso statico
        static NetworkClient* getInstance();
        
        // Connessione
        bool connect(const std::string& ip, unsigned short port);
        void disconnect();
        bool isConnected() const;

        // INVIO (Template per comodità)
        // Questa funzione magica accetta qualsiasi struct (Move, Login) e la spedisce
        template <typename T>
        void sendPacket(T& packet)
        {
            if (!connected)
                return;

            packet.header.packetSize = sizeof(T);

            const char* data = reinterpret_cast<const char*>(&packet);
            std::size_t totalSent = 0;
            std::size_t size = sizeof(T);

            while (totalSent < size)
            {
                std::size_t sent = 0;
                sf::Socket::Status status = socket.send(
                    data + totalSent,
                    size - totalSent,
                    sent
                );

                if (status != sf::Socket::Done)
                {
                    std::cerr << "Errore invio pacchetto!" << std::endl;
                    return;
                }

                totalSent += sent;
            }
        }


        // RICEZIONE (Semplificata per ora)
        // Cerca di ricevere dati nel buffer
        sf::Socket::Status receive(void* data, std::size_t size, std::size_t& received);
        
        sf::TcpSocket& getSocket(); // Getter se serve accesso diretto
};