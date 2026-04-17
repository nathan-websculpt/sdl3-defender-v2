#include <cstring>
#include <deque>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <core/globals.h>
#include <core/helpers_platform_rendering/render_helper.h>
#include <core/helpers_platform_rendering/ui_button_renderer.h>
#include <core/managers/font_manager.h>

namespace {

struct SDL_Texture_Deleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};

struct TextCacheKey {
    std::string text;
    int fontSize = 0;
    Uint32 colorKey = 0;

    bool operator==(const TextCacheKey& other) const = default;
};

struct TextCacheKeyHash {
    [[nodiscard]] std::size_t operator()(const TextCacheKey& key) const noexcept {
        std::size_t seed = std::hash<std::string>{}(key.text);
        seed ^= std::hash<int>{}(key.fontSize) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        seed ^= std::hash<Uint32>{}(key.colorKey) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct TextCacheEntry {
    std::unique_ptr<SDL_Texture, SDL_Texture_Deleter> texture;
    int width = 0;
    int height = 0;
};

std::unordered_map<TextCacheKey, TextCacheEntry, TextCacheKeyHash> g_textTextureCache;
std::deque<TextCacheKey> g_textCacheInsertionOrder;
std::unordered_set<std::string> g_loggedTextFailures;
constexpr std::size_t TEXT_CACHE_MAX_ENTRIES = 256;

void evictTextCacheEntriesToFitBound() {
    while (g_textTextureCache.size() >= TEXT_CACHE_MAX_ENTRIES &&
           !g_textCacheInsertionOrder.empty()) {
        g_textTextureCache.erase(g_textCacheInsertionOrder.front());
        g_textCacheInsertionOrder.pop_front();
    }

    if (g_textTextureCache.size() >= TEXT_CACHE_MAX_ENTRIES) {
        g_textTextureCache.clear();
        g_textCacheInsertionOrder.clear();
    }
}

[[nodiscard]] bool shouldLogTextFailureOnce(std::string_view failureKey) {
    return g_loggedTextFailures.emplace(failureKey).second;
}

[[nodiscard]] int getFontSize(FontSize sizeEnum) {
    switch (sizeEnum) {
    case FontSize::SMALL:
        return 16;
    case FontSize::MEDIUM:
        return 24;
    case FontSize::LARGE:
        return 36;
    case FontSize::GRANDELOCO:
        return 52;
    }

    return 16;
}

[[nodiscard]] Uint32 packColorKey(const SDL_Color& color) {
    return (static_cast<Uint32>(color.r) << 24) | (static_cast<Uint32>(color.g) << 16) |
           (static_cast<Uint32>(color.b) << 8) | static_cast<Uint32>(color.a);
}

} // namespace

// wraps SDL_SetRenderDrawColor - easier to keep SDL_Colors in one place
// pass RenderColors::colorxyz into this to set SDL Draw Color
void RenderHelper::setRenderDrawColor(const SDL_Color& c) {
    SDL_SetRenderDrawColor(globals.renderer, c.r, c.g, c.b, c.a);
}

void RenderHelper::renderCloseButton() {
    UIButtonRenderer::render(UIButtonPresets::close(globals.windowWidth));
}

void RenderHelper::renderMenuButton(int x, int y, int width, int height, const SDL_Color& textColor,
                                    const std::string& text) {
    UIButtonSpec button =
        UIButtonPresets::menu(SDL_FRect{static_cast<float>(x), static_cast<float>(y),
                                        static_cast<float>(width), static_cast<float>(height)},
                              text);
    button.style.text = textColor;
    UIButtonRenderer::render(button);
}

void RenderHelper::renderText(const char* text, int x, int y, const SDL_Color& color,
                              FontSize sizeEnum) {
    if (!text) {
        if (shouldLogTextFailureOnce("render_text_null")) {
            SDL_Log("RenderHelper::renderText called with null text");
        }
        return;
    }

    const int fontSize = getFontSize(sizeEnum);
    TextCacheKey cacheKey{std::string(text), fontSize, packColorKey(color)};
    auto cacheIt = g_textTextureCache.find(cacheKey);

    if (cacheIt == g_textTextureCache.end()) {
        TTF_Font* font =
            FontManager::getInstance().getFont(Config::Fonts::DEFAULT_FONT_FILE, fontSize);
        if (!font) {
            if (shouldLogTextFailureOnce("render_text_font")) {
                SDL_Log("Failed to get font from manager");
            }
            return;
        }

        SDL_Surface* fontSurface = TTF_RenderText_Solid(font, text, std::strlen(text), color);
        if (!fontSurface) {
            if (shouldLogTextFailureOnce("render_text_surface")) {
                SDL_Log("Text Render failed: %s", SDL_GetError());
            }
            return;
        }

        SDL_Texture* rawTexture = SDL_CreateTextureFromSurface(globals.renderer, fontSurface);
        if (!rawTexture) {
            SDL_DestroySurface(fontSurface);
            if (shouldLogTextFailureOnce("render_text_texture")) {
                SDL_Log("Failed to create texture from font surface: %s", SDL_GetError());
            }
            return;
        }

        TextCacheEntry cacheEntry;
        cacheEntry.texture = std::unique_ptr<SDL_Texture, SDL_Texture_Deleter>(rawTexture);
        cacheEntry.width = fontSurface->w;
        cacheEntry.height = fontSurface->h;
        SDL_DestroySurface(fontSurface);

        evictTextCacheEntriesToFitBound();
        auto [insertedIt, inserted] =
            g_textTextureCache.emplace(std::move(cacheKey), std::move(cacheEntry));
        if (inserted) {
            g_textCacheInsertionOrder.push_back(insertedIt->first);
        }
        cacheIt = insertedIt;
    }

    SDL_FRect dst = {static_cast<float>(x), static_cast<float>(y),
                     static_cast<float>(cacheIt->second.width),
                     static_cast<float>(cacheIt->second.height)};
    SDL_RenderTexture(globals.renderer, cacheIt->second.texture.get(), nullptr, &dst);
}

void RenderHelper::clearTextCache() {
    g_textTextureCache.clear();
    g_textCacheInsertionOrder.clear();
    g_loggedTextFailures.clear();
}
