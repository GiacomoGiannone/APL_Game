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
    Hittable(float maxHealth = 100.f)
        : maxHealth(maxHealth), currentHealth(maxHealth), dead(false),
          dying(false), deathTimer(0.f)
    {}

    virtual ~Hittable() = default;

    virtual void takeDamage(float amount)
    {
        if (dead || dying) return;
        
        currentHealth -= amount;
        if (currentHealth <= 0.f)
        {
            currentHealth = 0.f;
            dying = true;
            onDeath();
        }
    }

    virtual void heal(float amount)
    {
        if (dead || dying) return;
        
        currentHealth += amount;
        if (currentHealth > maxHealth)
        {
            currentHealth = maxHealth;
        }
    }

    // Resetta completamente la salute (per respawn)
    virtual void resetHealth()
    {
        currentHealth = maxHealth;
        dead = false;
        dying = false;
        deathTimer = 0.f;
    }

    // Chiama questo nel update() delle classi derivate
    // Ritorna true se l'entità deve essere rimossa
    bool updateDeath(float dt)
    {
        if (!dying) return false;
        
        deathTimer += dt;
        if (deathTimer >= deathDuration)
        {
            dead = true;
            return true; // Segnala che deve essere rimosso
        }
        return false;
    }

    virtual void onDeath() 
    {
        // Override nelle classi derivate per comportamento specifico
    }

    bool isDead() const { return dead; }
    bool isDying() const { return dying; }
    float getDeathProgress() const { return dying ? (deathTimer / deathDuration) : 0.f; }
    float getHealth() const { return currentHealth; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercent() const { return currentHealth / maxHealth; }
};
