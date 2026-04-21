#include "core/managers/font_manager.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <sstream>

FontManager& FontManager::getInstance() {
    static FontManager instance;
    return instance;
}

TTF_Font* FontManager::getFont(const std::string& filepath, int size) {

    // a unique key for filepath and size combination
    std::ostringstream keyStream;
    keyStream << filepath << "_" << size;
    std::string key = keyStream.str();

    auto it = m_fontCache.find(key);
    if (it != m_fontCache.end()) {
        return it->second.get();
    }

    TTF_Font* font = TTF_OpenFont(filepath.c_str(), static_cast<float>(size));
    if (!font) {
        SDL_Log("FontManager: Failed to load font '%s' with size %d: %s", filepath.c_str(), size,
                SDL_GetError());
        return nullptr;
    }

    // store owned font in the cache using the combined key and return borrowed pointer
    auto ownedFont = std::unique_ptr<TTF_Font, TTF_Font_Deleter>(font);
    auto [insertedIt, inserted] = m_fontCache.emplace(key, std::move(ownedFont));
    (void)inserted;

    return insertedIt->second.get();
}

void FontManager::clearCache() {
    SDL_Log("FontManager: Clearing cache and closing %zu fonts.", m_fontCache.size());
    m_fontCache.clear(); // will automatically call the deleter for each font
}

FontManager::~FontManager() {
    clearCache();
}
