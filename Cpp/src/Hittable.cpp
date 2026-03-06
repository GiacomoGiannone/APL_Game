#include "Hittable.h"

Hittable::Hittable(float maxHealth)
    : maxHealth(maxHealth), currentHealth(maxHealth), dead(false),
      dying(false), deathTimer(0.f)
{}

void Hittable::takeDamage(float amount)
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

void Hittable::heal(float amount)
{
    if (dead || dying) return;
    
    currentHealth += amount;
    if (currentHealth > maxHealth)
    {
        currentHealth = maxHealth;
    }
}

void Hittable::resetHealth()
{
    currentHealth = maxHealth;
    dead = false;
    dying = false;
    deathTimer = 0.f;
}

bool Hittable::updateDeath(float dt)
{
    if (!dying) return false;
    
    deathTimer += dt;
    if (deathTimer >= deathDuration)
    {
        dead = true;
        return true;
    }
    return false;
}

void Hittable::onDeath()
{
    // Override nelle classi derivate per comportamento specifico
}
