#pragma once
#include "core/config.h"
#include "entities/opponents/base_opponent.h"

class BasicOpponent : public BaseOpponent {
  public:
    BasicOpponent(float x, float y, float w, float h);
    ~BasicOpponent() = default;

    void update(float deltaTime, const SDL_FPoint& playerPos, float cameraX,
                Random::RngEngine& simRng) override;

    bool damagesWorldOnGroundImpact() const override {
        return true;
    }

    const std::string& getTextureKey() const override {
        return Config::Textures::BASIC_OPPONENT;
    }
};
