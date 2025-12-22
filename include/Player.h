#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>
#include <string>
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
        sf::Texture idle_texture;
        std::vector<sf::Texture> jump_textures;
        sf::Texture falling_texture;

        //we need to add a rectangle for the player collisions
        float colliderOffsetX;
        float colliderOffsetY;
        sf::FloatRect collider;

        int current_animation_frame;
        float animation_timer, animation_speed;
        
        bool isGrounded;
        float speed, gravity;
        bool facingRight;
        bool localPlayer;

        std::string playerName;
        int id; // max 255 giocatori

        void handle_input();
        void apply_gravity(float dt);
        void moveX(float dt, const std::vector<Block*>& blocks);
        void moveY(float dt, const std::vector<Block*>& blocks);
        void updateAnimation(float dt);
        void updateCollider();
    public:
        Player(std::string texturePathFolder, std::string playerName, bool localPlayer);
        void update(const Scene& scene) override;
        void draw(sf::RenderWindow &window) override;
        void syncFromNetwork(float x, float y, float velX, float velY, bool faceRight, bool grounded);
        int getId() const;
        void setId(int newId);

        enum class PlayerState
        {
            idle,
            walking,
            jumping,
            falling
        };

        bool isLocal();

    private:
        PlayerState state = PlayerState::idle;
        PlayerState getState();
};