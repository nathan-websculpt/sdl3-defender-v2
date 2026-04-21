#pragma once
#include <filesystem>
#include <string>

#include "core/config.h"
struct GameStateData;

bool isAllowedHighScoreInputChar(char c);
std::string normalizeHighScoreName(const std::string& name);

class HighScores {
  public:
    HighScores();
    HighScores(const std::filesystem::path& writablePathOverride,
               const std::filesystem::path& bundledPathOverride = {});

    void loadHighScores(GameStateData& state);
    void submitHighScore(const std::string& name, GameStateData& state);
    bool isHighScore(GameStateData& state) const;
    int getHighScoreIndex(GameStateData& state) const;

  private:
    [[nodiscard]] bool saveHighScores(const GameStateData& state);

    std::filesystem::path getWritableHighScoresPath() const;
    std::filesystem::path getBundledHighScoresPath() const;

    std::filesystem::path m_writablePathOverride;
    std::filesystem::path m_bundledPathOverride;
};
