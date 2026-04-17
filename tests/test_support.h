#pragma once

#include <SDL3/SDL.h>
#include <core/config.h>
#include <core/globals.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <iterator>
#include <memory>
#include <plf/plf_colony.h>
#include <vector>
namespace TestSupport
{

inline void resetGlobals(int windowWidth = 1000, int windowHeight = 800)
{
    globals.renderer = nullptr;
    globals.windowWidth = windowWidth;
    globals.windowHeight = windowHeight;
}

inline std::vector<SDL_FPoint> makeFlatLandscape(float y)
{
    return {{0.0f, y}, {Config::Game::WORLD_WIDTH, y}};
}

template <typename T> std::size_t colonySize(const plf::colony<T>& colony)
{
    return static_cast<std::size_t>(std::distance(colony.begin(), colony.end()));
}

template <typename T> std::size_t colonySize(const plf::colony<std::unique_ptr<T>>& colony)
{
    return static_cast<std::size_t>(std::distance(colony.begin(), colony.end()));
}

class GlobalStateFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        resetGlobals();
    }
};

} // namespace TestSupport
