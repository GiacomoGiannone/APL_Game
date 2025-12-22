#include <SFML/Graphics.hpp>
#include <vector>

#include "Player.h"
#include "Block.h"
#include "Scene.h"
#include "Game.h"
#include "NetworkClient.h"

// struct Block {
//     sf::RectangleShape shape;

//     Block(float x, float y, float w, float h) {
//         shape.setSize({ w, h });
//         shape.setPosition(x, y);
//         shape.setFillColor(sf::Color::White);
//     }

//     sf::FloatRect getBounds() const {
//         return shape.getGlobalBounds();
//     }
// };

// void applyLetterbox(sf::View& view, int windowWidth, int windowHeight)
// {
//     float windowRatio = (float)windowWidth / (float)windowHeight;
//     float viewRatio = view.getSize().x / view.getSize().y;
//     float sizeX = 1.f;
//     float sizeY = 1.f;
//     float posX = 0.f;
//     float posY = 0.f;

//     if (windowRatio < viewRatio) {
//         sizeY = windowRatio / viewRatio;
//         posY = (1.f - sizeY) / 2.f;
//     }
//     else {
//         sizeX = viewRatio / windowRatio;
//         posX = (1.f - sizeX) / 2.f;
//     }

//     view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
// }

int main()
{
    // sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Collision Demo");

    // //--------------------------------------
    // // FIXED VIEW
    // //--------------------------------------
    // sf::View view(sf::FloatRect(0, 0, 800, 600));

    // //--------------------------------------
    // // PLAYER
    // //--------------------------------------
    // sf::RectangleShape player({ 40, 50 });
    // player.setFillColor(sf::Color::Red);
    // player.setPosition(100, 100);
    // sf::Vector2f velocity(0.f, 0.f);
    // float speed = 200.f;
    // float gravity = 700.f;
    // bool onGround = false;

    // //--------------------------------------
    // // LEVEL BLOCKS
    // //--------------------------------------
    // std::vector<Block> blocks;
    // blocks.emplace_back(0, 550, 800, 50);
    // blocks.emplace_back(300, 450, 200, 40);
    // blocks.emplace_back(100, 350, 120, 40);
    // blocks.emplace_back(500, 300, 150, 40);

    // sf::Clock clock;

    // //--------------------------------------
    // // MAIN LOOP
    // //--------------------------------------
    // while (window.isOpen())
    // {
    //     //--------------------------------------
    //     // EVENT HANDLING
    //     //--------------------------------------
    //     sf::Event e;
    //     while (window.pollEvent(e))
    //     {
    //         if (e.type == sf::Event::Closed)
    //             window.close();

    //         if (e.type == sf::Event::Resized)
    //             applyLetterbox(view, e.size.width, e.size.height);
    //     }

    //     window.setView(view);

    //     float dt = clock.restart().asSeconds();
    //     if (dt > 0.05f) dt = 0.05f;   // evita dt folli durante il resize

    //     //--------------------------------------
    //     // INPUT
    //     //--------------------------------------
    //     velocity.x = 0;
    //     if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) velocity.x -= speed;
    //     if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) velocity.x += speed;

    //     if (onGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
    //         velocity.y = -350.f;
    //         onGround = false;
    //     }

    //     //--------------------------------------
    //     // PHYSICS — STEP X
    //     //--------------------------------------
    //     player.move(velocity.x * dt, 0.f);
    //     sf::FloatRect pBounds = player.getGlobalBounds();

    //     for (auto& b : blocks) {
    //         if (pBounds.intersects(b.getBounds())) {
    //             sf::FloatRect bBounds = b.getBounds();

    //             if (velocity.x > 0)
    //                 player.setPosition(bBounds.left - pBounds.width, player.getPosition().y);
    //             else if (velocity.x < 0)
    //                 player.setPosition(bBounds.left + bBounds.width, player.getPosition().y);

    //             velocity.x = 0;
    //             pBounds = player.getGlobalBounds();
    //         }
    //     }

    //     //--------------------------------------
    //     // PHYSICS — STEP Y
    //     //--------------------------------------
    //     velocity.y += gravity * dt;
    //     player.move(0.f, velocity.y * dt);
    //     pBounds = player.getGlobalBounds();
    //     onGround = false;

    //     for (auto& b : blocks) {
    //         if (pBounds.intersects(b.getBounds())) {
    //             sf::FloatRect bBounds = b.getBounds();

    //             if (velocity.y > 0) {
    //                 player.setPosition(player.getPosition().x, bBounds.top - pBounds.height);
    //                 velocity.y = 0;
    //                 onGround = true;
    //             }
    //             else if (velocity.y < 0) {
    //                 player.setPosition(player.getPosition().x, bBounds.top + bBounds.height);
    //                 velocity.y = 0;
    //             }

    //             pBounds = player.getGlobalBounds();
    //         }
    //     }

    //     //--------------------------------------
    //     // RENDER
    //     //--------------------------------------
    //     window.clear(sf::Color::Black);

    //     for (auto& b : blocks)
    //         window.draw(b.shape);

    //     window.draw(player);

    //     window.display();
    // }

    // return 0;

    
    
    //creiamo la scena
    Scene scene;
    //aggiungiamo un giocatore alla scena
    std::string playerName;
    std::cout << "Inserisci il tuo nome!: ";
    std::cin >> playerName;
    scene.addEntity(std::make_unique<Player>("PM1", playerName, /*localPlayer*/true));
    //aggiungiamo dei blocchi alla scena
    //dimensione dei blocchi(texture)
    //15x15
    for(int i=0; i < 40; i++)
    {
        scene.addEntity(std::make_unique<Block>(15.0f*i, 500.0f, "assets/pp1/Blocks/block1.png"));
        if(i%3==0)
            scene.addEntity(std::make_unique<Block>(15.0f*i, 400.0f, "assets/pp1/Blocks/block1.png"));
    }
    //creiamo la finestra di gioco
    sf::RenderWindow window(sf::VideoMode(800, 600), "Platformer Game");
    window.setVerticalSyncEnabled(false); // DISABILITA VSYNC
    window.setFramerateLimit(60); // Limita a 60 FPS
   
   // Si usa "127.0.0.1" (Localhost) se il server Go gira sullo stesso PC.
    // Se il server è su un altro PC, si mette l'IP di quel PC.
    // La porta 8080 deve corrispondere a quella che si userà nel server Go.
    if (NetworkClient::getInstance()->connect("127.0.0.1", 8080))
    {
        std::cout << "CONNESSO AL SERVER!" << std::endl;
    }
    else
    {
        std::cout << "IMPOSSIBILE CONNETTERSI (Server spento o IP errato)." << std::endl;
        std::cout << "Il gioco girerà in modalità offline." << std::endl;
    }
   
    //creiamo l'istanza del gioco
    Game* game = Game::getInstance(&window);
    //settiamo la scena corrente
    game->setScene(&scene);
    //main loop
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
        if (dt > 0.05f) dt = 0.05f; //clamp dt to avoid issues during resizing

        //update game logic
        game->update(dt);

        //rendering
        window.clear(sf::Color::Cyan);
        scene.draw(window);
        window.display();
    }

    //Disconnessione dal server prima di chiudere il gioco
    NetworkClient::getInstance()->disconnect();
    return 0;
}
