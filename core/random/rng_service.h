#pragma once

#include <cstdint>
#include <random>

namespace Random {

using RngEngine = std::mt19937;
using ValidationLogHook = void (*)(const char* message);

struct SeedSet {
    std::uint64_t baseSeed = 0;
    std::uint64_t simSeed = 0;
    std::uint64_t fxSeed = 0;
};

struct RngStreams {
    SeedSet seeds{};
    RngEngine simEngine{};
    RngEngine fxEngine{};
};

RngStreams makeRuntimeStreams();
RngStreams makeDeterministicStreams(std::uint64_t baseSeed);
void setValidationLogHook(ValidationLogHook hook);

bool randomChance(RngEngine& engine, double probability);
int randomIntInclusive(RngEngine& engine, int minValue, int maxValue);
float randomFloat01(RngEngine& engine);
float randomFloatRange(RngEngine& engine, float minValue, float maxValue);

} // namespace Random
