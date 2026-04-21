#pragma once
#include "core/config.h"
#include "entities/opponents/base_opponent.h"

class SniperOpponent : public BaseOpponent {
  public:
    SniperOpponent(float x, float y, float w, float h, float oscillationOffset);
    ~SniperOpponent() = default;

    void update(float deltaTime, const SDL_FPoint& playerPos, float cameraX,
                Random::RngEngine& simRng) override;

    const std::string& getTextureKey() const override {
        return Config::Textures::SNIPER_OPPONENT;
    }

  private:
    float m_oscillationSpeed;
    float m_oscillationOffset;
};
