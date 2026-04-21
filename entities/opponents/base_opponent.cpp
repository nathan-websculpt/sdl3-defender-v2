#include "entities/opponents/base_opponent.h"

#include <algorithm>
#include <cmath>

#include "core/config.h"
#include "core/globals.h"
#include "core/managers/sound_manager.h"
#include "entities/particle.h"

namespace {

Uint8 toColorChannel(int value) {
    const int clamped = std::clamp(value, 0, 255);
    return static_cast<Uint8>(clamped);
}

} // namespace

BaseOpponent::BaseOpponent(float x, float y, float w, float h)
    : m_rect{x, y, w, h}, m_speed(0.0f), m_angle(0.0f), m_angularSpeed(0.0f),
      m_oscillationAmplitude(0.0f), m_startX(x), m_health(3), m_fireTimer(0.0f),
      m_fireInterval(0.0f), m_scoreVal(0) {}

void BaseOpponent::takeDamage(int damage) {
    m_health -= damage;
    if (m_health < 0)
        m_health = 0;
}

SDL_FRect BaseOpponent::getBounds() const {
    return m_rect;
}

plf::colony<Projectile>& BaseOpponent::getProjectiles() {
    return m_projectiles;
}

const plf::colony<Projectile>& BaseOpponent::getProjectiles() const {
    return m_projectiles;
}

int BaseOpponent::getScoreVal() const {
    return m_scoreVal;
}

bool BaseOpponent::isOnScreen(float objX, float cameraX) const {
    const float screenMinX = cameraX;
    const float screenMaxX = cameraX + globals.windowWidth;
    // Y is always fully visible because there is no vertical camera movement
    return (objX >= screenMinX && objX <= screenMaxX);
}

void BaseOpponent::explode(plf::colony<Particle>& gameParticles, Random::RngEngine& fxRng) const {
    SoundManager::getInstance().playSound(Config::Sounds::OPPONENT_EXPLODE);

    SDL_FPoint center = {m_rect.x + m_rect.w / 2.0f, m_rect.y + m_rect.h / 2.0f};
    const ExplosionConfig& cfg = m_explosionConfig;
    const float particleCount = static_cast<float>(cfg.numParticles);
    const float twoPi = 2.0f * static_cast<float>(M_PI);

    for (int i = 0; i < cfg.numParticles; ++i) {
        float baseAngle = (static_cast<float>(i) / particleCount) * twoPi;
        float angle = baseAngle + Random::randomFloatRange(fxRng, 0.0f, cfg.angleJitter);
        float speed = Random::randomFloatRange(fxRng, cfg.speedMin, cfg.speedMax);

        float velX = cosf(angle) * speed;
        float velY = sinf(angle) * speed;

        const int r = Random::randomIntInclusive(fxRng, static_cast<int>(cfg.rMin),
                                                 static_cast<int>(cfg.rMax));
        const int g = Random::randomIntInclusive(fxRng, static_cast<int>(cfg.gMin),
                                                 static_cast<int>(cfg.gMax));
        const int b = Random::randomIntInclusive(fxRng, static_cast<int>(cfg.bMin),
                                                 static_cast<int>(cfg.bMax));

        gameParticles.emplace(center.x, center.y, velX, velY, toColorChannel(r), toColorChannel(g),
                              toColorChannel(b), cfg.size, cfg.life);
    }
}
