#include <gtest/gtest.h>
#include <limits>
#include <memory>

#include "core/random/rng_service.h"

TEST(RandomService, deterministicStreamsMatchForSameSeed) {
    constexpr std::uint64_t baseSeed = 123456789ULL;
    auto lhs = std::make_unique<Random::RngStreams>(Random::makeDeterministicStreams(baseSeed));
    auto rhs = std::make_unique<Random::RngStreams>(Random::makeDeterministicStreams(baseSeed));

    EXPECT_EQ(lhs->seeds.baseSeed, rhs->seeds.baseSeed);
    EXPECT_EQ(lhs->seeds.simSeed, rhs->seeds.simSeed);
    EXPECT_EQ(lhs->seeds.fxSeed, rhs->seeds.fxSeed);
    EXPECT_NE(lhs->seeds.simSeed, lhs->seeds.fxSeed);

    {
        auto& lhsSim = lhs->simEngine;
        auto& rhsSim = rhs->simEngine;

        for (int i = 0; i < 64; ++i) {
            EXPECT_EQ(Random::randomIntInclusive(lhsSim, -1000, 1000),
                      Random::randomIntInclusive(rhsSim, -1000, 1000));
        }
    }

    {
        auto& lhsFx = lhs->fxEngine;
        auto& rhsFx = rhs->fxEngine;

        for (int i = 0; i < 64; ++i) {
            EXPECT_EQ(Random::randomIntInclusive(lhsFx, -1000, 1000),
                      Random::randomIntInclusive(rhsFx, -1000, 1000));
        }
    }
}

TEST(RandomService, helperOutputsStayWithinExpectedRanges) {
    auto streams = Random::makeDeterministicStreams(987654321ULL);

    for (int i = 0; i < 256; ++i) {
        const int boundedInt = Random::randomIntInclusive(streams.simEngine, -5, 7);
        EXPECT_GE(boundedInt, -5);
        EXPECT_LE(boundedInt, 7);

        const float unit = Random::randomFloat01(streams.simEngine);
        EXPECT_GE(unit, 0.0f);
        EXPECT_LT(unit, 1.0f);

        const float ranged = Random::randomFloatRange(streams.fxEngine, -4.5f, 9.25f);
        EXPECT_GE(ranged, -4.5f);
        EXPECT_LE(ranged, 9.25f);
    }
}

TEST(RandomService, chanceBoundaryValuesAreDeterministic) {
    auto streams = Random::makeDeterministicStreams(222ULL);

    for (int i = 0; i < 128; ++i) {
        EXPECT_FALSE(Random::randomChance(streams.simEngine, 0.0));
        EXPECT_TRUE(Random::randomChance(streams.simEngine, 1.0));
    }
}

TEST(RandomService, invalidInputFallbacksAreDeterministicInRelease) {
#ifndef NDEBUG
    GTEST_SKIP() << "release-only fallback checks";
#else
    auto streams = Random::makeDeterministicStreams(333ULL);

    EXPECT_FALSE(Random::randomChance(streams.simEngine, std::numeric_limits<double>::quiet_NaN()));
    EXPECT_EQ(Random::randomIntInclusive(streams.simEngine, 10, -2), 10);
    EXPECT_FLOAT_EQ(
        Random::randomFloatRange(streams.fxEngine, std::numeric_limits<float>::quiet_NaN(), 5.0f),
        0.0f);
    EXPECT_FLOAT_EQ(Random::randomFloatRange(streams.fxEngine, 8.0f, 2.0f), 8.0f);
#endif
}
