#pragma once
#include <entities/opponents/base_opponent.h>

#include <core/config.h>

class AggressiveOpponent : public BaseOpponent {
  public:
    AggressiveOpponent(float x, float y, float w, float h);
    ~AggressiveOpponent() = default;

    void update(float deltaTime, const SDL_FPoint& playerPos, float cameraX,
                Random::RngEngine& simRng) override;

    const std::string& getTextureKey() const override {
        return Config::Textures::AGGRESSIVE_OPPONENT;
    }

  private:
    float m_lifetime;
};
