#include <algorithm>
#include <core/config.h>
#include <core/game.h>
#include <core/globals.h>
#include <core/ui/ui_button.h>
namespace
{
void submitNormalizedHighScoreFromInput(HighScores& highScores, GameStateData& state)
{
    highScores.submitHighScore(normalizeHighScoreName(state.highScoreNameInput), state);
}
} // namespace

void Game::handleEscapeKey()
{
    switch (m_state.state)
    {
    case GameStateData::State::MENU:
        m_state.running = false;
        break;
    case GameStateData::State::PLAYING:
        m_state.state = GameStateData::State::MENU;
        break;
    case GameStateData::State::GAME_OVER:
        if (m_state.waitingForHighScore)
        {
            submitNormalizedHighScoreFromInput(m_highScores, m_state);
            m_state.waitingForHighScore = false;
            m_state.state = GameStateData::State::MENU;
        }
        else
        {
            m_state.state = GameStateData::State::MENU;
        }
        break;
    case GameStateData::State::HOW_TO_PLAY:
        m_state.state = GameStateData::State::MENU;
        break;
    case GameStateData::State::VIEW_HIGH_SCORES:
        m_state.state = GameStateData::State::MENU;
        break;
    }
}

void Game::handleInputMenu(const GameInput& input)
{
    if (input.enter)
    {
        startNewGame();
    }
    else if (input.mouseClick)
    {
        const SDL_FRect playBtn = UIButtonLayout::mainMenuButtonRect(
            globals.windowWidth, globals.windowHeight, MainMenuButtonId::Play);
        const SDL_FRect howToPlayBtn = UIButtonLayout::mainMenuButtonRect(
            globals.windowWidth, globals.windowHeight, MainMenuButtonId::HowToPlay);
        const SDL_FRect viewHighScoresBtn = UIButtonLayout::mainMenuButtonRect(
            globals.windowWidth, globals.windowHeight, MainMenuButtonId::HighScores);
        const SDL_FRect exitBtn = UIButtonLayout::mainMenuButtonRect(
            globals.windowWidth, globals.windowHeight, MainMenuButtonId::Exit);

        if (uiPointInRect(input.mouseX, input.mouseY, playBtn))
        {
            startNewGame();
        }
        else if (uiPointInRect(input.mouseX, input.mouseY, howToPlayBtn))
        {
            m_state.state = GameStateData::State::HOW_TO_PLAY;
        }
        else if (uiPointInRect(input.mouseX, input.mouseY, viewHighScoresBtn))
        {
            m_state.state = GameStateData::State::VIEW_HIGH_SCORES;
        }
        else if (uiPointInRect(input.mouseX, input.mouseY, exitBtn))
        {
            m_state.running = false;
        }
    }
}

void Game::handleInputHowToPlay(const GameInput& input)
{
    const SDL_FRect closeRect = UIButtonLayout::closeButtonRect(globals.windowWidth);
    if (input.enter || (input.mouseClick && uiPointInRect(input.mouseX, input.mouseY, closeRect)))
    {
        m_state.state = GameStateData::State::MENU;
    }
}

void Game::handleInputViewHighScores(const GameInput& input)
{
    const SDL_FRect closeRect = UIButtonLayout::closeButtonRect(globals.windowWidth);
    if (input.enter || (input.mouseClick && uiPointInRect(input.mouseX, input.mouseY, closeRect)))
    {
        m_state.state = GameStateData::State::MENU;
    }
}

void Game::handleInputPlaying(const GameInput& input, float deltaTime)
{
    if (!m_state.player)
        return;

    m_state.player->setSpeedBoost(input.boost);

    if (input.shoot && !m_prevShootState)
    { // current frame: pressed, previous frame: not pressed
        m_state.player->shoot();
    }

    // update the previous state for the next frame
    m_prevShootState = input.shoot;

    float speed = m_state.player->getSpeed();
    float dx = 0;
    float dy = 0;
    if (input.moveLeft)
    {
        dx -= speed * deltaTime;
        m_state.player->setFacing(Direction::LEFT);
    }
    if (input.moveRight)
    {
        dx += speed * deltaTime;
        m_state.player->setFacing(Direction::RIGHT);
    }
    if (input.moveUp)
        dy -= speed * deltaTime;
    if (input.moveDown)
        dy += speed * deltaTime;
    m_state.player->moveBy(dx, dy);
}

void Game::handleInputGameOver(const GameInput& input, float deltaTime)
{
    if (m_state.waitingForHighScore)
    {
        if (input.charInputEvent)
        {
            char c = input.inputChar;
            if (m_state.highScoreNameInput.length() < 10 && isAllowedHighScoreInputChar(c))
            {
                m_state.highScoreNameInput += c;
            }
        }

        const SecondsF BACKSPACE_DELAY = SecondsF{0.1f};
        if (input.backspacePressed)
        {
            if (m_backspaceCooldown <= SecondsF{0.0f} && !m_state.highScoreNameInput.empty())
            {
                m_state.highScoreNameInput.pop_back();
                m_backspaceCooldown = BACKSPACE_DELAY;
            }
            else
            {
                m_backspaceCooldown =
                    std::max(SecondsF{0.0f}, m_backspaceCooldown - SecondsF{deltaTime});
            }
        }
        else
        {
            m_backspaceCooldown = SecondsF{0.0f};
        }

        // process enter/click for submission/cancellation
        if (input.enter)
        {
            submitNormalizedHighScoreFromInput(m_highScores, m_state);
            m_state.waitingForHighScore = false;
            m_state.state = GameStateData::State::MENU;
        }
        else if (input.mouseClick)
        {
            const SDL_FRect closeRect = UIButtonLayout::closeButtonRect(globals.windowWidth);
            if (uiPointInRect(input.mouseX, input.mouseY, closeRect))
            {
                submitNormalizedHighScoreFromInput(m_highScores, m_state);
                m_state.waitingForHighScore = false;
                m_state.state = GameStateData::State::MENU;
            }
        }
    }
    else
    { // not waiting for high score - game over screen
        const SDL_FRect closeRect = UIButtonLayout::closeButtonRect(globals.windowWidth);
        if (input.enter ||
            (input.mouseClick && uiPointInRect(input.mouseX, input.mouseY, closeRect)))
        {
            m_state.state = GameStateData::State::MENU;
        }
    }
}
