#include "core/random/rng_service.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>

namespace {

constexpr std::uint64_t SIM_DOMAIN = 0x73696d2d7273676eULL;
constexpr std::uint64_t FX_DOMAIN = 0x66782d7273676e21ULL;
constexpr std::uint64_t RUNTIME_FALLBACK = 0x4f5f7a9cd12ef903ULL;
Random::ValidationLogHook g_validationLogHook = nullptr;

void logValidationIssue(const char* message) {
    if (!message) {
        return;
    }

    if (g_validationLogHook) {
        g_validationLogHook(message);
        return;
    }

    std::fprintf(stderr, "random validation: %s\n", message);
}

std::uint64_t splitMix64(std::uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31U);
}

Random::RngEngine makeEngine(std::uint64_t seedValue) {
    std::array<std::uint32_t, 4> words{
        static_cast<std::uint32_t>(seedValue & 0xFFFFFFFFULL),
        static_cast<std::uint32_t>((seedValue >> 32U) & 0xFFFFFFFFULL),
        static_cast<std::uint32_t>(splitMix64(seedValue) & 0xFFFFFFFFULL),
        static_cast<std::uint32_t>((splitMix64(seedValue) >> 32U) & 0xFFFFFFFFULL)};

    std::seed_seq seq(words.begin(), words.end());
    return Random::RngEngine(seq);
}

std::uint64_t makeRuntimeBaseSeed() {
    std::random_device device;
    const std::uint64_t high = static_cast<std::uint64_t>(device()) << 32U;
    const std::uint64_t low = static_cast<std::uint64_t>(device());
    const std::uint64_t mixed = high | low;
    return splitMix64(mixed ^ RUNTIME_FALLBACK);
}

} // namespace

Random::RngStreams Random::makeRuntimeStreams() {
    return makeDeterministicStreams(makeRuntimeBaseSeed());
}

Random::RngStreams Random::makeDeterministicStreams(std::uint64_t baseSeed) {
    const std::uint64_t simSeed = splitMix64(baseSeed ^ SIM_DOMAIN);
    const std::uint64_t fxSeed = splitMix64(baseSeed ^ FX_DOMAIN);

    return {SeedSet{baseSeed, simSeed, fxSeed}, makeEngine(simSeed), makeEngine(fxSeed)};
}

void Random::setValidationLogHook(ValidationLogHook hook) {
    g_validationLogHook = hook;
}

bool Random::randomChance(RngEngine& engine, double probability) {
    if (!std::isfinite(probability)) {
        assert(false && "probability must be finite");
        logValidationIssue("probability must be finite");
        return false;
    }
    if (probability <= 0.0)
        return false;
    if (probability >= 1.0)
        return true;

    std::bernoulli_distribution distribution(probability);
    return distribution(engine);
}

int Random::randomIntInclusive(RngEngine& engine, int minValue, int maxValue) {
    if (minValue > maxValue) {
        assert(false && "minValue must be <= maxValue");
        logValidationIssue("minValue must be <= maxValue");
        return minValue;
    }

    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(engine);
}

float Random::randomFloat01(RngEngine& engine) {
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(engine);
}

float Random::randomFloatRange(RngEngine& engine, float minValue, float maxValue) {
    if (!std::isfinite(minValue) || !std::isfinite(maxValue)) {
        assert(false && "range bounds must be finite");
        logValidationIssue("range bounds must be finite");
        return 0.0f;
    }
    if (minValue > maxValue) {
        assert(false && "minValue must be <= maxValue");
        logValidationIssue("minValue must be <= maxValue");
        return minValue;
    }
    if (minValue == maxValue)
        return minValue;

    const float span = maxValue - minValue;
    if (!std::isfinite(span) || span < 0.0f) {
        assert(false && "range span must be finite and non-negative");
        logValidationIssue("range span must be finite and non-negative");
        return minValue;
    }

    const float unit = randomFloat01(engine);
    const float sampled = minValue + span * unit;
    if (!std::isfinite(sampled)) {
        assert(false && "sampled value must be finite");
        logValidationIssue("sampled value must be finite");
        return minValue;
    }

    return sampled;
}
