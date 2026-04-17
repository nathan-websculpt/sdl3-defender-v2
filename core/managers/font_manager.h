#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <string>
#include <unordered_map>
struct TTF_Font;

struct TTF_Font_Deleter {
    void operator()(TTF_Font* font) const {
        if (font) {
            TTF_CloseFont(font);
        }
    }
};

class FontManager {
  public:
    // enforce singleton
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

    // get singleton instance
    static FontManager& getInstance();

    // borrowed pointer is valid until this manager clears cache or shuts down
    TTF_Font* getFont(const std::string& filepath, int size);

    void clearCache();

  private:
    FontManager() = default;
    ~FontManager();

    std::unordered_map<std::string, std::unique_ptr<TTF_Font, TTF_Font_Deleter>> m_fontCache;
};
