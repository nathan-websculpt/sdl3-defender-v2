#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>
struct MIX_Audio;
struct MIX_Mixer;
struct MIX_Track;

struct MIX_Audio_Deleter {
    void operator()(MIX_Audio* audio) const;
};

struct MIX_Mixer_Deleter {
    void operator()(MIX_Mixer* mixer) const;
};

struct MIX_Track_Deleter {
    void operator()(MIX_Track* track) const;
};

class SoundManager {
  public:
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    // new: adding specifically because SoundManager has a raw SDL_AudioDeviceID
    SoundManager(SoundManager&&) = delete;
    SoundManager& operator=(SoundManager&&) = delete;

    static SoundManager& getInstance();
    // borrowed pointer is valid until this manager clears cache or shuts down
    MIX_Audio* getSound(const std::string& filepath);
    // borrowed pointer is valid until this manager clears cache or shuts down
    MIX_Track* getTrack(const std::string& filepath);
    void clearCache();
    bool initialize(SDL_AudioDeviceID deviceID, const SDL_AudioSpec& spec);
    void shutdown();

    void playSound(const std::string& filepath);

  private:
    using AudioPtr = std::unique_ptr<MIX_Audio, MIX_Audio_Deleter>;
    using MixerPtr = std::unique_ptr<MIX_Mixer, MIX_Mixer_Deleter>;
    using TrackPtr = std::unique_ptr<MIX_Track, MIX_Track_Deleter>;

    SoundManager() = default;
    ~SoundManager();
    std::unordered_map<std::string, AudioPtr> m_soundCache;
    std::unordered_map<std::string, TrackPtr> m_trackCache;
    bool m_initialized = false;
    // store the mixer instance created during initialization
    MixerPtr m_mixerInstance;
};
