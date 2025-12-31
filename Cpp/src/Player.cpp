#include "Player.h"
#include "Block.h"
#include "Scene.h"
#include "Game.h"
#include "NetMessages.h"
#include "NetworkClient.h"
#include "Enemy.h"
#include <iostream>

Player::Player(std::string Folder, std::string playerName, bool localPlayer)
    : Hittable(100.f), velocity(0.0f, 0.0f), isGrounded(false), speed(200.0f), gravity(200.0f),
      current_animation_frame(0), animation_timer(0.1f), animation_speed(0.1f),
      playerName(playerName), facingRight(true), localPlayer(localPlayer), folder(Folder),
      isAttacking(false), attackFrame(0), attackTimer(0.f), attackCooldownTimer(0.f),
      lastTexture(nullptr), lastFacingRight(true)
{
    // Carica texture idle
    std::string path_to_texture = "assets/pp1/" + Folder + "/Idle.png";
    if(!idle_texture.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load idle texture from path " << path_to_texture << std::endl;
    }
    
    // Carica texture walk
    sf::Texture text;
    for(int i = 1; i <= 4; i++)
    {
        path_to_texture = "assets/pp1/" + Folder + "/Walk_" + std::to_string(i) + ".png";
        if(!text.loadFromFile(path_to_texture))
        {
            std::cerr << "Could not load walk texture from path " << path_to_texture << std::endl;
        }
        walk_textures.push_back(text);
    }
    
    // Carica texture djump
    path_to_texture = "assets/pp1/" + Folder + "/Jump_1.png";
    if(!text.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load jump texture from path " << path_to_texture << std::endl;
    }
    jump_textures.push_back(text);
    
    path_to_texture = "assets/pp1/" + Folder + "/Jump_2.png";
    if(!text.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load jump texture from path " << path_to_texture << std::endl;
    }
    jump_textures.push_back(text);
    
    // Carica texture falling
    path_to_texture = "assets/pp1/" + Folder + "/Fall.png";
    if(!falling_texture.loadFromFile(path_to_texture))
    {
        std::cerr << "Could not load falling texture from path " << path_to_texture << std::endl;
    }

    //carica texture di attacco
    if(!text.loadFromFile("assets/pp1/" + Folder + "/A1.png"))
    {
        std::cerr << "Could not load attack texture from path " << "assets/pp1/" + Folder + "/A1.png" << std::endl;
    }
    attack_textures.push_back(text);
    if(!text.loadFromFile("assets/pp1/" + Folder + "/A2.png"))
    {
        std::cerr << "Could not load attack texture from path " << "assets/pp1/" + Folder + "/A2.png" << std::endl;
    }
    attack_textures.push_back(text);
    
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

void Player::handle_input(const Scene& scene)
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
    //check left click for attack
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && attackCooldownTimer <= 0.f)
    {
        attack(scene);
        attackCooldownTimer = attackCooldown; // Reset cooldown
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
    // Cache per ottimizzare cambi texture (non static, ogni player ha la sua)
    
    // Handle attack animation first (priority over other animations)
    if (isAttacking)
    {
        attackTimer += dt;
        if (attackTimer >= attackFrameDuration)
        {
            attackTimer = 0.f;
            attackFrame++;
            if (attackFrame >= attack_textures.size())
            {
                // Attack animation finished, return to normal
                isAttacking = false;
                attackFrame = 0;
                // Force cache invalidation so idle texture gets applied
                lastTexture = nullptr;
            }
        }
        
        if (isAttacking) // Still attacking
        {
            sprite.setTexture(attack_textures[attackFrame]);
            sprite.setTextureRect(sf::IntRect(41, 24, 40, 30));
            sprite.setScale(facingRight ? 1.f : -1.f, 1.f);
            return; // Don't process other animations
        }
    }
    
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
    // Fade out durante la morte
    if (dying)
    {
        float alpha = 255.f * (1.f - getDeathProgress());
        sprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
    }
    
    window.draw(sprite);
    
    // Non disegnare health bar se sta morendo
    if (dying) return;
    
    // Health bar
    float barWidth = 30.f;
    float barHeight = 4.f;
    float barOffsetY = -20.f; // Sopra la testa
    
    // Background (rosso)
    sf::RectangleShape healthBarBg;
    healthBarBg.setSize(sf::Vector2f(barWidth, barHeight));
    healthBarBg.setPosition(sprite.getPosition().x - barWidth / 2.f, 
                            sprite.getPosition().y + barOffsetY);
    healthBarBg.setFillColor(sf::Color(60, 60, 60));
    healthBarBg.setOutlineColor(sf::Color::Black);
    healthBarBg.setOutlineThickness(1.f);
    window.draw(healthBarBg);
    
    // Foreground (verde -> giallo -> rosso in base alla salute)
    sf::RectangleShape healthBar;
    float healthPercent = getHealthPercent();
    healthBar.setSize(sf::Vector2f(barWidth * healthPercent, barHeight));
    healthBar.setPosition(sprite.getPosition().x - barWidth / 2.f, 
                          sprite.getPosition().y + barOffsetY);
    
    // Colore in base alla percentuale
    if (healthPercent > 0.6f)
        healthBar.setFillColor(sf::Color(50, 205, 50)); // Verde
    else if (healthPercent > 0.3f)
        healthBar.setFillColor(sf::Color(255, 165, 0)); // Arancione
    else
        healthBar.setFillColor(sf::Color(220, 20, 60)); // Rosso
    
    window.draw(healthBar);
    
    // Draw attack hitbox when attacking
    if (isAttacking)
    {
        sf::RectangleShape hitboxRect;
        hitboxRect.setPosition(attackHitbox.left, attackHitbox.top);
        hitboxRect.setSize(sf::Vector2f(attackHitbox.width, attackHitbox.height));
        hitboxRect.setFillColor(sf::Color(255, 0, 0, 100));
        hitboxRect.setOutlineColor(sf::Color::Red);
        hitboxRect.setOutlineThickness(2.f);
        window.draw(hitboxRect);
    }
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

void Player::attack(const Scene& scene)
{
    // Calculate attack hitbox position
    attackHitbox.width = 20.f;
    attackHitbox.height = 20.f;
    if(facingRight)
    {
        attackHitbox.left = sprite.getPosition().x + collider.width / 2.f;
        attackHitbox.top = sprite.getPosition().y - 10.f;
    }
    else
    {
        attackHitbox.left = sprite.getPosition().x - collider.width / 2.f - 20.f;
        attackHitbox.top = sprite.getPosition().y - 10.f;
    }

    // Invia pacchetto attacco per sincronizzare l'animazione con gli altri client
    if (NetworkClient::getInstance()->isConnected())
    {
        PacketPlayerAttack attackPacket;
        attackPacket.header.type = PacketType::PLAYER_ATTACK;
        attackPacket.header.packetSize = sizeof(PacketPlayerAttack);
        attackPacket.playerId = this->id;
        attackPacket.x = sprite.getPosition().x;
        attackPacket.y = sprite.getPosition().y;
        attackPacket.isFacingRight = facingRight ? 1 : 0;
        memset(attackPacket.padding, 0, sizeof(attackPacket.padding));
        
        NetworkClient::getInstance()->sendPacket(attackPacket);
    }

    //we need to check if the attack hitbox intersects with any other entities in the scene
    for(const auto& player : scene.getPlayers())
    {
        if(player->getId() != this->id) //don't attack yourself
        {
            if(attackHitbox.intersects(player->collider))
            {
                std::cout << "Player " << playerName << " attacked Player " << player->playerName << "!" << std::endl;
                //here you can apply damage or any other effect to the attacked player
            }
        }
    }

    for(const auto& enemy : scene.getEnemies())
    {
        if(attackHitbox.intersects(enemy->getBounds()))
        {
            std::cout << "Player " << playerName << " attacked an Enemy!" << std::endl;
            
            // Applica danno localmente
            enemy->takeDamage(25.f);
            
            // Invia pacchetto danno al server per sincronizzare con altri client
            if (NetworkClient::getInstance()->isConnected())
            {
                PacketEnemyDamage damagePacket;
                damagePacket.header.type = PacketType::ENEMY_DAMAGE;
                damagePacket.header.packetSize = sizeof(PacketEnemyDamage);
                damagePacket.enemyId = enemy->getId();
                damagePacket.attackerId = this->id;
                damagePacket.damage = 25.f;
                
                NetworkClient::getInstance()->sendPacket(damagePacket);
            }
        }
    }
    setAttackAnimation();
}

void Player::setAttackAnimation()
{
    // Start attack if not already attacking
    if (!isAttacking)
    {
        isAttacking = true;
        attackFrame = 0;
        attackTimer = 0.f;
    }
}

void Player::update(const Scene& scene) 
{
    float dt = scene.getDt();
    
    // Gestione morte
    if (dying)
    {
        sprite.setRotation(90.f);
        updateDeath(dt);
        return;
    }
    
    auto blocks = scene.getBlocks();
    
    // Update attack cooldown
    if (attackCooldownTimer > 0.f)
    {
        attackCooldownTimer -= dt;
    }
    
    if (localPlayer) {
        handle_input(scene);
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
}

// Sincronizza lo stato (la posizione, la velocità, ecc.) dei giocatori remoti dai dati di rete ricevuti
// Da chiamare quando arriva un PacketMove dal server per questo player
void Player::syncFromNetwork(float x, float y, float velX, float velY, bool faceRight, bool grounded)
{
    if (localPlayer)
        return; // Per essere sicuri la funzione non venga chiamata sul player locale

    sprite.setPosition(x, y); // Teletrasporto (più avanti si potrà fare interpolazione)
    updateCollider();         // IMPORTANTE: aggiorna il collider dopo aver spostato lo sprite!
    velocity.x = velX;        // Serve per far funzionare updateAnimation()
    velocity.y = velY;
    facingRight = faceRight;
    isGrounded = grounded;
}

// Respawn del player locale alla posizione iniziale
void Player::respawn()
{
    // Reset posizione
    sprite.setPosition(100.f, 100.f);
    updateCollider();
    
    // Reset velocità
    velocity.x = 0.f;
    velocity.y = 0.f;
    
    // Reset stato
    facingRight = true;
    isGrounded = false;
    isAttacking = false;
    attackFrame = 0;
    attackTimer = 0.f;
    attackCooldownTimer = 0.f;
    
    // Reset rotazione sprite (era ruotato durante la morte)
    sprite.setRotation(0.f);
    
    // Reset colore sprite (era fadato durante la morte)
    sprite.setColor(sf::Color::White);
    
    // Reset texture all'idle
    sprite.setTexture(idle_texture);
    sprite.setTextureRect(sf::IntRect(41, 24, 15, 30));
    sprite.setOrigin(15.f / 2.f, 30.f / 2.f);
    
    std::cout << "Player respawnato!" << std::endl;
}

// Attiva l'animazione di attacco (chiamato dalla rete per player remoti)
void Player::triggerAttackAnimation()
{
    setAttackAnimation();
}

// Override di takeDamage per inviare il danno via rete
void Player::takeDamage(float amount)
{
    if (dead || dying) return;
    
    // Applica il danno localmente
    currentHealth -= amount;
    std::cout << "[DANNO] Player " << id << " (" << playerName << ") - Salute: " << currentHealth << "/" << maxHealth << std::endl;
    
    if (currentHealth <= 0.f)
    {
        currentHealth = 0.f;
        dying = true;
        onDeath();
    }
    
    // Se siamo il player locale, invia il danno agli altri client
    if (localPlayer && NetworkClient::getInstance()->isConnected())
    {
        PacketPlayerDamage damagePacket;
        damagePacket.header.type = PacketType::PLAYER_DAMAGE;
        damagePacket.header.packetSize = sizeof(PacketPlayerDamage);
        damagePacket.playerId = this->id;
        damagePacket.damage = amount;
        damagePacket.currentHealth = currentHealth;
        
        NetworkClient::getInstance()->sendPacket(damagePacket);
    }
}

// Riceve danno sincronizzato dalla rete (per player remoti)
void Player::syncDamageFromNetwork(float damage, float health)
{
    if (localPlayer) return; // Non applicare a noi stessi
    
    currentHealth = health;
    std::cout << "[DANNO REMOTO] Player " << id << " - Salute: " << currentHealth << "/" << maxHealth << std::endl;
    
    if (currentHealth <= 0.f && !dying)
    {
        currentHealth = 0.f;
        dying = true;
        onDeath();
    }
}

// Applica danno ricevuto dall'host (per il player locale, senza re-inviare)
void Player::applyDamageFromHost(float damage)
{
    if (dead || dying) return;
    
    currentHealth -= damage;
    std::cout << "[DANNO DA HOST] Player " << id << " (" << playerName << ") - Salute: " << currentHealth << "/" << maxHealth << std::endl;
    
    if (currentHealth <= 0.f)
    {
        currentHealth = 0.f;
        dying = true;
        onDeath();
    }
    
    // NON inviamo pacchetto - l'host ce l'ha già detto lui
}