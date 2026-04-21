#include <cmath>
#include <gtest/gtest.h>

#include "entities/opponents/aggressive_opponent.h"
#include "entities/opponents/basic_opponent.h"
#include "entities/opponents/sniper_opponent.h"
#include "entities/particle.h"
#include "tests/test_support.h"
using TestSupport::GlobalStateFixture;

TEST_F(GlobalStateFixture, basicOpponentMovesDownAndOscillates) {
    auto rng = Random::makeDeterministicStreams(11U);

    BasicOpponent opponent(100.0f, 50.0f, 40.0f, 40.0f);
    const SDL_FPoint playerPos{300.0f, 300.0f};

    opponent.update(1.0f, playerPos, 0.0f, rng.simEngine);
    SDL_FRect bounds = opponent.getBounds();

    EXPECT_NEAR(bounds.y, 80.0f, 0.0001f);
    EXPECT_NEAR(bounds.x, 100.0f + std::sin(1.5f) * 80.0f, 0.001f);
}

TEST_F(GlobalStateFixture, sniperOpponentFiresOnlyAfterIntervalWhenVisible) {
    auto rng = Random::makeDeterministicStreams(12U);

    globals.windowWidth = 2000;

    SniperOpponent opponent(100.0f, 0.0f, 35.0f, 35.0f, 0.0f);
    const SDL_FPoint playerPos{400.0f, 200.0f};

    opponent.update(1.0f, playerPos, 0.0f, rng.simEngine);
    opponent.update(1.0f, playerPos, 0.0f, rng.simEngine);
    opponent.update(1.0f, playerPos, 0.0f, rng.simEngine);
    EXPECT_EQ(TestSupport::colonySize(opponent.getProjectiles()), 0u);

    opponent.update(1.0f, playerPos, 0.0f, rng.simEngine);
    EXPECT_EQ(TestSupport::colonySize(opponent.getProjectiles()), 1u);
}

TEST_F(GlobalStateFixture, aggressiveOpponentChasesAndFiresOnCadence) {
    auto rng = Random::makeDeterministicStreams(13U);

    globals.windowWidth = 3000;

    AggressiveOpponent opponent(100.0f, 100.0f, 45.0f, 45.0f);
    const SDL_FPoint playerPos{1000.0f, 1000.0f};

    opponent.update(0.2f, playerPos, 0.0f, rng.simEngine);
    SDL_FRect movedBounds = opponent.getBounds();
    EXPECT_GT(movedBounds.x, 100.0f);
    EXPECT_GT(movedBounds.y, 100.0f);
    EXPECT_EQ(TestSupport::colonySize(opponent.getProjectiles()), 0u);

    opponent.update(2.0f, playerPos, 0.0f, rng.simEngine);
    EXPECT_EQ(TestSupport::colonySize(opponent.getProjectiles()), 1u);
}

TEST_F(GlobalStateFixture, sniperFiringVisibilityTracksWindowWidth_currentBehavior) {
    auto rng = Random::makeDeterministicStreams(14U);

    SniperOpponent opponent(500.0f, 0.0f, 35.0f, 35.0f, 0.0f);
    const SDL_FPoint playerPos{400.0f, 200.0f};

    globals.windowWidth = 200;
    opponent.update(4.1f, playerPos, 0.0f, rng.simEngine);
    EXPECT_EQ(TestSupport::colonySize(opponent.getProjectiles()), 0u);

    globals.windowWidth = 1000;
    opponent.update(0.0f, playerPos, 0.0f, rng.simEngine);
    EXPECT_EQ(TestSupport::colonySize(opponent.getProjectiles()), 1u);
}

TEST_F(GlobalStateFixture, basicOpponentExplosionUsesConfiguredSizeAndLifetime) {
    auto rng = Random::makeDeterministicStreams(19U);

    BasicOpponent opponent(100.0f, 50.0f, 40.0f, 40.0f);
    plf::colony<Particle> particles;
    opponent.explode(particles, rng.fxEngine);

    ASSERT_GT(TestSupport::colonySize(particles), 0u);

    auto firstParticle = particles.begin();
    EXPECT_NEAR(firstParticle->getCurrentSize(), 2.2f, 0.0001f);
    firstParticle->update(1.0f);
    EXPECT_TRUE(firstParticle->isAlive());
    firstParticle->update(0.3f);
    EXPECT_FALSE(firstParticle->isAlive());
}
