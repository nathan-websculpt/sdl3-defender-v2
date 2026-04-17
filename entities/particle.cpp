#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <entities/particle.h>

Particle::Particle(float x, float y, float velocityX, float velocityY, Uint8 r, Uint8 g, Uint8 b,
                   float initialSize, float lifetime)
    : m_rect{x, y, initialSize, initialSize}, m_velocity{velocityX, velocityY}, m_r(r), m_g(g),
      m_b(b), m_alpha(255), m_age(0.0f), m_lifetime(lifetime), m_initialSize(initialSize),
      m_growRate(2.0f), // particle growth rate
      m_fadeRate(0.8f)  // rate at which alpha decreases per second
{}

void Particle::update(float deltaTime) {
    m_rect.x += m_velocity.x * deltaTime;
    m_rect.y += m_velocity.y * deltaTime;

    m_age += deltaTime;

    // calc size based on age and growth rate
    float currentSize = m_initialSize + (m_age * m_growRate);

    // calc alpha based on age and fade rate
    float alphaFloat = 0.0f;
    if (std::isfinite(m_lifetime) && m_lifetime > 0.0f && std::isfinite(m_age) &&
        std::isfinite(m_fadeRate)) {
        alphaFloat = ((m_lifetime - m_age) / m_lifetime) * 255.0f * m_fadeRate;
    }

    if (!std::isfinite(alphaFloat)) {
        alphaFloat = 0.0f;
    }
    alphaFloat = std::clamp(alphaFloat, 0.0f, 255.0f);

    // size and alpha changes
    m_rect.w = currentSize;
    m_rect.h = currentSize;
    m_alpha = static_cast<Uint8>(alphaFloat);

    // adjust position since changing size affects top-left corner
    m_rect.x -= (currentSize - m_initialSize) * 0.5f;
    m_rect.y -= (currentSize - m_initialSize) * 0.5f;
}

SDL_FRect Particle::getBounds() const {
    return m_rect;
}

bool Particle::isAlive() const {
    return m_age < m_lifetime && m_alpha > 0;
}
