#include "Player.h"
#include "Block.h"
#include "Scene.h"
#include <iostream>

Player::Player(std::string texturePathFolder, std::string playerName):velocity(0.0f, 0.0f), isGrounded(false), speed(100.0f), gravity(200.0f), 
                                    current_animation_frame(0), animation_timer(0.1f), animation_speed(0.1f), playerName(playerName)
{
    //load character texture into ram only once at character creation
    sf::Texture text;
    std::string genericPath;
    for(int i = 0; i < 22; i++)
    {
        if(i  < 10)
        {
            genericPath = "metarig.004-0_000" + std::to_string(i) + ".png";
        }
        else
        {
            genericPath = "metarig.004-0_00" + std::to_string(i) + ".png";
        }
        std::string fullPath = texturePathFolder + "/" + genericPath;
        if(!text.loadFromFile(fullPath))
        {
            std::cerr << "Could not load texture from path " << fullPath << std::endl;
        }
        walk_textures.push_back(std::move(text));
    }
    if(!walk_textures.empty())
        sprite.setTexture(walk_textures[0]);

    sprite.setPosition(100.f, 100.f);
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
        velocity.y = -10.0f;
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

void Player::draw(sf::RenderWindow &window) 
{
    window.draw(sprite);
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