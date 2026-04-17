#pragma once
#include <SDL3/SDL.h>
#include <core/config.h>
#include <core/random/rng_service.h>
#include <entities/particle.h>
#include <entities/projectile.h>
#include <memory>
#include <plf/plf_colony.h>
#include <vector>
enum class Direction
{
    RIGHT,
    LEFT
};

class Player
{
  public:
    Player(float x, float y, float w, float h);
    ~Player() = default;

    void update(plf::colony<Particle>& particles, Random::RngEngine& fxRng);

    SDL_FRect getBounds() const;
    SDL_FPoint getFrontCenter() const;
    void setPosition(float x, float y);

    plf::colony<Projectile>& getProjectiles();
    const plf::colony<Projectile>& getProjectiles() const;

    void shoot();
    bool isAlive() const
    {
        return m_health > 0;
    }
    void takeDamage(int damage)
    {
        m_health -= damage;
        if (m_health < 0)
            m_health = 0;
    }
    int getHealth() const
    {
        return m_health;
    }
    int getMaxHealth() const
    {
        return m_maxHealth;
    }
    void restoreHealth()
    {
        m_health = getMaxHealth();
    }
    void setSpeedBoost(bool active);

    float getSpeed() const
    {
        return m_speed;
    }
    void setFacing(Direction dir)
    {
        m_facing = dir;
    }
    Direction getFacing() const
    {
        return m_facing;
    }
    void moveBy(float dx, float dy)
    {
        m_rect.x += dx;
        m_rect.y += dy;
    }

  private:
    SDL_FRect m_rect;
    float m_speed;
    Direction m_facing;

    plf::colony<Projectile> m_projectiles;

    int m_maxHealth;
    int m_health;
    float m_normalSpeed;
    float m_boostMultiplier;
    bool m_speedBoostActive;

    void spawnBoosterParticles(plf::colony<Particle>& particles, Random::RngEngine& fxRng);
    void spawnDefaultBoosterParticles(plf::colony<Particle>& particles, Random::RngEngine& fxRng);
};
