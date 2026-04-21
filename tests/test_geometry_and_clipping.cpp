#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <vector>

#include "core/helpers_game/game_helper.h"
#include "core/helpers_platform/projectile_clipping.h"
#include "entities/player.h"
#include "tests/test_support.h"
using TestSupport::GlobalStateFixture;

TEST_F(GlobalStateFixture, groundInterpolationAndBoundsWork) {
    const std::vector<SDL_FPoint> landscape{{0.0f, 700.0f}, {100.0f, 600.0f}, {200.0f, 650.0f}};
    const GameHelper helper(landscape);

    EXPECT_FLOAT_EQ(helper.getGroundYAt(-20.0f), 700.0f);
    EXPECT_FLOAT_EQ(helper.getGroundYAt(0.0f), 700.0f);
    EXPECT_NEAR(helper.getGroundYAt(50.0f), 650.0f, 0.0001f);
    EXPECT_NEAR(helper.getGroundYAt(150.0f), 625.0f, 0.0001f);
    EXPECT_FLOAT_EQ(helper.getGroundYAt(999.0f), 650.0f);
}

TEST_F(GlobalStateFixture, rectIntersectionIsCorrect) {
    const std::vector<SDL_FPoint> landscape = TestSupport::makeFlatLandscape(700.0f);
    const GameHelper helper(landscape);

    const SDL_FRect a{10.0f, 10.0f, 20.0f, 20.0f};
    const SDL_FRect b{25.0f, 15.0f, 20.0f, 20.0f};
    const SDL_FRect c{100.0f, 100.0f, 10.0f, 10.0f};

    EXPECT_TRUE(helper.rectsIntersect(a, b));
    EXPECT_FALSE(helper.rectsIntersect(a, c));
}

TEST_F(GlobalStateFixture, beamEndHandlesSlopeAndDirection) {
    const std::vector<SDL_FPoint> landscape{{0.0f, 700.0f}, {100.0f, 600.0f}, {200.0f, 700.0f}};
    const GameHelper helper(landscape);

    EXPECT_NEAR(helper.getBeamVisualEndX(0.0f, 650.0f, true), 50.0f, 0.0001f);
    EXPECT_NEAR(helper.getBeamVisualEndX(180.0f, 650.0f, false), 150.0f, 0.0001f);
    EXPECT_FLOAT_EQ(helper.getBeamVisualEndX(50.0f, 100.0f, true),
                    static_cast<float>(Config::Game::WORLD_WIDTH));

    EXPECT_FLOAT_EQ(
        helper.getBeamVisualEndX(0.0f, 650.0f, true),
        ProjectileClipping::findBeamLandscapeIntersection(0.0f, 650.0f, true, landscape));
    EXPECT_FLOAT_EQ(
        helper.getBeamVisualEndX(180.0f, 650.0f, false),
        ProjectileClipping::findBeamLandscapeIntersection(180.0f, 650.0f, false, landscape));
    EXPECT_FLOAT_EQ(
        helper.getBeamVisualEndX(50.0f, 100.0f, true),
        ProjectileClipping::findBeamLandscapeIntersection(50.0f, 100.0f, true, landscape));
}

TEST_F(GlobalStateFixture, keepPlayerInBoundsClampsHudWorldAndGround) {
    const std::vector<SDL_FPoint> landscape = TestSupport::makeFlatLandscape(700.0f);
    GameHelper helper(landscape);

    auto player = std::make_unique<Player>(-20.0f, 50.0f, 80.0f, 48.0f);
    SDL_FRect bounds = player->getBounds();
    helper.keepPlayerInBounds(*player, bounds);

    SDL_FRect clamped = player->getBounds();
    EXPECT_FLOAT_EQ(clamped.x, 0.0f);
    EXPECT_FLOAT_EQ(clamped.y, static_cast<float>(Config::Game::HUD_HEIGHT));

    player->setPosition(static_cast<float>(Config::Game::WORLD_WIDTH - 5), 900.0f);
    bounds = player->getBounds();
    helper.keepPlayerInBounds(*player, bounds);
    clamped = player->getBounds();

    EXPECT_FLOAT_EQ(clamped.x, static_cast<float>(Config::Game::WORLD_WIDTH) - clamped.w);
    EXPECT_FLOAT_EQ(clamped.y, 652.0f);
}

TEST_F(GlobalStateFixture, clippingHelpersReturnExpectedIntersections) {
    const std::vector<SDL_FPoint> landscape{{0.0f, 700.0f}, {100.0f, 600.0f}, {200.0f, 700.0f}};

    EXPECT_NEAR(ProjectileClipping::findBeamLandscapeIntersection(0.0f, 650.0f, true, landscape),
                50.0f, 0.0001f);
    EXPECT_NEAR(ProjectileClipping::findBeamLandscapeIntersection(180.0f, 650.0f, false, landscape),
                150.0f, 0.0001f);

    const std::vector<SDL_FPoint> flat{{0.0f, 100.0f}, {200.0f, 100.0f}};
    SDL_FPoint clipped = ProjectileClipping::clipRayToLandscape(50.0f, 50.0f, 50.0f, 150.0f, flat);
    EXPECT_NEAR(clipped.x, 50.0f, 0.0001f);
    EXPECT_NEAR(clipped.y, 100.0f, 0.0001f);

    SDL_FPoint unclipped = ProjectileClipping::clipRayToLandscape(10.0f, 10.0f, 20.0f, 20.0f, flat);
    EXPECT_NEAR(unclipped.x, 20.0f, 0.0001f);
    EXPECT_NEAR(unclipped.y, 20.0f, 0.0001f);

    SDL_FPoint degenerate =
        ProjectileClipping::clipRayToLandscape(30.0f, 40.0f, 30.0f, 40.0f, flat);
    EXPECT_NEAR(degenerate.x, 30.0f, 0.0001f);
    EXPECT_NEAR(degenerate.y, 40.0f, 0.0001f);

    SDL_FPoint nearVertical =
        ProjectileClipping::clipRayToLandscape(50.0f, 50.0f, 50.0000001f, 150.0f, flat);
    EXPECT_TRUE(std::isfinite(nearVertical.x));
    EXPECT_TRUE(std::isfinite(nearVertical.y));
    EXPECT_NEAR(nearVertical.y, 100.0f, 0.0001f);
}

TEST_F(GlobalStateFixture, clippingHelpersHandleNonFiniteInputsSafely) {
    const std::vector<SDL_FPoint> landscape{{0.0f, 700.0f}, {100.0f, 600.0f}, {200.0f, 700.0f}};
    const float nanValue = std::numeric_limits<float>::quiet_NaN();

    EXPECT_FLOAT_EQ(
        ProjectileClipping::findBeamLandscapeIntersection(nanValue, 650.0f, true, landscape),
        static_cast<float>(Config::Game::WORLD_WIDTH));
    EXPECT_FLOAT_EQ(
        ProjectileClipping::findBeamLandscapeIntersection(150.0f, nanValue, false, landscape),
        0.0f);

    SDL_FPoint fallbackToEnd =
        ProjectileClipping::clipRayToLandscape(nanValue, 10.0f, 20.0f, 30.0f, landscape);
    EXPECT_FLOAT_EQ(fallbackToEnd.x, 20.0f);
    EXPECT_FLOAT_EQ(fallbackToEnd.y, 30.0f);

    SDL_FPoint fallbackToZero =
        ProjectileClipping::clipRayToLandscape(nanValue, 10.0f, nanValue, nanValue, landscape);
    EXPECT_FLOAT_EQ(fallbackToZero.x, 0.0f);
    EXPECT_FLOAT_EQ(fallbackToZero.y, 0.0f);
}
