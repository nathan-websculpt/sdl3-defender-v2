#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <core/managers/sound_manager.h>

namespace {

SDL_AudioDeviceID openDefaultPlaybackDevice(SDL_AudioSpec& spec) {
    SDL_zero(spec);
    spec.freq = 44100;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 2;
    return SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
}

void closeDeviceIfOwnedByCaller(SDL_AudioDeviceID& deviceID) {
    if (!deviceID) {
        return;
    }

    SDL_PauseAudioDevice(deviceID);
    SDL_CloseAudioDevice(deviceID);
    deviceID = 0;
}

} // namespace

TEST(SoundManagerLifecycle, initializeShutdownInitializeShutdownIsSafe) {
    auto& soundManager = SoundManager::getInstance();
    soundManager.shutdown();

    if (!SDL_Init(SDL_INIT_AUDIO)) {
        GTEST_SKIP() << "audio init unavailable: " << SDL_GetError();
    }

    SDL_AudioDeviceID firstDevice = 0;
    SDL_AudioDeviceID secondDevice = 0;

    SDL_AudioSpec firstSpec;
    firstDevice = openDefaultPlaybackDevice(firstSpec);
    if (!firstDevice) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        GTEST_SKIP() << "first audio device open failed: " << SDL_GetError();
    }

    const bool firstInit = soundManager.initialize(firstDevice, firstSpec);
    if (!firstInit) {
        closeDeviceIfOwnedByCaller(firstDevice);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    ASSERT_TRUE(firstInit);

    soundManager.shutdown();

    SDL_AudioSpec secondSpec;
    secondDevice = openDefaultPlaybackDevice(secondSpec);
    if (!secondDevice) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        GTEST_SKIP() << "second audio device open failed: " << SDL_GetError();
    }

    const bool secondInit = soundManager.initialize(secondDevice, secondSpec);
    if (!secondInit) {
        closeDeviceIfOwnedByCaller(secondDevice);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    ASSERT_TRUE(secondInit);

    soundManager.shutdown();
    soundManager.shutdown();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
