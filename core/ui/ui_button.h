#pragma once

#include <SDL3/SDL.h>
#include <string>

#include "core/config.h"

struct ButtonStyle {
    SDL_Color background{0, 0, 0, 0};
    SDL_Color border{0, 0, 0, 0};
    SDL_Color text{255, 255, 255, 255};
    FontSize fontSize{FontSize::MEDIUM};
    bool drawBorder{true};
};

struct UIButtonSpec {
    SDL_FRect rect{};
    std::string label;
    ButtonStyle style{};
};

enum class MainMenuButtonId { Play, HowToPlay, HighScores, Exit };

namespace UIButtonLayout {

[[nodiscard]] SDL_FRect mainMenuButtonRect(int windowWidth, int windowHeight, MainMenuButtonId id);
[[nodiscard]] SDL_FRect closeButtonRect(int windowWidth, float margin = 10.0f, float size = 20.0f);

} // namespace UIButtonLayout

[[nodiscard]] bool uiPointInRect(int x, int y, const SDL_FRect& rect);
