#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "GameObject.h"

class Block;
class Scene;

class Player: public GameObject
{
    private:
        //no point in saving x and y
        //Sprite holds 
        sf::Sprite sprite;
        sf::Vector2f velocity;
        std::vector<sf::Texture> walk_textures;

        int current_animation_frame;
        float animation_timer, animation_speed;
        
        bool isGrounded;
        float speed, gravity;

        std::string playerName;

        void handle_input();
        void apply_gravity(float dt);
        void moveX(float dt, const std::vector<Block*>& blocks);
        void moveY(float dt, const std::vector<Block*>& blocks);
        void updateAnimation(float dt);
    public:
        Player(std::string texturePathFolder, std::string playerName);
        void update(const Scene& scene) override;
        void draw(sf::RenderWindow &window) override;
};