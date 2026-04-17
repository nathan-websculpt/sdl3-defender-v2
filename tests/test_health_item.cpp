#include <cmath>
#include <entities/health_item.h>
#include <gtest/gtest.h>
#include <limits>
#include <tests/test_support.h>
using TestSupport::GlobalStateFixture;

TEST_F(GlobalStateFixture, healthItemFallsBeforeBlinking) {
    HealthItem item(10.0f, 0.0f, 30.0f, 30.0f, HealthItemType::PLAYER, "texture");
    const float initialY = item.getBounds().y;

    item.update(1.0f);
    EXPECT_GT(item.getBounds().y, initialY);
    EXPECT_FALSE(item.isBlinking());
    EXPECT_TRUE(item.isAlive());
    EXPECT_FLOAT_EQ(item.getBlinkAlpha(), 255.0f);
}

TEST_F(GlobalStateFixture, blinkingAlphaTogglesAndEventuallyExpires) {
    HealthItem item(10.0f, 0.0f, 30.0f, 30.0f, HealthItemType::WORLD, "texture");
    item.startBlinking();

    EXPECT_TRUE(item.isBlinking());
    EXPECT_FLOAT_EQ(item.getBlinkAlpha(), 255.0f);

    item.update(HealthItem::BLINK_DURATION / 2.0f);
    EXPECT_FLOAT_EQ(item.getBlinkAlpha(), 0.0f);

    item.update(HealthItem::BLINK_DURATION / 2.0f);
    EXPECT_FLOAT_EQ(item.getBlinkAlpha(), 255.0f);

    for (int i = 0; i < 6; ++i) {
        item.update(HealthItem::BLINK_DURATION);
    }
    EXPECT_FALSE(item.isAlive());
}

TEST_F(GlobalStateFixture, blinkingAlphaRemainsValidAfterNonFiniteAndSpikeTimers) {
    HealthItem item(10.0f, 0.0f, 30.0f, 30.0f, HealthItemType::WORLD, "texture");
    item.startBlinking();

    item.update(std::numeric_limits<float>::quiet_NaN());
    const float alphaAfterNan = item.getBlinkAlpha();
    EXPECT_TRUE(std::isfinite(alphaAfterNan));
    EXPECT_TRUE(alphaAfterNan == 0.0f || alphaAfterNan == 255.0f);

    item.update(1000.0f);
    const float alphaAfterSpike = item.getBlinkAlpha();
    EXPECT_TRUE(std::isfinite(alphaAfterSpike));
    EXPECT_TRUE(alphaAfterSpike == 0.0f || alphaAfterSpike == 255.0f);
}
