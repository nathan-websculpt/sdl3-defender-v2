#include "entities/player.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>

#include "core/config.h"
#include "core/managers/sound_manager.h"
#include "core/managers/texture_manager.h"
#include "entities/particle.h"

namespace {

Uint8 toUint8Clamped(int value) {
    const int clamped = std::clamp(value, 0, 255);
    return static_cast<Uint8>(clamped);
}

} // namespace

Player::Player(float x, float y, float w, float h)
    : m_rect{x, y, w, h}, m_normalSpeed(220.0f), m_speedBoostActive(false), m_boostMultiplier(2.1f),
      m_facing(Direction::RIGHT), m_maxHealth(10) {
    m_speed = m_normalSpeed;
    m_health = m_maxHealth;
}

void Player::update(plf::colony<Particle>& particles, Random::RngEngine& fxRng) {
    spawnDefaultBoosterParticles(particles, fxRng);

    if (m_speedBoostActive)
        spawnBoosterParticles(particles, fxRng);
}

SDL_FRect Player::getBounds() const {
    return m_rect;
}

SDL_FPoint Player::getFrontCenter() const {
    SDL_FPoint frontCenter;
    if (m_facing == Direction::RIGHT) {
        frontCenter.x = m_rect.x + m_rect.w;
        frontCenter.y = m_rect.y + m_rect.h / 2.0f;
    } else {
        frontCenter.x = m_rect.x;
        frontCenter.y = m_rect.y + m_rect.h / 2.0f;
    }

    return frontCenter;
}

void Player::setPosition(float x, float y) {
    m_rect.x = x;
    m_rect.y = y;
}

plf::colony<Projectile>& Player::getProjectiles() {
    return m_projectiles;
}

const plf::colony<Projectile>& Player::getProjectiles() const {
    return m_projectiles;
}

void Player::shoot() {
    constexpr float baseShotSpeed = 2040.0f;
    SDL_FPoint spawn = getFrontCenter();
    float dir = (m_facing == Direction::RIGHT) ? 1.0f : -1.0f;
    float shotSpeed = baseShotSpeed;
    if (m_speedBoostActive) {
        shotSpeed += (m_speed - m_normalSpeed);
    }
    m_projectiles.emplace(spawn.x, spawn.y, dir, shotSpeed);

    SoundManager::getInstance().playSound(Config::Sounds::PLAYER_SHOOT);
}

void Player::setSpeedBoost(bool active) {
    m_speedBoostActive = active;
    m_speed = active ? m_normalSpeed * m_boostMultiplier : m_normalSpeed;
}

void Player::spawnBoosterParticles(plf::colony<Particle>& particles, Random::RngEngine& fxRng) {
    if (!m_speedBoostActive)
        return;

    SDL_FPoint rearCenter = getFrontCenter();
    if (m_facing == Direction::RIGHT)
        rearCenter.x = m_rect.x;
    else
        rearCenter.x = m_rect.x + m_rect.w;

    const int numParticles = 12;
    for (int i = 0; i < numParticles; ++i) {
        // random offset within a 12-unit wide by 22-unit tall rectangle centered on rearCenter
        const int spawnOffsetX = Random::randomIntInclusive(fxRng, 0, 11) - 6;
        const int spawnOffsetY = Random::randomIntInclusive(fxRng, 0, 21) - 11;
        float spawnX = rearCenter.x + static_cast<float>(spawnOffsetX);
        float spawnY = rearCenter.y + static_cast<float>(spawnOffsetY);

        float velX = (m_facing == Direction::RIGHT) ? -100.0f : 100.0f;
        velX += static_cast<float>(Random::randomIntInclusive(fxRng, 0, 39) - 20);
        float velY = static_cast<float>(Random::randomIntInclusive(fxRng, 0, 39) - 20);

        Uint8 r = 255;
        Uint8 g = toUint8Clamped(Random::randomIntInclusive(fxRng, 100, 199));
        Uint8 b = toUint8Clamped(Random::randomIntInclusive(fxRng, 0, 49));

        particles.emplace(spawnX, spawnY, velX, velY, r, g, b);
    }
}

void Player::spawnDefaultBoosterParticles(plf::colony<Particle>& particles,
                                          Random::RngEngine& fxRng) {
    SDL_FPoint rearCenter = getFrontCenter();
    if (m_facing == Direction::RIGHT)
        rearCenter.x = m_rect.x;
    else
        rearCenter.x = m_rect.x + m_rect.w;

    // random offset within a 5-unit wide by 6-unit tall rectangle centered on rearCenter
    float spawnX =
        rearCenter.x + static_cast<float>(Random::randomIntInclusive(fxRng, 0, 4)) - 2.5f;
    float spawnY =
        rearCenter.y + static_cast<float>(Random::randomIntInclusive(fxRng, 0, 5)) - 3.0f;

    float velX = (m_facing == Direction::RIGHT) ? -40.0f : 40.0f;
    velX += static_cast<float>(Random::randomIntInclusive(fxRng, 0, 19) - 10);
    float velY = static_cast<float>(Random::randomIntInclusive(fxRng, 0, 19) - 10);

    Uint8 r = 255;
    Uint8 g = toUint8Clamped(Random::randomIntInclusive(fxRng, 100, 199));
    Uint8 b = toUint8Clamped(Random::randomIntInclusive(fxRng, 0, 49));

    particles.emplace(spawnX, spawnY, velX, velY, r, g, b);
}
