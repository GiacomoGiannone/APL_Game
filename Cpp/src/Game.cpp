#include <iostream>

#include "Game.h"
#include "Scene.h"

// Inizializzazione membro statico
Game* Game::instance = nullptr;

Game* Game::getInstance(sf::RenderWindow* window)
{
    if(instance == nullptr)
    {
        if (window != nullptr) {
            instance = new Game(window);
        } else {
            std::cerr << "ERRORE: Tentativo di accedere a Game::getInstance() prima dell'inizializzazione!" << std::endl;
        }
    }
    return instance; 
}

Game::Game(sf::RenderWindow* window) : window(window), enemiesToDefeat(0), gameWon(false), levelComplete(false), gameOver(false), currentLevel(1), isHost(false)
{
    // Carica il font per l'UI (prova diversi percorsi)
    bool fontLoaded = false;
    
    // Prima prova un font locale
    if (!fontLoaded && gameFont.loadFromFile("assets/pp1/miscellaneous/font.ttf")) {
        fontLoaded = true;
    }
    // Poi prova i font di sistema Windows
    if (!fontLoaded && gameFont.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        fontLoaded = true;
    }
    if (!fontLoaded && gameFont.loadFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        fontLoaded = true;
    }
    
    if (!fontLoaded) {
        std::cerr << "ATTENZIONE: Impossibile caricare nessun font!" << std::endl;
    }
    
    // Setup testo contatore nemici
    enemyCountText.setFont(gameFont);
    enemyCountText.setCharacterSize(24);
    enemyCountText.setFillColor(sf::Color::Black);
    enemyCountText.setOutlineColor(sf::Color::White);
    enemyCountText.setOutlineThickness(2.f);
    enemyCountText.setPosition(10.f, 10.f);
    
    // Setup testo livello
    levelText.setFont(gameFont);
    levelText.setCharacterSize(24);
    levelText.setFillColor(sf::Color::Black);
    levelText.setOutlineColor(sf::Color::White);
    levelText.setOutlineThickness(2.f);
    levelText.setPosition(10.f, 40.f);
    levelText.setString("Livello: 1");
    
    // Setup testo vittoria
    gameOverText.setFont(gameFont);
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Green);
    gameOverText.setOutlineColor(sf::Color::Black);
    gameOverText.setOutlineThickness(3.f);
    gameOverText.setString("HAI VINTO!");
    gameOverText.setPosition(280.f, 250.f);
    
    // Setup testo restart
    restartText.setFont(gameFont);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setOutlineColor(sf::Color::Black);
    restartText.setOutlineThickness(2.f);
    restartText.setString("Premi R per ricominciare");
    restartText.setPosition(280.f, 320.f);
    
    // Setup testo host
    hostText.setFont(gameFont);
    hostText.setCharacterSize(18);
    hostText.setFillColor(sf::Color::Yellow);
    hostText.setOutlineColor(sf::Color::Black);
    hostText.setOutlineThickness(1.f);
    hostText.setPosition(650.f, 10.f);
}

void Game::update(float dt)
{
    // Se il gioco è finito (vinto o perso), non aggiornare più
    if (gameWon || gameOver) return;
    
    if (currentScene != nullptr) 
    {
        currentScene->setDt(dt);
        currentScene->update();
    }
}

void Game::setScene(Scene* newScene)
{
    currentScene = newScene;
}

void Game::setLocalPlayerId(int id) {
    this->localPlayerId = id;
}

int Game::getLocalPlayerId() const {
    return this->localPlayerId;
}

void Game::setFocus(bool focus) {
    this->isWindowFocused = focus;
}

bool Game::hasFocus() const {
    return this->isWindowFocused;
}

sf::RenderWindow& Game::getWindow() const {
    return *window;
}

void Game::setEnemiesToDefeat(int count) {
    enemiesToDefeat = count;
    enemyCountText.setString("Nemici: " + std::to_string(enemiesToDefeat));
}

void Game::incrementEnemiesToDefeat() {
    enemiesToDefeat++;
    enemyCountText.setString("Nemici: " + std::to_string(enemiesToDefeat));
}

int Game::getEnemiesToDefeat() const {
    return enemiesToDefeat;
}

void Game::enemyDefeated() {
    if (enemiesToDefeat > 0) {
        enemiesToDefeat--;
        enemyCountText.setString("Nemici: " + std::to_string(enemiesToDefeat));
        
        if (enemiesToDefeat == 0) {
            levelComplete = true;
            std::cout << "LIVELLO " << currentLevel << " COMPLETATO!" << std::endl;
        }
    }
}

bool Game::isGameWon() const {
    return gameWon;
}

void Game::drawUI() {
    window->draw(enemyCountText);
    window->draw(levelText);
    
    // Mostra se siamo host
    if (isHost) {
        hostText.setString("[HOST]");
        window->draw(hostText);
    }
    
    if (gameOver) {
        gameOverText.setString("GAME OVER");
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setPosition(270.f, 250.f);
        window->draw(gameOverText);
        window->draw(restartText);
    }
    else if (gameWon) {
        window->draw(gameOverText);
    }
}

int Game::getCurrentLevel() const {
    return currentLevel;
}

void Game::nextLevel() {
    currentLevel++;
    levelComplete = false;
    levelText.setString("Livello: " + std::to_string(currentLevel));
    std::cout << "Inizia il LIVELLO " << currentLevel << "!" << std::endl;
}

bool Game::isLevelComplete() const {
    return levelComplete;
}

void Game::resetLevelComplete() {
    levelComplete = false;
}

void Game::setGameOver() {
    gameOver = true;
    std::cout << "GAME OVER! Hai raggiunto il livello " << currentLevel << std::endl;
}

bool Game::isGameOver() const {
    return gameOver;
}

void Game::restartGame() {
    gameOver = false;
    gameWon = false;
    levelComplete = false;
    currentLevel = 1;
    levelText.setString("Livello: 1");
    std::cout << "Gioco riavviato!" << std::endl;
}

void Game::setIsHost(bool host) {
    isHost = host;
    if (currentScene) {
        currentScene->setIsHost(host);  // Propaga alla scena
    }
    if (host) {
        std::cout << "Sei l'HOST - Controlli i nemici!" << std::endl;
    }
}

bool Game::getIsHost() const {
    return isHost;
}