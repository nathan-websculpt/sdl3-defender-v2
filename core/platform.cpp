#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <core/game.h>
#include <core/globals.h>
#include <core/helpers_platform_rendering/render_helper.h>
#include <core/managers/sound_manager.h>
#include <core/platform.h>
#include <cstring>
#include <entities/health_item.h>
#include <limits>
#include <sstream>
Platform::Platform()
    : m_running(true), m_textInputActive(false), m_soundManagerInitialized(false),
      m_audioDeviceID(0)
{
}

Platform::~Platform()
{
    shutdown();
}

bool Platform::initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("unable to initialize sdl: %s", SDL_GetError());
        return false;
    }
    if (!TTF_Init())
    {
        SDL_Log("unable to initialize sdl_ttf: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    m_window = SDL_CreateWindow("sdl3 defender", globals.windowWidth, globals.windowHeight,
                                SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        SDL_Log("failed to create window: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    globals.renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!globals.renderer)
    {
        SDL_Log("failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // attempt to enable VSync using SDL_SetRenderVSync
    if (SDL_SetRenderVSync(globals.renderer, 1) != 0)
    { // 1 enables VSync, 0 disables
        // if setting VSync fails, log it but continue (maybe VSync isn't supported on this
        // display/driver)
        SDL_Log("Warning: Failed to enable VSync: %s. Running without VSync.", SDL_GetError());
    }
    else
    {
        SDL_Log("VSync successfully enabled.");
    }

    // audio device initialization
    // define the desired audio format using SDL3 enums
    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);
    desired_spec.freq = 44100;
    desired_spec.format = SDL_AUDIO_F32; // SDL3 enum for floating point format
    desired_spec.channels = 2;           // stereo

    m_audioDeviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!m_audioDeviceID)
    {
        SDL_Log("Failed to open default audio device! SDL_GetError: %s", SDL_GetError());
        SDL_Log("Desired audio spec: %d Hz, %s, %d channels", desired_spec.freq,
                SDL_GetAudioFormatName(desired_spec.format), desired_spec.channels);
    }
    else
    {
        m_audioSpec = desired_spec;
        SDL_Log("Successfully opened audio device %u with actual spec: %d Hz, %s, %d channels",
                static_cast<unsigned int>(m_audioDeviceID), m_audioSpec.freq,
                SDL_GetAudioFormatName(m_audioSpec.format), m_audioSpec.channels);

        SDL_PauseAudioDevice(m_audioDeviceID); // unpauses (starts) the device
        SDL_Log("Started audio device %u (unpaused).", static_cast<unsigned int>(m_audioDeviceID));
    }

    // sound manager initialization
    if (m_audioDeviceID)
    {
        if (!SoundManager::getInstance().initialize(m_audioDeviceID, m_audioSpec))
        {
            SDL_Log("Failed to initialize SoundManager even though audio device was opened.");
            m_soundManagerInitialized = false;
        }
        else
        {
            SDL_Log("SoundManager initialized successfully.");
            m_soundManagerInitialized = true;
        }
    }
    else
    {
        SDL_Log("Skipping SoundManager initialization due to audio device failure.");
        m_soundManagerInitialized = false;
    }

    return true;
}

void Platform::shutdown()
{
    if (m_textInputActive)
    {
        SDL_StopTextInput(m_window); // stop text input
        m_textInputActive = false;
        SDL_Log("Platform: Text input STOPPED during shutdown.");
    }

    const bool shouldCloseAudioDeviceManually =
        (m_audioDeviceID != 0) && !m_soundManagerInitialized;

    SoundManager::getInstance().shutdown();
    if (m_soundManagerInitialized && m_audioDeviceID)
    {
        SDL_Log("Audio device %u was closed by SoundManager shutdown.",
                static_cast<unsigned int>(m_audioDeviceID));
        m_audioDeviceID = 0;
    }
    m_soundManagerInitialized = false;

    RenderHelper::clearTextCache();
    TextureManager::getInstance().clearCache();
    FontManager::getInstance().clearCache();

    // audio device shutdown
    if (shouldCloseAudioDeviceManually)
    {
        // explicitly pause the audio device before closing it.
        SDL_PauseAudioDevice(m_audioDeviceID);
        SDL_Log("Paused audio device %u before closing.",
                static_cast<unsigned int>(m_audioDeviceID));

        SDL_AudioDeviceID closedAudioDeviceID = m_audioDeviceID;
        SDL_CloseAudioDevice(closedAudioDeviceID);
        m_audioDeviceID = 0;
        SDL_Log("Closed audio device %u.", static_cast<unsigned int>(closedAudioDeviceID));
    }

    if (globals.renderer)
    {
        SDL_DestroyRenderer(globals.renderer);
        globals.renderer = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

void Platform::run(Game& sim)
{
    const int TARGET_FPS = 60;
    const SecondsF FIXED_DELTA_TIME = SecondsF{1.0f / static_cast<float>(TARGET_FPS)};

    auto previousFrameTime = std::chrono::steady_clock::now();
    SecondsF accumulator = SecondsF{0.0f};

    m_running = true;
    while (m_running)
    {
        const auto currentTime = std::chrono::steady_clock::now();
        SecondsF deltaTime = std::chrono::duration_cast<SecondsF>(currentTime - previousFrameTime);
        previousFrameTime = currentTime;

        if (deltaTime > SecondsF{0.2f})
        {
            deltaTime = SecondsF{0.2f};
        }

        accumulator += deltaTime;

        auto& state = sim.getState();

        updateTextInputState(state); // update text input state

        // timestep update loop
        while (accumulator >= FIXED_DELTA_TIME)
        {
            GameInput input = pollInput(state);
            sim.handleInput(input, FIXED_DELTA_TIME);

            if (input.quit || state.running == false)
            {
                m_running = false;
                break;
            }

            sim.update(FIXED_DELTA_TIME);
            accumulator -= FIXED_DELTA_TIME;
        }

        RenderMain::render(state);
    }

    // ensure text input is stopped when the loop exits
    if (m_textInputActive)
    {
        SDL_StopTextInput(m_window);
        m_textInputActive = false;
        SDL_Log("Platform: Text input STOPPED on shutdown.");
    }
}

GameInput Platform::pollInput(const GameStateData& state)
{
    GameInput input{};
    SDL_Event event;
    auto toIntMouseCoord = [](float value) -> int
    {
        if (!std::isfinite(value))
        {
            return 0;
        }
        if (value > static_cast<float>(std::numeric_limits<int>::max()))
        {
            return std::numeric_limits<int>::max();
        }
        if (value < static_cast<float>(std::numeric_limits<int>::lowest()))
        {
            return std::numeric_limits<int>::lowest();
        }
        return static_cast<int>(value);
    };

    // always poll quit/escape/enter/mouse
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_WINDOW_RESIZED:
        {
            const int safeWidth = std::max(1, event.window.data1);
            const int safeHeight = std::max(1, event.window.data2);
            globals.windowWidth = safeWidth;
            globals.windowHeight = safeHeight;
            break;
        }
        case SDL_EVENT_QUIT:
            input.quit = true;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE)
                input.escape = true;
            else if (event.key.key == SDLK_RETURN)
                input.enter = true;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                input.mouseClick = true;
                input.mouseX = toIntMouseCoord(event.button.x);
                input.mouseY = toIntMouseCoord(event.button.y);
            }
            break;
        case SDL_EVENT_TEXT_INPUT:
            if (event.text.text[0] != '\0' && event.text.text[1] == '\0')
            { // ensure it's a single character
                char c = event.text.text[0];
                if (isAllowedHighScoreInputChar(c))
                {
                    input.charInputEvent = true;
                    input.inputChar = c;
                }
            }
            break;
        }
    }

    // only poll movement/shoot/boost if playing
    if (state.state == GameStateData::State::PLAYING)
    {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        input.moveLeft = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A];
        input.moveRight = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
        input.moveUp = keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W];
        input.moveDown = keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S];
        input.shoot = keys[SDL_SCANCODE_SPACE];
        input.boost =
            keys[SDL_SCANCODE_C] || keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    }
    else if (state.state == GameStateData::State::GAME_OVER && state.waitingForHighScore)
    {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        // poll for backspace/delete specifically on the high score screen
        if (keys[SDL_SCANCODE_BACKSPACE] || keys[SDL_SCANCODE_DELETE])
        {
            input.backspacePressed = true;
        }
    }

    return input;
}

void Platform::updateTextInputState(const GameStateData& state)
{
    bool shouldTextInputBeActive =
        (state.state == GameStateData::State::GAME_OVER && state.waitingForHighScore);

    if (shouldTextInputBeActive && !m_textInputActive)
    {
        // start text input
        SDL_StartTextInput(m_window);
        m_textInputActive = true;
        SDL_Log("Platform: Text input STARTED for high score entry.");
    }
    else if (!shouldTextInputBeActive && m_textInputActive)
    {
        // stop text input
        SDL_StopTextInput(m_window);
        m_textInputActive = false;
        SDL_Log("Platform: Text input STOPPED.");
    }
}
