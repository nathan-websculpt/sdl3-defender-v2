#include <gtest/gtest.h>

#include "core/helpers_game/colony_update_and_prune.h"
#include "core/helpers_game/game_helper.h"
#include "entities/health_item.h"
#include "entities/particle.h"
#include "entities/projectile.h"
#include "tests/test_support.h"
using TestSupport::GlobalStateFixture;

TEST_F(GlobalStateFixture, projectilePruneRemovesOutOfWorldEntries) {
    const std::vector<SDL_FPoint> landscape = TestSupport::makeFlatLandscape(700.0f);
    const GameHelper helper(landscape);

    plf::colony<Projectile> projectiles;
    projectiles.emplace(10.0f, 10.0f, 1.0f, 100.0f);
    projectiles.emplace(-10.0f, 10.0f, 1.0f, 100.0f);

    ColonyUpdateAndPrune::projectiles(projectiles, 0.0f, helper);
    EXPECT_EQ(TestSupport::colonySize(projectiles), 1u);
}

TEST_F(GlobalStateFixture, projectilePruneRemovesWhenTouchingGround) {
    const std::vector<SDL_FPoint> landscape = TestSupport::makeFlatLandscape(50.0f);
    const GameHelper helper(landscape);

    plf::colony<Projectile> projectiles;
    projectiles.emplace(10.0f, 49.0f, 1.0f, 0.0f);

    ColonyUpdateAndPrune::projectiles(projectiles, 0.0f, helper);
    EXPECT_EQ(TestSupport::colonySize(projectiles), 0u);
}

TEST_F(GlobalStateFixture, projectilePruneRemovesExpiredByLifetime) {
    const std::vector<SDL_FPoint> landscape = TestSupport::makeFlatLandscape(700.0f);
    const GameHelper helper(landscape);

    plf::colony<Projectile> projectiles;
    projectiles.emplace(120.0f, 200.0f, 1.0f, 0.0f);

    ColonyUpdateAndPrune::projectiles(projectiles, 0.6f, helper);
    EXPECT_EQ(TestSupport::colonySize(projectiles), 0u);
}

TEST_F(GlobalStateFixture, particlePruneRemovesExpiredParticles) {
    plf::colony<Particle> particles;
    particles.emplace(0.0f, 0.0f, 0.0f, 0.0f, 255, 0, 0, 2.0f, 0.05f);

    ColonyUpdateAndPrune::particles(particles, 0.1f);
    EXPECT_EQ(TestSupport::colonySize(particles), 0u);
}

TEST_F(GlobalStateFixture, healthItemPruneStartsBlinkingAndRemovesWhenDone) {
    const std::vector<SDL_FPoint> landscape = TestSupport::makeFlatLandscape(100.0f);
    const GameHelper helper(landscape);

    plf::colony<std::unique_ptr<HealthItem>> items;
    items.emplace(std::make_unique<HealthItem>(50.0f, 95.0f, 10.0f, 10.0f, HealthItemType::PLAYER,
                                               "texture"));

    ColonyUpdateAndPrune::healthItems(items, 0.0f, helper);
    ASSERT_EQ(TestSupport::colonySize(items), 1u);
    ASSERT_TRUE(items.begin()->get()->isBlinking());

    for (int i = 0; i < 6; ++i) {
        ColonyUpdateAndPrune::healthItems(items, HealthItem::BLINK_DURATION, helper);
    }

    EXPECT_EQ(TestSupport::colonySize(items), 0u);
}
