#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

#include <core/managers/texture_manager.h>

TextureManager& TextureManager::getInstance() {
    static TextureManager instance; // only created once (C++11)
    return instance;
}

SDL_Texture* TextureManager::getTexture(const std::string& filepath, SDL_Renderer* renderer) {
    auto it = m_textureCache.find(filepath);
    if (it != m_textureCache.end()) {
        // return cached borrowed pointer
        return it->second.get();
    }

    SDL_Texture* texture = IMG_LoadTexture(renderer, filepath.c_str());
    if (!texture) {
        SDL_Log("Failed to load texture '%s': %s", filepath.c_str(), SDL_GetError());
        return nullptr;
    }

    // store owned texture in cache and return a borrowed pointer
    auto ownedTexture = std::unique_ptr<SDL_Texture, SDL_Texture_Deleter>(texture);
    auto [insertedIt, inserted] = m_textureCache.emplace(filepath, std::move(ownedTexture));
    (void)inserted;

    return insertedIt->second.get();
}

void TextureManager::clearCache() {
    SDL_Log("TextureManager: Clearing cache and destroying %zu textures.", m_textureCache.size());
    m_textureCache.clear(); // will call the deleter for each texture
}

TextureManager::~TextureManager() {
    clearCache(); // clean up all cached textures when manager is destroyed
}
