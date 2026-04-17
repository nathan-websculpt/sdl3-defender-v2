#include <entities/health_item.h>
#include <entities/opponents/basic_opponent.h>
#include <entities/opponents/sniper_opponent.h>
#include <entities/player.h>
#include <gtest/gtest.h>
#include <limits>
#include <tests/test_support.h>

#include <core/game.h>
#include <core/game_state_data.h>
#include <core/helpers_game/collision_handler.h>
#include <core/helpers_game/game_helper.h>
#include <core/high_scores/high_scores.h>
using TestSupport::GlobalStateFixture;

TEST_F(GlobalStateFixture, projectileCollisionAwardsScoreAndRemovesShot) {
    auto rng = Random::makeDeterministicStreams(1U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 10;
    state.playerScore = 0;
    state.cameraX = 0.0f;
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.player = std::make_unique<Player>(0.0f, 0.0f, 80.0f, 48.0f);

    state.opponents.emplace(std::make_unique<SniperOpponent>(100.0f, 100.0f, 35.0f, 35.0f, 0.0f));
    state.player->getProjectiles().emplace(110.0f, 110.0f, 1.0f, 600.0f);

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_TRUE(completed);
    EXPECT_EQ(state.playerScore, 100);
    EXPECT_EQ(TestSupport::colonySize(state.player->getProjectiles()), 0u);
    ASSERT_EQ(TestSupport::colonySize(state.opponents), 1u);
    auto opponentIt = state.opponents.begin();
    ASSERT_TRUE(*opponentIt);
    EXPECT_FALSE((*opponentIt)->isAlive());
}

TEST_F(GlobalStateFixture, projectileCollisionSaturatesScoreAtIntMax) {
    auto rng = Random::makeDeterministicStreams(8U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 10;
    state.playerScore = std::numeric_limits<int>::max() - 50;
    state.cameraX = 0.0f;
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.player = std::make_unique<Player>(0.0f, 0.0f, 80.0f, 48.0f);

    state.opponents.emplace(std::make_unique<SniperOpponent>(100.0f, 100.0f, 35.0f, 35.0f, 0.0f));
    state.player->getProjectiles().emplace(110.0f, 110.0f, 1.0f, 600.0f);

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_TRUE(completed);
    EXPECT_EQ(state.playerScore, std::numeric_limits<int>::max());
}

TEST_F(GlobalStateFixture, projectileCollisionAtBeamEdgeStillHitsWhenBoundsStraddleClipPoint) {
    auto rng = Random::makeDeterministicStreams(9U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 10;
    state.playerScore = 0;
    state.cameraX = 0.0f;
    state.landscape = {{0.0f, 700.0f}, {100.0f, 600.0f}, {200.0f, 700.0f}};
    state.player = std::make_unique<Player>(0.0f, 0.0f, 80.0f, 48.0f);

    state.opponents.emplace(std::make_unique<SniperOpponent>(49.0f, 650.0f, 35.0f, 35.0f, 0.0f));
    state.player->getProjectiles().emplace(49.0f, 650.0f, 1.0f, 600.0f);

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_TRUE(completed);
    EXPECT_EQ(state.playerScore, 100);
    EXPECT_EQ(TestSupport::colonySize(state.player->getProjectiles()), 0u);
    ASSERT_EQ(TestSupport::colonySize(state.opponents), 1u);
    auto opponentIt = state.opponents.begin();
    ASSERT_TRUE(*opponentIt);
    EXPECT_FALSE((*opponentIt)->isAlive());
}

TEST_F(GlobalStateFixture, playerHealthItemCollisionRestoresPlayerHealth) {
    auto rng = Random::makeDeterministicStreams(2U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 5;
    state.playerScore = 0;
    state.cameraX = 0.0f;
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.player = std::make_unique<Player>(100.0f, 100.0f, 80.0f, 48.0f);
    state.player->takeDamage(4);
    ASSERT_EQ(state.player->getHealth(), 6);

    state.healthItems.emplace(std::make_unique<HealthItem>(110.0f, 110.0f, 30.0f, 30.0f,
                                                           HealthItemType::PLAYER, "texture"));

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_TRUE(completed);
    EXPECT_EQ(state.player->getHealth(), state.player->getMaxHealth());
    EXPECT_EQ(TestSupport::colonySize(state.healthItems), 0u);
}

TEST_F(GlobalStateFixture, worldHealthItemCollisionRestoresWorldHealth) {
    auto rng = Random::makeDeterministicStreams(3U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 3;
    state.playerScore = 0;
    state.cameraX = 0.0f;
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.player = std::make_unique<Player>(100.0f, 100.0f, 80.0f, 48.0f);

    state.healthItems.emplace(std::make_unique<HealthItem>(110.0f, 110.0f, 30.0f, 30.0f,
                                                           HealthItemType::WORLD, "texture"));

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_TRUE(completed);
    EXPECT_EQ(state.worldHealth, state.maxWorldHealth);
    EXPECT_EQ(TestSupport::colonySize(state.healthItems), 0u);
}

TEST_F(GlobalStateFixture, bodyCollisionCanKillPlayerAndReportGameOver) {
    auto rng = Random::makeDeterministicStreams(4U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 8;
    state.playerScore = 0;
    state.cameraX = 0.0f;
    state.highScores = {{"AAA", 1000}};
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.player = std::make_unique<Player>(100.0f, 100.0f, 80.0f, 48.0f);
    state.player->takeDamage(9);
    ASSERT_EQ(state.player->getHealth(), 1);

    state.opponents.emplace(std::make_unique<SniperOpponent>(110.0f, 110.0f, 35.0f, 35.0f, 0.0f));
    state.healthItems.emplace(std::make_unique<HealthItem>(110.0f, 110.0f, 30.0f, 30.0f,
                                                           HealthItemType::PLAYER, "texture"));

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_FALSE(completed);
    EXPECT_EQ(state.state, GameStateData::State::MENU);
    EXPECT_FALSE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, -1);
    EXPECT_TRUE(state.highScoreNameInput.empty());
    EXPECT_FALSE(state.player->isAlive());
    EXPECT_EQ(state.playerScore, 100);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);

    // collision processing should early-return once player dies
    EXPECT_EQ(TestSupport::colonySize(state.healthItems), 1u);
}

TEST_F(GlobalStateFixture, opponentProjectileCanKillPlayerAndEarlyReturnSkipsHealthItems) {
    auto rng = Random::makeDeterministicStreams(5U);

    GameStateData state{};
    state.maxWorldHealth = 10;
    state.worldHealth = 8;
    state.playerScore = 0;
    state.cameraX = 0.0f;
    state.highScores = {{"AAA", 1000}};
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.player = std::make_unique<Player>(100.0f, 100.0f, 80.0f, 48.0f);
    state.player->takeDamage(9);
    ASSERT_EQ(state.player->getHealth(), 1);

    auto sniper = std::make_unique<SniperOpponent>(300.0f, 300.0f, 35.0f, 35.0f, 0.0f);
    sniper->getProjectiles().emplace(110.0f, 110.0f, 110.0f, 110.0f, 250.0f);
    state.opponents.emplace(std::move(sniper));
    state.healthItems.emplace(std::make_unique<HealthItem>(110.0f, 110.0f, 30.0f, 30.0f,
                                                           HealthItemType::WORLD, "texture"));

    const GameHelper helper(state.landscape);
    const HighScores highScores;
    const SDL_FRect playerBounds = state.player->getBounds();

    const bool completed = CollisionHandler::processAllCollisions(state, helper, highScores,
                                                                  playerBounds, rng.fxEngine);

    EXPECT_FALSE(completed);
    EXPECT_EQ(state.state, GameStateData::State::MENU);
    EXPECT_FALSE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, -1);
    EXPECT_TRUE(state.highScoreNameInput.empty());
    EXPECT_FALSE(state.player->isAlive());
    EXPECT_EQ(state.playerScore, 0);

    ASSERT_EQ(TestSupport::colonySize(state.opponents), 1u);
    auto opponentIt = state.opponents.begin();
    ASSERT_TRUE(*opponentIt);
    EXPECT_EQ(TestSupport::colonySize((*opponentIt)->getProjectiles()), 0u);

    // collision processing should early-return once player dies
    EXPECT_EQ(TestSupport::colonySize(state.healthItems), 1u);
}

TEST_F(GlobalStateFixture, basicOpponentGroundContactReducesWorldHealth) {
    Game game(6U);
    game.startNewGame();
    auto& state = game.getState();

    state.worldHealth = 5;
    state.playerScore = 0;
    state.highScores = {{"AAA", 1000}};
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.opponents.emplace(std::make_unique<BasicOpponent>(100.0f, 680.0f, 40.0f, 40.0f));

    game.update(SecondsF{0.0f});

    EXPECT_EQ(state.state, GameStateData::State::PLAYING);
    EXPECT_EQ(state.worldHealth, 4);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
}

TEST_F(GlobalStateFixture, nonBasicGroundContactDoesNotReduceWorldHealth) {
    Game game(19U);
    game.startNewGame();
    auto& state = game.getState();

    state.worldHealth = 5;
    state.playerScore = 0;
    state.highScores = {{"AAA", 1000}};
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.opponents.emplace(std::make_unique<SniperOpponent>(100.0f, 680.0f, 35.0f, 35.0f, 0.0f));

    game.update(SecondsF{0.0f});

    EXPECT_EQ(state.state, GameStateData::State::PLAYING);
    EXPECT_EQ(state.worldHealth, 5);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
}

TEST_F(GlobalStateFixture, basicOpponentGroundContactAtThresholdTriggersGameOver) {
    Game game(7U);
    game.startNewGame();
    auto& state = game.getState();

    state.worldHealth = 1;
    state.playerScore = 0;
    state.highScores = {{"AAA", 1000}};
    state.highScoreIndex = -1;
    state.waitingForHighScore = false;
    state.highScoreNameInput = "TEMP";
    state.cameraX = 0.0f;
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.opponents.emplace(std::make_unique<BasicOpponent>(100.0f, 680.0f, 40.0f, 40.0f));

    game.update(SecondsF{0.0f});

    EXPECT_EQ(state.state, GameStateData::State::GAME_OVER);
    EXPECT_EQ(state.worldHealth, 0);
    EXPECT_TRUE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, 1);
    EXPECT_TRUE(state.highScoreNameInput.empty());

    // camera now updates before opponent handling, even when world-health game-over triggers
    EXPECT_GT(state.cameraX, 0.0f);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 1u);
}

TEST_F(GlobalStateFixture, collisionTriggeredPlayerDeathTransitionsAtTopLevelUpdate) {
    Game game(20U);
    game.startNewGame();
    auto& state = game.getState();

    state.worldHealth = 10;
    state.playerScore = 0;
    state.highScores = {{"AAA", 1000}};
    state.highScoreIndex = -1;
    state.waitingForHighScore = false;
    state.highScoreNameInput = "TEMP";
    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.healthItems.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.player->setPosition(100.0f, 100.0f);
    state.player->takeDamage(9);
    ASSERT_EQ(state.player->getHealth(), 1);

    state.opponents.emplace(std::make_unique<SniperOpponent>(110.0f, 110.0f, 35.0f, 35.0f, 0.0f));

    game.update(SecondsF{0.0f});

    EXPECT_EQ(state.state, GameStateData::State::GAME_OVER);
    EXPECT_TRUE(state.waitingForHighScore);
    EXPECT_EQ(state.highScoreIndex, 1);
    EXPECT_TRUE(state.highScoreNameInput.empty());
    EXPECT_FALSE(state.player->isAlive());
    EXPECT_EQ(state.playerScore, 100);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
}

TEST_F(GlobalStateFixture, staleBoundsAfterClamp_usesPostClampPlayerBounds) {
    Game game(14U);
    game.startNewGame();
    auto& state = game.getState();

    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.player->setPosition(200.0f, 0.0f);

    auto sniper = std::make_unique<SniperOpponent>(500.0f, 100.0f, 35.0f, 35.0f, 0.0f);
    sniper->getProjectiles().emplace(210.0f, 10.0f, 210.0f, 10.0f, 250.0f);
    state.opponents.emplace(std::move(sniper));

    const int healthBefore = state.player->getHealth();

    game.update(SecondsF{0.0f});

    const SDL_FRect clampedBounds = state.player->getBounds();
    EXPECT_FLOAT_EQ(clampedBounds.y, static_cast<float>(Config::Game::HUD_HEIGHT));
    EXPECT_EQ(state.player->getHealth(), healthBefore);
}

TEST_F(GlobalStateFixture, expiredProjectileIsRemovedBeforeCollisionPhase) {
    Game game(21U);
    game.startNewGame();
    auto& state = game.getState();

    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.playerScore = 0;

    state.opponents.emplace(std::make_unique<SniperOpponent>(100.0f, 100.0f, 35.0f, 35.0f, 0.0f));
    state.player->getProjectiles().emplace(130.0f, 130.0f, 1.0f, 0.0f);

    game.update(SecondsF{0.6f});

    EXPECT_EQ(state.playerScore, 0);
    EXPECT_EQ(TestSupport::colonySize(state.player->getProjectiles()), 0u);
    ASSERT_EQ(TestSupport::colonySize(state.opponents), 1u);
    auto opponentIt = state.opponents.begin();
    ASSERT_TRUE(*opponentIt);
    EXPECT_TRUE((*opponentIt)->isAlive());
}

TEST_F(GlobalStateFixture, projectileKill_lifecycleScoreExplodeOnce_removeNextUpdate) {
    Game game(15U);
    game.startNewGame();
    auto& state = game.getState();

    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.playerScore = 0;

    state.opponents.emplace(std::make_unique<SniperOpponent>(100.0f, 100.0f, 35.0f, 35.0f, 0.0f));
    state.player->getProjectiles().emplace(110.0f, 110.0f, 1.0f, 600.0f);

    game.update(SecondsF{0.0f});

    EXPECT_EQ(state.playerScore, 100);
    ASSERT_EQ(TestSupport::colonySize(state.opponents), 1u);
    auto firstOpponent = state.opponents.begin();
    ASSERT_TRUE(*firstOpponent);
    EXPECT_FALSE((*firstOpponent)->isAlive());

    const std::size_t particlesAfterFirstUpdate = TestSupport::colonySize(state.particles);
    EXPECT_EQ(particlesAfterFirstUpdate, 346u);

    game.update(SecondsF{0.0f});

    EXPECT_EQ(state.playerScore, 100);
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
    const std::size_t particlesAfterSecondUpdate = TestSupport::colonySize(state.particles);
    EXPECT_EQ(particlesAfterSecondUpdate, particlesAfterFirstUpdate + 1u);
}

TEST_F(GlobalStateFixture, deathPath_projectileKill_preservesOpponentProjectilesUntilRemoval) {
    Game game(16U);
    game.startNewGame();
    auto& state = game.getState();

    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.playerScore = 0;

    auto sniper = std::make_unique<SniperOpponent>(100.0f, 100.0f, 35.0f, 35.0f, 0.0f);
    sniper->getProjectiles().emplace(300.0f, 200.0f, 300.0f, 200.0f, 250.0f);
    state.opponents.emplace(std::move(sniper));
    state.player->getProjectiles().emplace(110.0f, 110.0f, 1.0f, 600.0f);

    game.update(SecondsF{0.0f});

    ASSERT_EQ(TestSupport::colonySize(state.opponents), 1u);
    auto opponentIt = state.opponents.begin();
    ASSERT_TRUE(*opponentIt);
    EXPECT_FALSE((*opponentIt)->isAlive());
    EXPECT_EQ(TestSupport::colonySize((*opponentIt)->getProjectiles()), 1u);

    game.update(SecondsF{0.0f});
    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
}

TEST_F(GlobalStateFixture, deathPath_bodyCollision_destroysOpponentAndProjectilesSameFrame) {
    Game game(17U);
    game.startNewGame();
    auto& state = game.getState();

    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.player->setPosition(100.0f, 100.0f);
    state.playerScore = 0;

    auto sniper = std::make_unique<SniperOpponent>(110.0f, 110.0f, 35.0f, 35.0f, 0.0f);
    sniper->getProjectiles().emplace(300.0f, 300.0f, 300.0f, 300.0f, 250.0f);
    state.opponents.emplace(std::move(sniper));

    game.update(SecondsF{0.0f});

    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
    EXPECT_EQ(state.playerScore, 100);
}

TEST_F(GlobalStateFixture,
       deathPath_groundImpact_destroysOpponentAndProjectilesBeforeCollisionPhase) {
    Game game(18U);
    game.startNewGame();
    auto& state = game.getState();

    state.landscape = TestSupport::makeFlatLandscape(700.0f);
    state.opponents.clear();
    state.particles.clear();
    state.player->getProjectiles().clear();
    state.player->setPosition(100.0f, 100.0f);
    state.playerScore = 0;
    state.worldHealth = 10;

    auto basic = std::make_unique<BasicOpponent>(100.0f, 680.0f, 40.0f, 40.0f);
    basic->getProjectiles().emplace(110.0f, 110.0f, 110.0f, 110.0f, 250.0f);
    state.opponents.emplace(std::move(basic));

    const int healthBefore = state.player->getHealth();

    game.update(SecondsF{0.0f});

    EXPECT_EQ(TestSupport::colonySize(state.opponents), 0u);
    EXPECT_EQ(state.player->getHealth(), healthBefore);
    EXPECT_EQ(state.worldHealth, 9);
}
