#pragma once

#include "GameObject.h"

class Hittable : public GameObject
{
protected:
    float maxHealth;
    float currentHealth;
    bool dead;
    
    // Death animation
    bool dying;
    float deathTimer;
    static constexpr float deathDuration = 2.0f;

public:
    Hittable(float maxHealth = 100.f);
    virtual ~Hittable() = default;

    virtual void takeDamage(float amount);
    virtual void heal(float amount);
    
    // Resetta completamente la salute (per respawn)
    virtual void resetHealth();

    // Chiama questo nel update() delle classi derivate
    // Ritorna true se l'entità deve essere rimossa
    bool updateDeath(float dt);

    virtual void onDeath();

    bool isDead() const { return dead; }
    bool isDying() const { return dying; }
    float getDeathProgress() const { return dying ? (deathTimer / deathDuration) : 0.f; }
    float getHealth() const { return currentHealth; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return currentHealth / maxHealth; }
};
