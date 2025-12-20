#include "Player.h"
#include "Block.h"
#include "Scene.h"
#include <iostream>

Player::Player(std::string Folder, std::string playerName)
    : velocity(0.0f, 0.0f), isGrounded(false), speed(200.0f), gravity(200.0f),
      current_animation_frame(0), animation_timer(0.1f), animation_speed(0.1f),
      playerName(playerName), facingRight(true)
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
    
    // Carica texture jump
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
    
    // Imposta origine al centro per flip corretto
    sprite.setPosition(100.f, 100.f);
    
    // Setup collider (più piccolo del personaggio
    //il personaggio e' centrato con la texture
    //dobbiamo ritagliare il collider per ottenere la dimensione giusta
    float spriteWidth = sprite.getLocalBounds().width;
    float spriteHeight = sprite.getLocalBounds().height;
    
    // Dimensioni del collider (adatta queste al tuo personaggio)
    collider.width = 15.f;   // Larghezza del collider
    collider.height = 30.f;  // Altezza del collider
    
    // Calcola offset per centrare il collider
    // Il collider è centrato orizzontalmente e in basso verticalmente
    colliderOffsetX = (spriteWidth - collider.width) / 2.f;
    colliderOffsetY = spriteHeight - collider.height; // Collider nella parte bassa
    
    // Inizializza la posizione del collider
    updateCollider();
}

void Player::handle_input()
{
    velocity.x = 0.0f;
    //check if player wants to go to the left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        velocity.x -= speed;
    }
    //check if player wants to go to the right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        velocity.x += speed;
    }
    //check if player wants to jump
    if(isGrounded && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
        //we will adjust gravity later in the applyGravity function
        //if not the player would keep flying
        velocity.y = -200.0f;
    }
}

void Player::apply_gravity(float dt)
{
    velocity.y += gravity * dt;
}

void Player::moveX(float dt, const std::vector<Block*>& blocks)
{
    //move the sprite object with velocity
    sprite.move(velocity.x * dt, 0.0f);
    //now we check the collision on this axis
    //first we obtain the player bounds
    auto playerBounds = sprite.getGlobalBounds();

    //now we need to check for all the blocks in the blocks vector
    //(cant we do the check only on nearby blocks?)
    for(const auto& block : blocks)
    {
        if(playerBounds.intersects(block->getBounds()))
        {
            //collision on X axis detected, we need to "block" the player on this axis
            if(velocity.x > 0)
            {
                //player is trying to go through the block
                //we prevent them from doing so by setting the position to the last valid position
                sprite.setPosition(block->getBounds().left - playerBounds.width, sprite.getPosition().y);
            }
        }
    }
    updateCollider();
}

void Player::moveY(float dt, const std::vector<Block*>& blocks)
{
    //move the sprite object with velocity
    sprite.move(0.0f, velocity.y * dt);
    //now we check the collision on this axis
    //first we obtain the player bounds
    auto playerBounds = sprite.getGlobalBounds();
    isGrounded = false;

    //now we need to check for all the blocks in the blocks vector
    //(cant we do the check only on nearby blocks?)
    for(const auto& block : blocks)
    {
        if(playerBounds.intersects(block->getBounds()))
        {
            //collision on X axis detected, we need to "block" the player on this axis
            if(velocity.y > 0)
            {
                //player is trying to go through the block from the low side (head impacting on the block)
                //we prevent them from doing so by setting the position to the last valid position
                sprite.setPosition(sprite.getPosition().x, block->getBounds().top - playerBounds.height);
                isGrounded = true;
            }
            else
            {
                //player is simply walking on the block
                sprite.setPosition(sprite.getPosition().x, block->getBounds().top + block->getBounds().height);
            }
            velocity.y = 0;
            playerBounds = sprite.getGlobalBounds();
        }
    }
    updateCollider();
}

void Player::updateAnimation(float dt)
{
    if (velocity.x == 0.f) 
        return;

    animation_timer += dt;
    if (animation_timer >= animation_speed)
    {
        animation_timer = 0.f;
        current_animation_frame = (current_animation_frame + 1) % walk_textures.size();
        sprite.setTexture(walk_textures[current_animation_frame]);
    }
}

void Player::updateCollider()
{
    // Aggiorna la posizione del collider in base alla posizione dello sprite
    collider.left = sprite.getPosition().x + colliderOffsetX;
    collider.top = sprite.getPosition().y + colliderOffsetY;
}

void Player::draw(sf::RenderWindow &window) 
{
    window.draw(sprite);
    sf::RectangleShape debugRect;
    debugRect.setPosition(collider.left, collider.top);
    debugRect.setSize(sf::Vector2f(collider.width, collider.height));
    debugRect.setFillColor(sf::Color(255, 0, 0, 100)); // Rosso semitrasparente
    debugRect.setOutlineColor(sf::Color::Red);
    debugRect.setOutlineThickness(1.f);
    window.draw(debugRect);
}

void Player::update(const Scene& scene) 
{
    float dt = scene.getDt();
    auto blocks = scene.getBlocks();
    handle_input();
    apply_gravity(scene.getDt());
    moveX(dt, blocks);
    moveY(dt, blocks);
    updateAnimation(dt);

    //each 500 frames print all the info about the player
    static int frameCounter = 0;
    frameCounter++;
    if(frameCounter >= 500)
    {
        frameCounter = 0;
        std::cout << "Player: " << playerName << " Position: (" << sprite.getPosition().x << ", " 
                    << sprite.getPosition().y << ") Velocity: (" << velocity.x << ", " << velocity.y << ") " 
                     << isGrounded << std::endl;
    }
}