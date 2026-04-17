#include <cmath>
#include <entities/opponents/sniper_opponent.h>

SniperOpponent::SniperOpponent(float x, float y, float w, float h, float oscillationOffset)
    : BaseOpponent(x, y, w, h) {
    m_speed = 30.0f;
    m_oscillationAmplitude = 60.0f;
    m_oscillationSpeed = 1.0f;
    m_oscillationOffset = oscillationOffset;
    m_fireInterval = 4.0f;

    m_health = 1;
    m_scoreVal = 100;

    m_explosionConfig.numParticles = 345;
    m_explosionConfig.angleJitter = 0.2f;
    m_explosionConfig.speedMin = 70.0f;
    m_explosionConfig.speedMax = 180.0f; // keeps prior max speed range
    m_explosionConfig.rMin = 55;
    m_explosionConfig.rMax = 154;
    m_explosionConfig.gMin = 155;
    m_explosionConfig.gMax = 254;
    m_explosionConfig.bMin = 55;
    m_explosionConfig.bMax = 104;
    m_explosionConfig.life = 1.25f;
    m_explosionConfig.size = 1.35f;
}

void SniperOpponent::update(float deltaTime, const SDL_FPoint& playerPos, float cameraX,
                            [[maybe_unused]] Random::RngEngine& simRng) {
    if (m_health <= 0)
        return;

    // simple movement
    m_rect.y += m_speed * deltaTime;
    m_angle += m_oscillationSpeed * deltaTime;
    m_rect.x = m_startX + sin(m_angle + m_oscillationOffset) * m_oscillationAmplitude;

    m_fireTimer += deltaTime;
    bool opponentVisible = isOnScreen(m_rect.x + m_rect.w / 2, cameraX);

    if (opponentVisible && m_fireTimer >= m_fireInterval) {
        m_projectiles.emplace(m_rect.x + m_rect.w / 2, m_rect.y + m_rect.h / 2, playerPos.x,
                              playerPos.y, 1800.0f);
        m_fireTimer = 0.0f;
    }
}
