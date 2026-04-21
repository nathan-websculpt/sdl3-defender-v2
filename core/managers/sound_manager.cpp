#include "core/managers/sound_manager.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <algorithm>
#include <iostream>

void MIX_Audio_Deleter::operator()(MIX_Audio* audio) const {
    if (audio) {
        MIX_DestroyAudio(audio);
    }
}

void MIX_Mixer_Deleter::operator()(MIX_Mixer* mixer) const {
    if (mixer) {
        MIX_DestroyMixer(mixer);
    }
}

void MIX_Track_Deleter::operator()(MIX_Track* track) const {
    if (track) {
        MIX_DestroyTrack(track);
    }
}

SoundManager::~SoundManager() {
    shutdown();
}

void SoundManager::shutdown() {
    if (!m_initialized && m_soundCache.empty() && m_trackCache.empty() && !m_mixerInstance) {
        return;
    }

    clearCache();
    m_mixerInstance.reset();

    const bool wasInitialized = m_initialized;
    m_initialized = false;
    if (wasInitialized) {
        SDL_Log("SoundManager: SDL_mixer shut down.");
        MIX_Quit();
    }
}

void SoundManager::clearCache() {
    SDL_Log("SoundManager: Clearing sound cache and destroying %zu audio objects AND %zu tracks.",
            m_soundCache.size(), m_trackCache.size());
    m_soundCache.clear();
    m_trackCache.clear();
}

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

bool SoundManager::initialize(SDL_AudioDeviceID deviceID, const SDL_AudioSpec& spec) {
    if (m_initialized) {
        SDL_Log("SoundManager: Already initialized.");
        return true;
    }

    bool init_success = MIX_Init();
    if (!init_success) {
        SDL_Log("SDL_mixer could not initialize! MIX_Init Error: %s", SDL_GetError());
        return false;
    }

    MIX_Mixer* mixer = MIX_CreateMixerDevice(deviceID, &spec);
    if (!mixer) {
        SDL_Log("SDL_mixer could not create mixer for device %u! SDL_GetError: %s",
                static_cast<unsigned int>(deviceID), SDL_GetError());
        MIX_Quit();
        return false;
    }

    m_mixerInstance = MixerPtr(mixer);

    m_initialized = true;
    SDL_Log("SoundManager: SDL_mixer initialized successfully with device %u.",
            static_cast<unsigned int>(deviceID));
    return true;
}

MIX_Audio* SoundManager::getSound(const std::string& filepath) {
    if (!m_initialized || !m_mixerInstance) {
        SDL_Log("SoundManager: Cannot get sound, not initialized or mixer is null.");
        return nullptr;
    }

    auto it = m_soundCache.find(filepath);
    if (it != m_soundCache.end()) {
        SDL_Log("SoundManager: Cache HIT for sound '%s'.", filepath.c_str());
        return it->second.get();
    }

    MIX_Audio* audio = MIX_LoadAudio(m_mixerInstance.get(), filepath.c_str(), true);
    if (!audio) {
        SDL_Log("Failed to load sound '%s': %s", filepath.c_str(), SDL_GetError());
        return nullptr;
    }

    AudioPtr ownedAudio(audio);
    auto [insertedIt, inserted] = m_soundCache.emplace(filepath, std::move(ownedAudio));
    (void)inserted;

    SDL_Log("SoundManager: Cache MISS, LOADED sound '%s'.", filepath.c_str());
    return insertedIt->second.get();
}

MIX_Track* SoundManager::getTrack(const std::string& filepath) {
    if (!m_initialized || !m_mixerInstance) {
        SDL_Log("SoundManager: Cannot get track, not initialized or mixer is null.");
        return nullptr;
    }

    auto it = m_trackCache.find(filepath);
    if (it != m_trackCache.end()) {
        SDL_Log("SoundManager: Cache HIT for track '%s'.", filepath.c_str());
        return it->second.get();
    }

    MIX_Track* track = MIX_CreateTrack(m_mixerInstance.get());
    if (!track) {
        SDL_Log("Failed to load track '%s': %s", filepath.c_str(), SDL_GetError());
        return nullptr;
    }

    TrackPtr ownedTrack(track);
    auto [insertedIt, inserted] = m_trackCache.emplace(filepath, std::move(ownedTrack));
    (void)inserted;

    SDL_Log("SoundManager: Cache MISS, LOADED track '%s'.", filepath.c_str());
    return insertedIt->second.get();
}

void SoundManager::playSound(const std::string& filepath) {
    if (!m_initialized || !m_mixerInstance) {
        SDL_Log("SoundManager: Cannot play sound, not initialized or mixer is null.");
        return;
    }

    MIX_Audio* audio = getSound(filepath);
    if (!audio) {
        SDL_Log("SoundManager: Cannot play sound '%s', failed to load or retrieve.",
                filepath.c_str());
        return;
    }

    MIX_Track* track = getTrack(filepath);
    if (!track) {
        SDL_Log("SoundManager: Failed to create track for sound '%s': %s", filepath.c_str(),
                SDL_GetError());
        return;
    }

    if (!MIX_SetTrackAudio(track, audio)) {
        SDL_Log("SoundManager: Failed to assign audio to track for sound '%s': %s",
                filepath.c_str(), SDL_GetError());
        return;
    }

    bool playSuccess = MIX_PlayTrack(track, 0);

    if (!playSuccess) {
        SDL_Log("SoundManager: Failed to play track for sound '%s': %s", filepath.c_str(),
                SDL_GetError());
        return;
    }

    SDL_Log("SoundManager: Played sound '%s'.", filepath.c_str());
    return;
}
