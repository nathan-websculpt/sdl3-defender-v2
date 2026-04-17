#include <algorithm>
#include <cmath>
#include <entities/health_item.h>
#include <entities/opponents/basic_opponent.h>
#include <entities/opponents/sniper_opponent.h>
#include <gtest/gtest.h>
#include <string>
#include <tests/test_support.h>

#include <core/game.h>
#include <core/game_state_data.h>
#include <core/ui/ui_button.h>
using TestSupport::GlobalStateFixture;

namespace {

std::size_t countHealthItemsOfType(const plf::colony<std::unique_ptr<HealthItem>>& items,
                                   HealthItemType type) {
    const auto matchCount =
        std::ranges::count_if(items, [type](const std::unique_ptr<HealthItem>& item) {
            return item && item->getType() == type;
        });
    return static_cast<std::size_t>(matchCount);
}

GameInput makeMouseClickAt(int x, int y) {
    GameInput input{};
    input.mouseClick = true;
    input.mouseX = x;
    input.mouseY = y;
    return input;
}

GameInput makeMouseClickAtCenter(const SDL_FRect& rect) {
    const int centerX = static_cast<int>(rect.x + (rect.w * 0.5f));
    const int centerY = static_cast<int>(rect.y + (rect.h * 0.5f));
    return makeMouseClickAt(centerX, centerY);
}

void prepareHighScoreSubmissionState(Game& game, const std::string& nameInput,
                                     int playerScore = 1234) {
    auto& state = game.getState();
    state.state = GameStateData::State::GAME_OVER;
    state.waitingForHighScore = true;
    state.highScoreNameInput = nameInput;
    state.highScoreIndex = 0;
    state.playerScore = playerScore;
    state.highScores.clear();
}

std::string submittedNameAfterInput(Game& game, const GameInput& input) {
    auto& state = game.getState();
    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(state.state, GameStateData::State::MENU);
    EXPECT_FALSE(state.waitingForHighScore);
    EXPECT_FALSE(state.highScores.empty());
    if (state.highScores.empty()) {
        return {};
    }
    return state.highScores.front().name;
}

class TestSniperOpponent final : public SniperOpponent {
  public:
    using SniperOpponent::SniperOpponent;

    void setFireTimer(float value) {
        m_fireTimer = value;
    }
};

} // namespace

TEST_F(GlobalStateFixture, gameStateDataDefaultScalarStateIsInitialized) {
    GameStateData state{};

    EXPECT_EQ(state.worldHealth, 0);
    EXPECT_EQ(state.playerScore, 0);
    EXPECT_FLOAT_EQ(state.cameraX, 0.0f);
}

TEST_F(GlobalStateFixture, escapeInMenuStopsGameLoop) {
    Game game(101U);
    GameInput input{};
    input.escape = true;

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_FALSE(game.getState().running);
}

TEST_F(GlobalStateFixture, enterInMenuStartsNewGame) {
    Game game(102U);
    GameInput input{};
    input.enter = true;

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    const auto& state = game.getState();

    EXPECT_EQ(state.state, GameStateData::State::PLAYING);
    EXPECT_TRUE(state.player != nullptr);
    EXPECT_EQ(state.worldHealth, state.maxWorldHealth);
    EXPECT_EQ(state.playerScore, 0);
}

TEST_F(GlobalStateFixture, escapeWhilePlayingReturnsToMenu) {
    Game game(103U);
    game.startNewGame();
    ASSERT_EQ(game.getState().state, GameStateData::State::PLAYING);

    GameInput input{};
    input.escape = true;
    game.handleInput(input, SecondsF{1.0f / 60.0f});

    EXPECT_EQ(game.getState().state, GameStateData::State::MENU);
}

TEST_F(GlobalStateFixture, menuMouseClickCanOpenHowToPlay) {
    Game game(104U);
    GameInput input{};
    input.mouseClick = true;
    input.mouseX = 450;
    input.mouseY = 420;

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::HOW_TO_PLAY);
}

TEST_F(GlobalStateFixture, mainMenuLayoutMatchesLegacyGeometry) {
    const SDL_FRect play = UIButtonLayout::mainMenuButtonRect(1000, 800, MainMenuButtonId::Play);
    const SDL_FRect howToPlay =
        UIButtonLayout::mainMenuButtonRect(1000, 800, MainMenuButtonId::HowToPlay);
    const SDL_FRect highScores =
        UIButtonLayout::mainMenuButtonRect(1000, 800, MainMenuButtonId::HighScores);
    const SDL_FRect exit = UIButtonLayout::mainMenuButtonRect(1000, 800, MainMenuButtonId::Exit);

    EXPECT_FLOAT_EQ(play.x, 400.0f);
    EXPECT_FLOAT_EQ(play.y, 340.0f);
    EXPECT_FLOAT_EQ(play.w, 200.0f);
    EXPECT_FLOAT_EQ(play.h, 50.0f);

    EXPECT_FLOAT_EQ(howToPlay.x, 400.0f);
    EXPECT_FLOAT_EQ(howToPlay.y, 400.0f);
    EXPECT_FLOAT_EQ(howToPlay.w, 200.0f);
    EXPECT_FLOAT_EQ(howToPlay.h, 50.0f);

    EXPECT_FLOAT_EQ(highScores.x, 400.0f);
    EXPECT_FLOAT_EQ(highScores.y, 460.0f);
    EXPECT_FLOAT_EQ(highScores.w, 200.0f);
    EXPECT_FLOAT_EQ(highScores.h, 50.0f);

    EXPECT_FLOAT_EQ(exit.x, 400.0f);
    EXPECT_FLOAT_EQ(exit.y, 520.0f);
    EXPECT_FLOAT_EQ(exit.w, 200.0f);
    EXPECT_FLOAT_EQ(exit.h, 50.0f);
}

TEST_F(GlobalStateFixture, closeLayoutMatchesLegacyGeometry) {
    const SDL_FRect close = UIButtonLayout::closeButtonRect(1000);

    EXPECT_FLOAT_EQ(close.x, 970.0f);
    EXPECT_FLOAT_EQ(close.y, 10.0f);
    EXPECT_FLOAT_EQ(close.w, 20.0f);
    EXPECT_FLOAT_EQ(close.h, 20.0f);
}

TEST_F(GlobalStateFixture, uiPointInRectUsesInclusiveExclusiveBounds) {
    const SDL_FRect close = UIButtonLayout::closeButtonRect(1000);

    EXPECT_TRUE(uiPointInRect(970, 10, close));
    EXPECT_TRUE(uiPointInRect(989, 29, close));

    EXPECT_FALSE(uiPointInRect(969, 10, close));
    EXPECT_FALSE(uiPointInRect(970, 9, close));
    EXPECT_FALSE(uiPointInRect(990, 10, close));
    EXPECT_FALSE(uiPointInRect(970, 30, close));
}

TEST_F(GlobalStateFixture, menuMouseClickCanStartGame) {
    Game game(115U);
    const SDL_FRect play = UIButtonLayout::mainMenuButtonRect(
        globals.windowWidth, globals.windowHeight, MainMenuButtonId::Play);
    const GameInput input = makeMouseClickAtCenter(play);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::PLAYING);
}

TEST_F(GlobalStateFixture, menuMouseClickCanOpenHighScores) {
    Game game(116U);
    const SDL_FRect highScores = UIButtonLayout::mainMenuButtonRect(
        globals.windowWidth, globals.windowHeight, MainMenuButtonId::HighScores);
    const GameInput input = makeMouseClickAtCenter(highScores);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::VIEW_HIGH_SCORES);
}

TEST_F(GlobalStateFixture, menuMouseClickCanExitGameLoop) {
    Game game(117U);
    const SDL_FRect exit = UIButtonLayout::mainMenuButtonRect(
        globals.windowWidth, globals.windowHeight, MainMenuButtonId::Exit);
    const GameInput input = makeMouseClickAtCenter(exit);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_FALSE(game.getState().running);
}

TEST_F(GlobalStateFixture, closeClickInsideReturnsToMenuFromHowToPlay) {
    Game game(118U);
    game.getState().state = GameStateData::State::HOW_TO_PLAY;
    const SDL_FRect close = UIButtonLayout::closeButtonRect(globals.windowWidth);
    const GameInput input = makeMouseClickAtCenter(close);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::MENU);
}

TEST_F(GlobalStateFixture, closeClickOutsideDoesNotReturnToMenuFromHowToPlay) {
    Game game(119U);
    game.getState().state = GameStateData::State::HOW_TO_PLAY;
    const SDL_FRect close = UIButtonLayout::closeButtonRect(globals.windowWidth);
    const GameInput input =
        makeMouseClickAt(static_cast<int>(close.x), static_cast<int>(close.y) - 1);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::HOW_TO_PLAY);
}

TEST_F(GlobalStateFixture, closeClickInsideReturnsToMenuFromViewHighScores) {
    Game game(120U);
    game.getState().state = GameStateData::State::VIEW_HIGH_SCORES;
    const SDL_FRect close = UIButtonLayout::closeButtonRect(globals.windowWidth);
    const GameInput input = makeMouseClickAtCenter(close);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::MENU);
}

TEST_F(GlobalStateFixture, viewHighScoresDoesNotFallThroughToPlayingInput) {
    Game game(122U);
    game.startNewGame();
    auto& state = game.getState();
    state.state = GameStateData::State::VIEW_HIGH_SCORES;

    const SDL_FRect before = state.player->getBounds();
    GameInput input{};
    input.moveRight = true;

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    const SDL_FRect after = state.player->getBounds();

    EXPECT_EQ(state.state, GameStateData::State::VIEW_HIGH_SCORES);
    EXPECT_FLOAT_EQ(after.x, before.x);
    EXPECT_FLOAT_EQ(after.y, before.y);
}

TEST_F(GlobalStateFixture, closeClickInsideReturnsToMenuFromGameOver) {
    Game game(121U);
    auto& state = game.getState();
    state.state = GameStateData::State::GAME_OVER;
    state.waitingForHighScore = false;
    const SDL_FRect close = UIButtonLayout::closeButtonRect(globals.windowWidth);
    const GameInput input = makeMouseClickAtCenter(close);

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(game.getState().state, GameStateData::State::MENU);
}

TEST_F(GlobalStateFixture, enterInHowToPlayReturnsToMenu) {
    Game game(105U);
    game.getState().state = GameStateData::State::HOW_TO_PLAY;

    GameInput input{};
    input.enter = true;
    game.handleInput(input, SecondsF{1.0f / 60.0f});

    EXPECT_EQ(game.getState().state, GameStateData::State::MENU);
}

TEST_F(GlobalStateFixture, updateIsNoOpWhenNotPlaying) {
    Game game(106U);
    auto& state = game.getState();

    state.state = GameStateData::State::MENU;
    state.worldHealth = 7;
    state.playerScore = 99;
    state.cameraX = 123.0f;
    state.player = std::make_unique<Player>(200.0f, 180.0f, 80.0f, 48.0f);
    state.opponents.emplace(std::make_unique<BasicOpponent>(100.0f, 100.0f, 40.0f, 40.0f));

    const SDL_FRect beforePlayerBounds = state.player->getBounds();
    const std::size_t beforeOpponentCount = TestSupport::colonySize(state.opponents);

    game.update(SecondsF{50.0f});

    EXPECT_EQ(state.state, GameStateData::State::MENU);
    EXPECT_EQ(state.worldHealth, 7);
    EXPECT_EQ(state.playerScore, 99);
    EXPECT_FLOAT_EQ(state.cameraX, 123.0f);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), beforeOpponentCount);

    const SDL_FRect afterPlayerBounds = state.player->getBounds();
    EXPECT_FLOAT_EQ(afterPlayerBounds.x, beforePlayerBounds.x);
    EXPECT_FLOAT_EQ(afterPlayerBounds.y, beforePlayerBounds.y);
}

TEST_F(GlobalStateFixture, updateWithMissingPlayerFailsSafeToMenuInRelease) {
#ifndef NDEBUG
    GTEST_SKIP() << "debug build intentionally asserts on missing player";
#else
    Game game(140U);
    game.startNewGame();
    auto& state = game.getState();

    state.player.reset();
    state.state = GameStateData::State::PLAYING;

    game.update(SecondsF{1.0f / 60.0f});

    EXPECT_EQ(state.state, GameStateData::State::MENU);
#endif
}

TEST_F(GlobalStateFixture, updateCameraClampsAtWorldBounds) {
    Game game(107U);
    game.startNewGame();
    auto& state = game.getState();

    state.player->setPosition(10.0f, 200.0f);
    game.update(SecondsF{0.0f});
    EXPECT_FLOAT_EQ(state.cameraX, 0.0f);

    const SDL_FRect bounds = state.player->getBounds();
    state.player->setPosition(static_cast<float>(Config::Game::WORLD_WIDTH) - bounds.w, 200.0f);
    game.update(SecondsF{0.0f});
    EXPECT_FLOAT_EQ(state.cameraX,
                    static_cast<float>(Config::Game::WORLD_WIDTH - globals.windowWidth));
}

TEST_F(GlobalStateFixture, updateCameraRemainsNonNegativeWhenViewportExceedsWorldWidth) {
    Game game(123U);
    game.startNewGame();
    auto& state = game.getState();

    globals.windowWidth = Config::Game::WORLD_WIDTH + 500;
    state.player->setPosition(2000.0f, 200.0f);
    game.update(SecondsF{0.0f});

    EXPECT_FLOAT_EQ(state.cameraX, 0.0f);
}

TEST_F(GlobalStateFixture, opponentSpawnTimerProgressesDeterministically) {
    Game game(108U);
    game.startNewGame();
    auto& state = game.getState();

    state.opponents.clear();
    game.update(SecondsF{1.9f});
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);

    game.update(SecondsF{0.1f});
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 1u);
}

TEST_F(GlobalStateFixture, opponentSpawnTimerPreservesOvershootRemainder) {
    Game game(124U);
    game.startNewGame();
    auto& state = game.getState();

    state.opponents.clear();
    game.update(SecondsF{2.5f});
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 1u);

    state.opponents.clear();
    game.update(SecondsF{1.5f});
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 1u);
}

TEST_F(GlobalStateFixture, opponentSpawnTimerCatchesUpAcrossMultipleIntervals) {
    Game game(125U);
    game.startNewGame();
    auto& state = game.getState();

    state.opponents.clear();
    game.update(SecondsF{4.1f});
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 2u);
}

TEST_F(GlobalStateFixture, opponentVisibilityUsesCurrentFrameCameraForFiring) {
    Game game(126U);
    game.startNewGame();
    auto& state = game.getState();

    state.opponents.clear();
    state.cameraX = 0.0f;
    state.player->setPosition(3000.0f, 200.0f);

    auto sniper = std::make_unique<TestSniperOpponent>(2600.0f, -50.0f, 35.0f, 35.0f, 0.0f);
    sniper->setFireTimer(3.95f);
    TestSniperOpponent* sniperPtr = sniper.get();
    state.opponents.emplace(std::move(sniper));

    game.update(SecondsF{0.1f});

    EXPECT_GT(state.cameraX, 0.0f);
    ASSERT_NE(sniperPtr, nullptr);
    EXPECT_GT(TestSupport::colonySize(sniperPtr->getProjectiles()), 0u);
}

TEST_F(GlobalStateFixture, playerHealthItemSpawnTimerProgressesDeterministically) {
    Game game(109U);
    game.startNewGame();
    auto& state = game.getState();

    state.healthItems.clear();
    game.update(SecondsF{16.9f});
    EXPECT_EQ(countHealthItemsOfType(state.healthItems, HealthItemType::PLAYER), 0u);

    game.update(SecondsF{0.1f});
    EXPECT_EQ(countHealthItemsOfType(state.healthItems, HealthItemType::PLAYER), 1u);
}

TEST_F(GlobalStateFixture, worldHealthItemSpawnTimerProgressesDeterministically) {
    Game game(110U);
    game.startNewGame();
    auto& state = game.getState();

    state.healthItems.clear();
    game.update(SecondsF{35.9f});
    EXPECT_EQ(countHealthItemsOfType(state.healthItems, HealthItemType::WORLD), 0u);

    game.update(SecondsF{0.1f});
    EXPECT_EQ(countHealthItemsOfType(state.healthItems, HealthItemType::WORLD), 1u);
}

TEST_F(GlobalStateFixture, updateRecomputesLandscapeAfterWindowResize) {
    Game game(111U);
    game.startNewGame();
    auto& state = game.getState();

    ASSERT_FALSE(state.landscape.empty());
    const float oldFrontY = state.landscape.front().y;

    globals.windowHeight = 900;
    game.update(SecondsF{0.0f});

    ASSERT_FALSE(state.landscape.empty());
    EXPECT_FLOAT_EQ(oldFrontY, 780.0f);
    EXPECT_FLOAT_EQ(state.landscape.front().y, 880.0f);
    EXPECT_FLOAT_EQ(state.landscape.back().y, 860.0f);

    globals.windowHeight = 1;
    game.update(SecondsF{0.0f});

    ASSERT_FALSE(state.landscape.empty());
    EXPECT_TRUE(std::isfinite(state.landscape.front().y));
    EXPECT_TRUE(std::isfinite(state.landscape.back().y));
    EXPECT_FLOAT_EQ(state.landscape.front().y, -19.0f);
    EXPECT_FLOAT_EQ(state.landscape.back().y, -39.0f);
}

TEST_F(GlobalStateFixture, gameOverAcceptsValidCharactersAndRejectsInvalidOnes) {
    Game game(112U);
    auto& state = game.getState();
    state.state = GameStateData::State::GAME_OVER;
    state.waitingForHighScore = true;
    state.highScoreNameInput = "AB";
    state.highScoreIndex = 4;

    GameInput input{};
    input.charInputEvent = true;
    input.inputChar = 'C';
    game.handleInput(input, SecondsF{1.0f / 60.0f});

    EXPECT_EQ(state.highScoreNameInput, "ABC");
    EXPECT_EQ(state.state, GameStateData::State::GAME_OVER);
    EXPECT_TRUE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, 4);

    input.inputChar = '!';
    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(state.highScoreNameInput, "ABC");

    input.inputChar = ' ';
    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(state.highScoreNameInput, "ABC");
    EXPECT_EQ(state.state, GameStateData::State::GAME_OVER);
    EXPECT_TRUE(state.waitingForHighScore);
}

TEST_F(GlobalStateFixture, gameOverSubmissionPathsTrimNamesIdentically) {
    Game enterGame(127U);
    prepareHighScoreSubmissionState(enterGame, " \tAce\t ");
    GameInput enterInput{};
    enterInput.enter = true;
    const std::string enterName = submittedNameAfterInput(enterGame, enterInput);

    Game escapeGame(128U);
    prepareHighScoreSubmissionState(escapeGame, " \tAce\t ");
    GameInput escapeInput{};
    escapeInput.escape = true;
    const std::string escapeName = submittedNameAfterInput(escapeGame, escapeInput);

    Game closeGame(129U);
    prepareHighScoreSubmissionState(closeGame, " \tAce\t ");
    const SDL_FRect closeRect = UIButtonLayout::closeButtonRect(globals.windowWidth);
    const GameInput closeInput = makeMouseClickAtCenter(closeRect);
    const std::string closeName = submittedNameAfterInput(closeGame, closeInput);

    EXPECT_EQ(enterName, "Ace");
    EXPECT_EQ(escapeName, "Ace");
    EXPECT_EQ(closeName, "Ace");
}

TEST_F(GlobalStateFixture, gameOverSubmissionPathsApplyAnonFallbackIdentically) {
    Game enterGame(130U);
    prepareHighScoreSubmissionState(enterGame, " \t ");
    GameInput enterInput{};
    enterInput.enter = true;
    const std::string enterName = submittedNameAfterInput(enterGame, enterInput);

    Game escapeGame(131U);
    prepareHighScoreSubmissionState(escapeGame, " \t ");
    GameInput escapeInput{};
    escapeInput.escape = true;
    const std::string escapeName = submittedNameAfterInput(escapeGame, escapeInput);

    Game closeGame(132U);
    prepareHighScoreSubmissionState(closeGame, " \t ");
    const SDL_FRect closeRect = UIButtonLayout::closeButtonRect(globals.windowWidth);
    const GameInput closeInput = makeMouseClickAtCenter(closeRect);
    const std::string closeName = submittedNameAfterInput(closeGame, closeInput);

    EXPECT_EQ(enterName, "ANON");
    EXPECT_EQ(escapeName, "ANON");
    EXPECT_EQ(closeName, "ANON");
}

TEST_F(GlobalStateFixture, gameOverNameInputRespectsMaxLength) {
    Game game(113U);
    auto& state = game.getState();
    state.state = GameStateData::State::GAME_OVER;
    state.waitingForHighScore = true;
    state.highScoreNameInput = "ABCDEFGHIJ";
    state.highScoreIndex = 2;

    GameInput input{};
    input.charInputEvent = true;
    input.inputChar = 'K';
    game.handleInput(input, SecondsF{1.0f / 60.0f});

    EXPECT_EQ(state.highScoreNameInput, "ABCDEFGHIJ");
    EXPECT_EQ(state.state, GameStateData::State::GAME_OVER);
    EXPECT_TRUE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, 2);
}

TEST_F(GlobalStateFixture, gameOverBackspaceCooldownUsesDeltaTime) {
    Game game(114U);
    auto& state = game.getState();
    state.state = GameStateData::State::GAME_OVER;
    state.waitingForHighScore = true;
    state.highScoreNameInput = "AB";
    state.highScoreIndex = 6;

    GameInput input{};
    input.backspacePressed = true;

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(state.highScoreNameInput, "A");

    game.handleInput(input, SecondsF{0.05f});
    EXPECT_EQ(state.highScoreNameInput, "A");

    game.handleInput(input, SecondsF{0.05f});
    EXPECT_EQ(state.highScoreNameInput, "A");

    game.handleInput(input, SecondsF{1.0f / 60.0f});
    EXPECT_EQ(state.highScoreNameInput, "");

    EXPECT_EQ(state.state, GameStateData::State::GAME_OVER);
    EXPECT_TRUE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, 6);
}
