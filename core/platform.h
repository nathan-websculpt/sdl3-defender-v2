#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <string>

#include <core/game_state_data.h>
#include <core/helpers_platform/projectile_clipping.h>
#include <core/helpers_platform_rendering/render_main.h>
#include <core/managers/font_manager.h>
#include <core/managers/texture_manager.h>
class Game;

class Platform {
  public:
    Platform();
    ~Platform();

    // delete copy operations
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    // delete move operations
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;

    bool initialize();
    void run(Game& sim);
    void shutdown();

  private:
    SDL_Window* m_window = nullptr;
    bool m_running;
    bool m_textInputActive; // track if text input is currently active
    bool m_soundManagerInitialized;

    SDL_AudioDeviceID m_audioDeviceID;
    SDL_AudioSpec m_audioSpec;

    // input
    GameInput pollInput(const GameStateData& state);
    void updateTextInputState(const GameStateData& state);
};
