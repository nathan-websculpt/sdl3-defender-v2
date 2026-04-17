#include <core/game_state_data.h>
#include <core/high_scores/high_scores.h>
#include <gtest/gtest.h>
#include <tests/test_support.h>
#include <vector>
using TestSupport::GlobalStateFixture;

namespace
{

std::vector<GameStateData::HighScore> makeFullSortedScores()
{
    return {{"AAA", 1000}, {"BBB", 900}, {"CCC", 800}, {"DDD", 700}, {"EEE", 600},
            {"FFF", 500},  {"GGG", 400}, {"HHH", 300}, {"III", 200}, {"JJJ", 100}};
}

} // namespace

TEST_F(GlobalStateFixture, emptyListAlwaysAcceptsHighScore)
{
    GameStateData state{};
    state.playerScore = 250;

    const HighScores highScores;
    EXPECT_TRUE(highScores.isHighScore(state));
    EXPECT_EQ(highScores.getHighScoreIndex(state), 0);
}

TEST_F(GlobalStateFixture, partialListAcceptsAndAppendsLowerScore)
{
    GameStateData state{};
    state.highScores = {{"AAA", 1000}, {"BBB", 900}};
    state.playerScore = 10;

    const HighScores highScores;
    EXPECT_TRUE(highScores.isHighScore(state));
    EXPECT_EQ(highScores.getHighScoreIndex(state), 2);
}

TEST_F(GlobalStateFixture, fullListInsertsAtExpectedRank)
{
    GameStateData state{};
    state.highScores = makeFullSortedScores();
    state.playerScore = 550;

    const HighScores highScores;
    EXPECT_TRUE(highScores.isHighScore(state));
    EXPECT_EQ(highScores.getHighScoreIndex(state), 5);
}

TEST_F(GlobalStateFixture, fullListRejectsScoresBelowLowest)
{
    GameStateData state{};
    state.highScores = makeFullSortedScores();
    state.playerScore = 90;

    const HighScores highScores;
    EXPECT_FALSE(highScores.isHighScore(state));
    EXPECT_EQ(highScores.getHighScoreIndex(state), -1);
}
