#include "Player.h"
#include "Block.h"
#include "Scene.h"
#include "Game.h"
#include "NetMessages.h"
#include "NetworkClient.h"
#include <iostream>

Player::Player(std::string Folder, std::string playerName, bool localPlayer)
    : velocity(0.0f, 0.0f), isGrounded(false), speed(200.0f), gravity(200.0f),
      current_animation_frame(0), animation_timer(0.1f), animation_speed(0.1f),
      playerName(playerName), facingRight(true), localPlayer(localPlayer), folder(Folder)
{
    // Carica texture idle
    std::string path_to_texture = "assets/pp1/" + Folder + "/metarig.004-0_0000.png";
    if(!idle_texture.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load idle texture from path " << path_to_texture << std::endl;
    }
    
    // Carica texture walk
    sf::Texture text;
    for(int i = 2; i <= 5; i++)
    {
        path_to_texture = "assets/pp1/" + Folder + "/metarig.004-0_000" + std::to_string(i) + ".png";
        if(!text.loadFromFile(path_to_texture))
        {
            std::cerr << "Could not load walk texture from path " << path_to_texture << std::endl;
        }
        walk_textures.push_back(text);
    }
    
    // Carica texture djump
    path_to_texture = "assets/pp1/" + Folder + "/metarig.004-0_0009.png";
    if(!text.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load jump texture from path " << path_to_texture << std::endl;
    }
    jump_textures.push_back(text);
    
    path_to_texture = "assets/pp1/" + Folder + "/metarig.004-0_0010.png";
    if(!text.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load jump texture from path " << path_to_texture << std::endl;
    }
    jump_textures.push_back(text);
    
    // Carica texture falling
    path_to_texture = "assets/pp1/" + Folder + "/metarig.004-0_0011.png";
    if(!falling_texture.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load falling texture from path " << path_to_texture << std::endl;
    }
    
    // Setup sprite
    sprite.setTexture(idle_texture);
    
    // 1. MISURA MANUALE: guarda la texture e trova questi valori
    // Dovrai adattarli alla tua texture specifica
    float textureFullWidth = sprite.getLocalBounds().width;   // Es: 128px
    float textureFullHeight = sprite.getLocalBounds().height; // Es: 128px
    
    // 2. Dove inizia il personaggio nella texture (ritaglio)
    float characterStartX = 41.f;  // Spazio vuoto a sinistra prima del personaggio
    float characterStartY = 24.0f;  // Spazio vuoto sopra prima del personaggio
    float characterWidth = 15.f;   // Larghezza effettiva del personaggio  
    float characterHeight = 30.f; // Altezza effettiva del personaggio
    
    // 3. Imposta il textureRect per ritagliare SOLO il personaggio
    sprite.setTextureRect(sf::IntRect(
        characterStartX,      // X di inizio ritaglio
        characterStartY,      // Y di inizio ritaglio
        characterWidth,       // Larghezza del ritaglio
        characterHeight       // Altezza del ritaglio
    ));
    
    // 4. Imposta l'origine al CENTRO del personaggio ritagliato
    sprite.setOrigin(characterWidth / 2.f, characterHeight / 2.f);
    
    // 5. Posiziona il personaggio
    sprite.setPosition(100.f, 100.f);
    
    // 6. Configura il collider (ora è più semplice!)
    collider.width = characterWidth * 0.75;    // Collider più stretto del personaggio
    collider.height = characterHeight;  // Collider più basso del personaggio
    
    // Offset per centrare il collider DENTRO il personaggio
    colliderOffsetX = (characterWidth - collider.width) / 2.f;
    colliderOffsetY = characterHeight - collider.height; // Collider nella parte bassa
    
    updateCollider();
}

int Player::getId() const
{
    return id;
}

void Player::setId(int newId)
{
    id = newId;
}

void Player::handle_input()
{
    if (!Game::getInstance()->hasFocus()) return;

    velocity.x = 0.0f;
    //check if player wants to go to the left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        velocity.x -= speed;
        facingRight = false;
    }
    //check if player wants to go to the right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        velocity.x += speed;
        facingRight = true;
    }
    //check if player wants to jump
    if(isGrounded && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        //we will adjust gravity later in the applyGravity function
        //if not the player would keep flying
        velocity.y = -250.0f;
    }
}

void Player::apply_gravity(float dt)
{
    velocity.y += gravity * dt;
}

void Player::moveX(float dt, const std::vector<Block*>& blocks)
{
    sprite.move(velocity.x * dt, 0.0f);
    updateCollider();
    
    for(const auto& block : blocks)
    {
        if(collider.intersects(block->getBounds()))
        {
            // Calcola la sovrapposizione
            float overlap = 0.f;
            
            if(velocity.x > 0) // Destra
            {
                // Il lato destro del collider è oltre il lato sinistro del blocco
                overlap = (collider.left + collider.width) - block->getBounds().left;
                sprite.move(-overlap, 0.f);
            }
            else if(velocity.x < 0) // Sinistra
            {
                // Il lato sinistro del collider è oltre il lato destro del blocco
                overlap = block->getBounds().left + block->getBounds().width - collider.left;
                sprite.move(overlap, 0.f);
            }
            
            updateCollider();
            velocity.x = 0;
            break;
        }
    }
}

void Player::moveY(float dt, const std::vector<Block*>& blocks)
{
    sprite.move(0.0f, velocity.y * dt);
    updateCollider();
    
    isGrounded = false;
    
    for(const auto& block : blocks)
    {
        if(collider.intersects(block->getBounds()))
        {
            float overlap = 0.f;
            
            if(velocity.y > 0) // Cadendo
            {
                // Il fondo del collider è oltre la cima del blocco
                overlap = (collider.top + collider.height) - block->getBounds().top;
                sprite.move(0.f, -overlap);
                isGrounded = true;
            }
            else if(velocity.y < 0) // Saltando
            {
                // La cima del collider è oltre il fondo del blocco
                overlap = block->getBounds().top + block->getBounds().height - collider.top;
                sprite.move(0.f, overlap);
            }
            
            updateCollider();
            velocity.y = 0;
            break;
        }
    }
}

void Player::updateAnimation(float dt)
{
    // Cache
    static sf::Texture* lastTexture = nullptr;
    static bool lastFacingRight = true;
    
    // Determina texture
    sf::Texture* texture = &idle_texture;
    
    if(!isGrounded)
    {
        texture = (velocity.y < 0) ? &jump_textures[0] : &falling_texture;
    }
    else if(velocity.x != 0.f)
    {
        // Walking animation
        animation_timer += dt;
        if(animation_timer >= animation_speed)
        {
            animation_timer = 0.f;
            current_animation_frame = (current_animation_frame + 1) % walk_textures.size();
        }
        texture = &walk_textures[current_animation_frame];
    }
    
    // Cambia texture SOLO se necessario
    if(texture != lastTexture || facingRight != lastFacingRight)
    {
        sprite.setTexture(*texture);
        sprite.setTextureRect(sf::IntRect(41, 24, 15, 30));
        
        // Flip
        sprite.setScale(facingRight ? 1.f : -1.f, 1.f);
        
        // Update cache
        lastTexture = texture;
        lastFacingRight = facingRight;
    }
}

Player::PlayerState Player::getState() 
{
    if(!isGrounded)
    {
        if(velocity.y < 0)
            return PlayerState::jumping;
        else
            return PlayerState::falling;
    }
    else if(velocity.x != 0.f)
    {
        return PlayerState::walking;
    }
    else
    {
        return PlayerState::idle;
    }
}

void Player::updateCollider()
{
    // Semplicissimo: collider centrato sull'origine dello sprite
    collider.left = sprite.getPosition().x - (collider.width / 2.f);
    collider.top = sprite.getPosition().y - (collider.height / 2.f);
}

bool Player::isLocal()
{
    return localPlayer;
}

void Player::draw(sf::RenderWindow &window) 
{
    window.draw(sprite);
    /*
    // Disegna l'origine (punto rosso)
    sf::CircleShape originDot(3.f);
    originDot.setPosition(
        sprite.getPosition().x - 3.f,
        sprite.getPosition().y - 3.f
    );
    originDot.setFillColor(sf::Color::Red);
    window.draw(originDot);
    
    // Disegna il rettangolo del personaggio visibile
    sf::RectangleShape visibleRect;
    visibleRect.setPosition(
        sprite.getPosition().x - (collider.width / 2.f),
        sprite.getPosition().y - (collider.height / 2.f)
    );
    visibleRect.setSize(sf::Vector2f(collider.width, collider.height));
    visibleRect.setFillColor(sf::Color::Transparent);
    visibleRect.setOutlineColor(sf::Color::Green);
    visibleRect.setOutlineThickness(1.f);
    window.draw(visibleRect);
    
    // Disegna il collider
    sf::RectangleShape colliderRect;
    colliderRect.setPosition(collider.left, collider.top);
    colliderRect.setSize(sf::Vector2f(collider.width, collider.height));
    colliderRect.setFillColor(sf::Color(255, 0, 0, 100));
    colliderRect.setOutlineColor(sf::Color::Red);
    colliderRect.setOutlineThickness(2.f);
    window.draw(colliderRect);*/
}

void Player::update(const Scene& scene) 
{
    float dt = scene.getDt();
    auto blocks = scene.getBlocks();
    
    if (localPlayer) {
        handle_input();
        apply_gravity(scene.getDt());
        moveX(dt, blocks);
        moveY(dt, blocks);

        // Send movement packet to server
        if (localPlayer && NetworkClient::getInstance()->isConnected()) {
            PacketMove packet;
            packet.header.type = PacketType::MOVE;
            packet.playerId = this->id;
            packet.x = sprite.getPosition().x;
            packet.y = sprite.getPosition().y;
            packet.velocityX = velocity.x;
            packet.velocityY = velocity.y;
            packet.isFacingRight = facingRight;
            packet.isGrounded = isGrounded;

            NetworkClient::getInstance()->sendPacket(packet); // Spedisci!
        }
    }
    
    updateAnimation(dt);

    //each 60 frames print all the info about the player
    static int frameCounter = 0;
    frameCounter++;
    if(frameCounter >= 60)
    {
        frameCounter = 0;
        std::cout << "Player: " << playerName << " Position: (" << sprite.getPosition().x << ", " 
                    << sprite.getPosition().y << ") Velocity: (" << velocity.x << ", " << velocity.y << ") "  << std::endl;
        if(isGrounded)
            std::cout << "isGrounded" << std::endl;
        else   
            std::cout << "NotGrounded" << std::endl;
    }
}

// Sincronizza lo stato (la posizione, la velocità, ecc.) dei giocatori remoti dai dati di rete ricevuti
// Da chiamare quando arriva un PacketMove dal server per questo player
void Player::syncFromNetwork(float x, float y, float velX, float velY, bool faceRight, bool grounded)
{
    if (localPlayer)
        return; // Per essere sicuri la funzione non venga chiamata sul player locale

    sprite.setPosition(x, y); // Teletrasporto (più avanti si potrà fare interpolazione)
    velocity.x = velX;        // Serve per far funzionare updateAnimation()
    velocity.y = velY;
    facingRight = faceRight;
    isGrounded = grounded;
}