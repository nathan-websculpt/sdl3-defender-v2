#include "entities/opponents/basic_opponent.h"

#include <cmath>

BasicOpponent::BasicOpponent(float x, float y, float w, float h) : BaseOpponent(x, y, w, h) {
    m_speed = 30.0f;
    m_angularSpeed = 1.5f;
    m_oscillationAmplitude = 80.0f;

    m_scoreVal = 300;

    m_explosionConfig.numParticles = 450;
    m_explosionConfig.angleJitter = 0.2f;
    m_explosionConfig.speedMin = 80.0f;
    m_explosionConfig.speedMax = 230.0f; // keeps prior max speed range
    m_explosionConfig.rMin = 155;
    m_explosionConfig.rMax = 254;
    m_explosionConfig.gMin = 55;
    m_explosionConfig.gMax = 154;
    m_explosionConfig.bMin = 0;
    m_explosionConfig.bMax = 49;
    m_explosionConfig.life = 1.25f;
    m_explosionConfig.size = 2.2f;
}

void BasicOpponent::update(float deltaTime, [[maybe_unused]] const SDL_FPoint& playerPos,
                           [[maybe_unused]] float cameraX,
                           [[maybe_unused]] Random::RngEngine& simRng) {
    if (m_health <= 0)
        return;

    m_rect.y += m_speed * deltaTime;
    m_angle += m_angularSpeed * deltaTime;
    m_rect.x = m_startX + sin(m_angle) * m_oscillationAmplitude;
}
