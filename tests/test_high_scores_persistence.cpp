#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <tests/test_support.h>
#include <vector>

#include <core/game_state_data.h>
#include <core/high_scores/high_scores.h>
using TestSupport::GlobalStateFixture;

namespace {

bool isSortedDescendingByScore(const std::vector<GameStateData::HighScore>& highScores) {
    return std::is_sorted(highScores.begin(), highScores.end(),
                          [](const GameStateData::HighScore& a, const GameStateData::HighScore& b) {
                              return a.score > b.score;
                          });
}

std::vector<GameStateData::HighScore> makeOverfullUnsortedScores() {
    return {{"S01", 100}, {"S02", 450}, {"S03", 300}, {"S04", 800}, {"S05", 700}, {"S06", 200},
            {"S07", 650}, {"S08", 500}, {"S09", 900}, {"S10", 50},  {"S11", 600}};
}

void writeFileText(const std::filesystem::path& filePath, const std::string& contents) {
    std::ofstream file(filePath, std::ios::trunc);
    ASSERT_TRUE(file.is_open());
    file << contents;
    ASSERT_TRUE(file.good());
}

class HighScorePersistenceFixture : public GlobalStateFixture {
  protected:
    void SetUp() override {
        GlobalStateFixture::SetUp();
        const auto uniqueSuffix =
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        m_testDir = std::filesystem::temp_directory_path() /
                    std::filesystem::path("sdl3defender_high_scores_" + uniqueSuffix);
        std::filesystem::create_directories(m_testDir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(m_testDir, ec);
    }

    std::filesystem::path path(const std::string& name) const {
        return m_testDir / std::filesystem::path(name);
    }

    std::filesystem::path m_testDir;
};

} // namespace

TEST_F(HighScorePersistenceFixture, loadValidFileReplacesStateAndEnforcesOrderingAndCap) {
    const std::filesystem::path writablePath = path("valid_high_scores.txt");
    const std::filesystem::path bundledPath = path("unused_bundled.txt");
    writeFileText(
        writablePath,
        "A 100\nB 900\nC 300\nD 1200\nE 1100\nF 400\nG 500\nH 600\nI 700\nJ 800\nK 200\nL 1000\n");

    GameStateData state{};
    state.highScores = {{"OLD", 42}};

    HighScores highScores(writablePath, bundledPath);
    highScores.loadHighScores(state);

    ASSERT_EQ(state.highScores.size(), static_cast<std::size_t>(state.MAX_HIGH_SCORES));
    EXPECT_TRUE(isSortedDescendingByScore(state.highScores));
    EXPECT_EQ(state.highScores.front().score, 1200);
    EXPECT_EQ(state.highScores.back().score, 100);
}

TEST_F(HighScorePersistenceFixture, loadMalformedFileKeepsExistingScoresAndPreservesInvariants) {
    const std::filesystem::path writablePath = path("malformed_high_scores.txt");
    const std::filesystem::path bundledPath = path("missing_bundled.txt");
    writeFileText(writablePath, "AAA 100\nBROKEN_LINE\nBBB 200\n");

    GameStateData state{};
    state.highScores = {{"LOW", 100}, {"HIGH", 900}, {"MID", 400}};

    HighScores highScores(writablePath, bundledPath);
    highScores.loadHighScores(state);

    ASSERT_EQ(state.highScores.size(), static_cast<std::size_t>(3));
    EXPECT_TRUE(isSortedDescendingByScore(state.highScores));
    EXPECT_EQ(state.highScores[0].name, "HIGH");
    EXPECT_EQ(state.highScores[1].name, "MID");
    EXPECT_EQ(state.highScores[2].name, "LOW");
}

TEST_F(HighScorePersistenceFixture, missingFileDoesNotClearExistingScoresAndPreservesInvariants) {
    const std::filesystem::path writablePath = path("missing_writable.txt");
    const std::filesystem::path bundledPath = path("missing_bundled.txt");

    GameStateData state{};
    state.highScores = makeOverfullUnsortedScores();

    HighScores highScores(writablePath, bundledPath);
    highScores.loadHighScores(state);

    ASSERT_EQ(state.highScores.size(), static_cast<std::size_t>(state.MAX_HIGH_SCORES));
    EXPECT_TRUE(isSortedDescendingByScore(state.highScores));
    EXPECT_EQ(state.highScores.front().score, 900);
    EXPECT_EQ(state.highScores.back().score, 100);
}

TEST_F(HighScorePersistenceFixture,
       loadFailureFromNonFilePathKeepsExistingScoresAndPreservesInvariants) {
    const std::filesystem::path nonFilePath = path("non_file_path");
    std::filesystem::create_directories(nonFilePath);

    GameStateData state{};
    state.highScores = {{"AA", 10}, {"BB", 30}, {"CC", 20}};

    HighScores highScores(nonFilePath, nonFilePath);
    highScores.loadHighScores(state);

    ASSERT_EQ(state.highScores.size(), static_cast<std::size_t>(3));
    EXPECT_TRUE(isSortedDescendingByScore(state.highScores));
    EXPECT_EQ(state.highScores[0].score, 30);
    EXPECT_EQ(state.highScores[1].score, 20);
    EXPECT_EQ(state.highScores[2].score, 10);
}

TEST_F(HighScorePersistenceFixture, saveFailureAfterSubmitKeepsInMemoryHighScoreListValid) {
    const std::filesystem::path nonFilePath = path("save_target_directory");
    std::filesystem::create_directories(nonFilePath);

    GameStateData state{};
    state.highScores = {{"AAA", 1000}, {"BBB", 900}, {"CCC", 800}, {"DDD", 700}, {"EEE", 600},
                        {"FFF", 500},  {"GGG", 400}, {"HHH", 300}, {"III", 200}, {"JJJ", 100}};
    state.playerScore = 650;

    HighScores highScores(nonFilePath, nonFilePath);
    highScores.submitHighScore("NEW", state);

    ASSERT_EQ(state.highScores.size(), static_cast<std::size_t>(state.MAX_HIGH_SCORES));
    EXPECT_TRUE(isSortedDescendingByScore(state.highScores));
    EXPECT_EQ(state.highScores[4].name, "NEW");
    EXPECT_EQ(state.highScores[4].score, 650);
    EXPECT_EQ(state.highScores.back().score, 200);
    EXPECT_TRUE(std::filesystem::is_directory(nonFilePath));
}
