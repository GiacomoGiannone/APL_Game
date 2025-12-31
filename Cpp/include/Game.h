#pragma once

#include <SFML/Graphics.hpp>

//#include "GameObject.h"

class Scene;

class Game
{   
    //Implements the singleton pattern
    private:
        Scene* currentScene;
        static Game* instance;
        sf::RenderWindow* window;
        Game(sf::RenderWindow* window);
        int localPlayerId;
        bool isWindowFocused;
        
        // Sistema di gioco
        int enemiesToDefeat;
        bool gameWon;
        bool levelComplete;
        bool gameOver;
        int currentLevel;
        bool isHost;  // True se siamo l'host (controlliamo i nemici)
        sf::Font gameFont;
        sf::Text enemyCountText;
        sf::Text gameOverText;
        sf::Text levelText;
        sf::Text restartText;
        sf::Text hostText;

    public:
        static Game* getInstance(sf::RenderWindow* window = nullptr);
        void update(float dt);
        void setScene(Scene* newScene);
        void setLocalPlayerId(int id);
        int getLocalPlayerId() const;
        void setFocus(bool focus);
        bool hasFocus() const;
        sf::RenderWindow& getWindow() const;
        
        // Sistema conteggio nemici
        void setEnemiesToDefeat(int count);
        void incrementEnemiesToDefeat(); // Per client che ricevono spawn
        int getEnemiesToDefeat() const;
        void enemyDefeated();
        bool isGameWon() const;
        void drawUI();
        
        // Sistema livelli
        int getCurrentLevel() const;
        void nextLevel();
        bool isLevelComplete() const;
        void resetLevelComplete();
        
        // Game Over
        void setGameOver();
        bool isGameOver() const;
        void restartGame();
        Scene* getCurrentScene() const { return currentScene; }
        
        // Host system
        void setIsHost(bool host);
        bool getIsHost() const;
};