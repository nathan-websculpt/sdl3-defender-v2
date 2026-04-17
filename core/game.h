#pragma once
#include <chrono>
#include <entities/health_item.h>

#include <core/game_state_data.h>
#include <core/helpers_game/game_helper.h>
#include <core/high_scores/high_scores.h>
#include <core/random/rng_service.h>
using SecondsF = std::chrono::duration<float>;

class Game {
  public:
    Game();
    explicit Game(std::uint64_t baseSeed);
    ~Game() = default;

    // non-copyable/non-movable because this is the entire game state
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    void startNewGame();
    void update(SecondsF deltaTime);
    void handleInput(const GameInput& input, SecondsF deltaTime);

    const GameStateData& getState() const {
        return m_state;
    }

    GameStateData& getState() {
        return m_state;
    }

    const Random::SeedSet& getRngSeeds() const {
        return m_rng.seeds;
    }

  private:
    GameStateData m_state;
    HighScores m_highScores;
    GameHelper m_gameHelpers;
    Random::RngStreams m_rng;

    static constexpr SecondsF OPPONENT_SPAWN_INTERVAL = SecondsF{2.0f};
    static constexpr SecondsF PLAYER_HEALTH_ITEM_SPAWN_INTERVAL = SecondsF{17.0f};
    static constexpr SecondsF WORLD_HEALTH_ITEM_SPAWN_INTERVAL = SecondsF{36.0f};

    int m_lastWindowHeight;
    SecondsF m_opponentSpawnTimer;
    bool m_prevShootState;
    SecondsF m_backspaceCooldown;

    SecondsF m_playerHealthItemSpawnTimer;
    SecondsF m_worldHealthItemSpawnTimer;

    void setLandscape();
    void applyGameOverTransition();
    void spawnOpponent();
    void spawnHealthItem(HealthItemType type);
    void updateCamera(const SDL_FRect& playerBounds);

    // inputs
    void handleEscapeKey();
    void handleInputMenu(const GameInput& input);
    void handleInputHowToPlay(const GameInput& input);
    void handleInputViewHighScores(const GameInput& input);
    void handleInputPlaying(const GameInput& input, float deltaTime);
    void handleInputGameOver(const GameInput& input, float deltaTime);
    // END: inputs

    // updates
    void updatePlayerAndProjectiles(float deltaTime);
    bool updateOpponents(float deltaTime, const SDL_FRect& pb);
    void handleSpawnsAndTimers(SecondsF deltaTime);
    // END: updates
};
