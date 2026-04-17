#include <entities/particle.h>
#include <entities/player.h>
#include <entities/projectile.h>
#include <gtest/gtest.h>
#include <numbers>
#include <tests/test_support.h>
using TestSupport::GlobalStateFixture;

TEST_F(GlobalStateFixture, horizontalProjectileConstructorAndUpdate)
{
    Projectile projectile(10.0f, 20.0f, 1.0f, 600.0f);

    EXPECT_TRUE(projectile.isHorizontal());
    EXPECT_NEAR(projectile.getVelocity().x, 600.0f, 0.0001f);
    EXPECT_NEAR(projectile.getVelocity().y, 0.0f, 0.0001f);

    projectile.update(0.1f);
    EXPECT_NEAR(projectile.getCurrentX(), 70.0f, 0.0001f);
    EXPECT_NEAR(projectile.getCurrentY(), 20.0f, 0.0001f);
    EXPECT_NEAR(projectile.getAge(), 0.1f, 0.0001f);
}

TEST_F(GlobalStateFixture, horizontalProjectileLifetimeUsesWindowCoverageWithVisibilityBuffer)
{
    globals.windowWidth = 1280;
    Projectile projectile(10.0f, 20.0f, 1.0f, 2040.0f);

    const float strictCoverageLifetime = static_cast<float>(globals.windowWidth) / 2040.0f;
    const float expectedLifetime = (static_cast<float>(globals.windowWidth) + 64.0f) / 2040.0f;

    EXPECT_NEAR(projectile.getLifetime(), expectedLifetime, 0.0001f);
    EXPECT_GT(projectile.getLifetime(), strictCoverageLifetime);
}

TEST_F(GlobalStateFixture, horizontalProjectileLifetimeFallsBackForInvalidSpeed)
{
    Projectile projectile(10.0f, 20.0f, 1.0f, 0.0f);

    EXPECT_NEAR(projectile.getLifetime(), 0.55f, 0.0001f);
}

TEST_F(GlobalStateFixture, playerShootUsesTripledBaseSpeedWhenNotBoosted)
{
    Player player(10.0f, 20.0f, 80.0f, 48.0f);

    player.shoot();

    const auto& projectiles = player.getProjectiles();
    ASSERT_EQ(TestSupport::colonySize(projectiles), 1u);
    const auto projectileIt = projectiles.begin();
    ASSERT_NE(projectileIt, projectiles.end());
    EXPECT_NEAR(projectileIt->getVelocity().x, 2040.0f, 0.0001f);
}

TEST_F(GlobalStateFixture, playerShootBoostAddsMovementDeltaToShotSpeed)
{
    Player player(10.0f, 20.0f, 80.0f, 48.0f);
    const float regularSpeed = player.getSpeed();

    player.setSpeedBoost(true);
    const float boostedSpeed = player.getSpeed();
    const float expectedShotSpeed = 2040.0f + (boostedSpeed - regularSpeed);

    player.shoot();

    const auto& projectiles = player.getProjectiles();
    ASSERT_EQ(TestSupport::colonySize(projectiles), 1u);
    const auto projectileIt = projectiles.begin();
    ASSERT_NE(projectileIt, projectiles.end());
    EXPECT_NEAR(projectileIt->getVelocity().x, expectedShotSpeed, 0.0001f);
}

TEST_F(GlobalStateFixture, aimedProjectileConstructorNormalizesVelocity)
{
    Projectile projectile(0.0f, 0.0f, 3.0f, 4.0f, 10.0f);

    EXPECT_FALSE(projectile.isHorizontal());
    EXPECT_NEAR(projectile.getVelocity().x, 6.0f, 0.0001f);
    EXPECT_NEAR(projectile.getVelocity().y, 8.0f, 0.0001f);
}

TEST_F(GlobalStateFixture, aimedProjectileHandlesZeroDistanceSafely)
{
    Projectile projectile(10.0f, 10.0f, 10.0f, 10.0f, 250.0f);

    EXPECT_NEAR(projectile.getVelocity().x, 0.0f, 0.0001f);
    EXPECT_NEAR(projectile.getVelocity().y, 0.0f, 0.0001f);
}

TEST_F(GlobalStateFixture, projectileColorBlueChannelClampsAtZeroForNegativeSineRange)
{
    Projectile projectile(10.0f, 20.0f, 1.0f, 600.0f);
    const float ageAtBlueMinimum = std::numbers::pi_v<float> / 35.0f;
    projectile.update(ageAtBlueMinimum);

    const SDL_Color color = projectile.getColor();
    EXPECT_EQ(color.b, static_cast<Uint8>(0));
    EXPECT_LE(color.r, static_cast<Uint8>(255));
    EXPECT_LE(color.g, static_cast<Uint8>(255));
    EXPECT_LE(color.a, static_cast<Uint8>(255));
}

TEST_F(GlobalStateFixture, particleFadesAndExpiresWithLifetime)
{
    Particle particle(0.0f, 0.0f, 20.0f, 0.0f, 255, 120, 50, 2.0f, 0.2f);

    EXPECT_TRUE(particle.isAlive());
    particle.update(0.1f);
    EXPECT_GT(particle.getX(), 0.0f);
    EXPECT_LT(particle.getAlpha(), 255);
    EXPECT_TRUE(particle.isAlive());

    particle.update(0.2f);
    EXPECT_FALSE(particle.isAlive());
}

TEST_F(GlobalStateFixture, particleWithZeroLifetimeFallsBackToZeroAlphaSafely)
{
    Particle particle(0.0f, 0.0f, 0.0f, 0.0f, 255, 120, 50, 2.0f, 0.0f);

    particle.update(0.016f);
    EXPECT_EQ(particle.getAlpha(), static_cast<Uint8>(0));
    EXPECT_FALSE(particle.isAlive());
}

TEST_F(GlobalStateFixture, particleWithTinyLifetimeKeepsAlphaChannelValid)
{
    Particle particle(0.0f, 0.0f, 0.0f, 0.0f, 255, 120, 50, 2.0f, 0.0001f);

    particle.update(1.0f);
    EXPECT_LE(particle.getAlpha(), static_cast<Uint8>(255));
    EXPECT_FALSE(particle.isAlive());
}
