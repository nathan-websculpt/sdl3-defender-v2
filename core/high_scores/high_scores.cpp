#include "core/high_scores/high_scores.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <vector>

#include "core/game_state_data.h"

namespace {
std::filesystem::path pathFromUtf8(const char* utf8Path) {
    if (!utf8Path) {
        return {};
    }

    std::u8string utf8Bytes;
    for (const char* it = utf8Path; *it != '\0'; ++it) {
        utf8Bytes.push_back(static_cast<char8_t>(static_cast<unsigned char>(*it)));
    }

    return std::filesystem::path(utf8Bytes);
}

std::filesystem::path getDefaultBundledHighScoresPath() {
    const char* basePathUtf8 = SDL_GetBasePath();
    if (!basePathUtf8) {
        return std::filesystem::path(Config::Game::HIGH_SCORES_RESOURCE_PATH);
    }

    std::filesystem::path basePath = pathFromUtf8(basePathUtf8);
    return basePath / std::filesystem::path(Config::Game::HIGH_SCORES_RESOURCE_PATH);
}

std::filesystem::path getDefaultWritableHighScoresPath() {
#if defined(_WIN32)
    const char* prefPathUtf8 = SDL_GetPrefPath("SDL3Defender", "SDL3Defender");
    if (prefPathUtf8) {
        std::filesystem::path prefPath = pathFromUtf8(prefPathUtf8);

        std::error_code ec;
        std::filesystem::create_directories(prefPath, ec);
        if (ec) {
            SDL_Log("Warning: could not create pref path '%s': %s", prefPath.string().c_str(),
                    ec.message().c_str());
        }

        return prefPath / Config::Game::HIGH_SCORES_FILENAME;
    }

    SDL_Log("Warning: SDL_GetPrefPath failed, falling back to bundled high scores path: %s",
            SDL_GetError());
#endif
    return getDefaultBundledHighScoresPath();
}

void normalizeHighScoresList(std::vector<GameStateData::HighScore>& highScores,
                             std::size_t maxHighScores) {
    std::sort(highScores.begin(), highScores.end(),
              [](const GameStateData::HighScore& a, const GameStateData::HighScore& b) {
                  return a.score > b.score;
              });
    if (highScores.size() > maxHighScores) {
        highScores.resize(maxHighScores);
    }
}

bool parseHighScoreLine(const std::string& line, GameStateData::HighScore& entry) {
    std::istringstream iss(line);
    std::string name;
    int score = 0;
    std::string trailingToken;
    if (!(iss >> name >> score)) {
        return false;
    }
    if (iss >> trailingToken) {
        return false;
    }

    entry.name = name;
    entry.score = score;
    return true;
}

bool loadHighScoresFromFile(const std::filesystem::path& path,
                            std::vector<GameStateData::HighScore>& outScores,
                            std::size_t maxHighScores) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    outScores.clear();
    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line) && outScores.size() < maxHighScores) {
        ++lineNumber;
        if (line.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }

        GameStateData::HighScore entry;
        if (!parseHighScoreLine(line, entry)) {
            SDL_Log("Warning: malformed high scores entry in '%s' at line %d",
                    path.string().c_str(), lineNumber);
            outScores.clear();
            return false;
        }
        outScores.push_back(entry);
    }

    if (!file.eof() && file.fail()) {
        SDL_Log("Warning: failed while reading high scores file '%s'", path.string().c_str());
        outScores.clear();
        return false;
    }

    return true;
}
} // namespace

HighScores::HighScores() = default;

HighScores::HighScores(const std::filesystem::path& writablePathOverride,
                       const std::filesystem::path& bundledPathOverride)
    : m_writablePathOverride(writablePathOverride), m_bundledPathOverride(bundledPathOverride) {}

bool isAllowedHighScoreInputChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0;
}

std::string normalizeHighScoreName(const std::string& name) {
    std::string normalizedName = name;
    if (!normalizedName.empty()) {
        const std::size_t start = normalizedName.find_first_not_of(" \t");
        if (start != std::string::npos) {
            const std::size_t end = normalizedName.find_last_not_of(" \t");
            normalizedName = normalizedName.substr(start, end - start + 1);
        } else {
            normalizedName.clear();
        }
    }
    if (normalizedName.empty()) {
        normalizedName = "ANON";
    }
    return normalizedName;
}

void HighScores::loadHighScores(GameStateData& state) {
    const std::filesystem::path writablePath = getWritableHighScoresPath();
    std::vector<GameStateData::HighScore> loadedScores;
    bool loaded = loadHighScoresFromFile(writablePath, loadedScores,
                                         static_cast<std::size_t>(state.MAX_HIGH_SCORES));

#if defined(_WIN32)
    if (!loaded) {
        const std::filesystem::path bundledPath = getBundledHighScoresPath();
        if (bundledPath != writablePath) {
            loaded = loadHighScoresFromFile(bundledPath, loadedScores,
                                            static_cast<std::size_t>(state.MAX_HIGH_SCORES));
        }
    }
#endif

    if (loaded) {
        normalizeHighScoresList(loadedScores, static_cast<std::size_t>(state.MAX_HIGH_SCORES));
        state.highScores = std::move(loadedScores);
    } else {
        SDL_Log("Warning: no high scores file could be loaded");
        normalizeHighScoresList(state.highScores, static_cast<std::size_t>(state.MAX_HIGH_SCORES));
    }
}

bool HighScores::isHighScore(GameStateData& state) const {
    return state.highScores.size() < state.MAX_HIGH_SCORES ||
           state.playerScore > state.highScores.back().score;
}

int HighScores::getHighScoreIndex(GameStateData& state) const {
    // finds the index where new score should be inserted (0 is highest)
    const auto insertPos =
        std::ranges::find_if(state.highScores, [&state](const GameStateData::HighScore& entry) {
            return state.playerScore > entry.score;
        });
    if (insertPos != state.highScores.end()) {
        return static_cast<int>(std::distance(state.highScores.begin(), insertPos));
    }

    // if no higher score exists, append only when there is room
    if (state.highScores.size() < state.MAX_HIGH_SCORES) {
        return static_cast<int>(state.highScores.size());
    }

    return -1;
}

void HighScores::submitHighScore(const std::string& name, GameStateData& state) {
    int index = getHighScoreIndex(state);
    if (index != -1) {
        GameStateData::HighScore newEntry;
        newEntry.name = normalizeHighScoreName(name);
        newEntry.score = state.playerScore;
        state.highScores.insert(state.highScores.begin() + index, newEntry);
        if (state.highScores.size() > state.MAX_HIGH_SCORES) {
            state.highScores.pop_back();
        }
        if (!saveHighScores(state)) {
            SDL_Log("Warning: high scores updated in memory but could not be persisted");
        }
    }
}

bool HighScores::saveHighScores(const GameStateData& state) {
    const std::filesystem::path writablePath = getWritableHighScoresPath();

    std::error_code ec;
    if (writablePath.has_parent_path()) {
        std::filesystem::create_directories(writablePath.parent_path(), ec);
        if (ec) {
            SDL_Log("Warning: Could not create high score directory '%s': %s",
                    writablePath.parent_path().string().c_str(), ec.message().c_str());
            return false;
        }
    }

    std::ofstream file(writablePath, std::ios::trunc);
    if (!file.is_open()) {
        SDL_Log("Warning: Could not save high scores to file '%s'.", writablePath.string().c_str());
        return false;
    }

    for (const auto& entry : state.highScores) {
        file << entry.name << " " << entry.score << "\n";
    }
    if (!file.good()) {
        SDL_Log("Warning: Failed while writing high scores file '%s'.",
                writablePath.string().c_str());
        return false;
    }

    SDL_Log("High scores saved.");
    return true;
}

std::filesystem::path HighScores::getWritableHighScoresPath() const {
    if (!m_writablePathOverride.empty()) {
        return m_writablePathOverride;
    }

    return getDefaultWritableHighScoresPath();
}

std::filesystem::path HighScores::getBundledHighScoresPath() const {
    if (!m_bundledPathOverride.empty()) {
        return m_bundledPathOverride;
    }

    return getDefaultBundledHighScoresPath();
}
