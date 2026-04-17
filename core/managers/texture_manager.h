#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <unordered_map>
struct SDL_Texture;

struct SDL_Texture_Deleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};

class TextureManager {
  public:
    // enforce singleton
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    // get singleton instance
    static TextureManager& getInstance();

    // borrowed pointer is valid until this manager clears cache or shuts down
    SDL_Texture* getTexture(const std::string& filepath, SDL_Renderer* renderer);

    void clearCache();

  private:
    TextureManager() = default;
    ~TextureManager(); // handles SDL_DestroyTexture

    std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDL_Texture_Deleter>>
        m_textureCache;
};
