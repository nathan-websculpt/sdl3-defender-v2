#include <array>
#include <core/ui/ui_button.h>
namespace
{

constexpr float MENU_BUTTON_WIDTH = 200.0f;
constexpr float MENU_BUTTON_HEIGHT = 50.0f;
constexpr float MENU_BUTTON_SPACING = 60.0f;
constexpr float MENU_START_Y_OFFSET = -60.0f;

constexpr std::array<float, 4> MENU_Y_OFFSETS{MENU_START_Y_OFFSET,
                                              MENU_START_Y_OFFSET + MENU_BUTTON_SPACING,
                                              MENU_START_Y_OFFSET + (2.0f * MENU_BUTTON_SPACING),
                                              MENU_START_Y_OFFSET + (3.0f * MENU_BUTTON_SPACING)};

[[nodiscard]] int menuButtonIndex(MainMenuButtonId id)
{
    switch (id)
    {
    case MainMenuButtonId::Play:
        return 0;
    case MainMenuButtonId::HowToPlay:
        return 1;
    case MainMenuButtonId::HighScores:
        return 2;
    case MainMenuButtonId::Exit:
        return 3;
    }

    return 0;
}

} // namespace

SDL_FRect UIButtonLayout::mainMenuButtonRect(int windowWidth, int windowHeight, MainMenuButtonId id)
{
    const float buttonX = (static_cast<float>(windowWidth) * 0.5f) - (MENU_BUTTON_WIDTH * 0.5f);
    const float buttonY = (static_cast<float>(windowHeight) * 0.5f) +
                          MENU_Y_OFFSETS[static_cast<std::size_t>(menuButtonIndex(id))];
    return SDL_FRect{buttonX, buttonY, MENU_BUTTON_WIDTH, MENU_BUTTON_HEIGHT};
}

SDL_FRect UIButtonLayout::closeButtonRect(int windowWidth, float margin, float size)
{
    const float safeMargin = margin < 0.0f ? 0.0f : margin;
    const float safeSize = size <= 0.0f ? 20.0f : size;
    const float x = static_cast<float>(windowWidth) - safeSize - safeMargin;
    return SDL_FRect{x, safeMargin, safeSize, safeSize};
}

bool uiPointInRect(int x, int y, const SDL_FRect& rect)
{
    if (rect.w <= 0.0f || rect.h <= 0.0f)
    {
        return false;
    }

    const float px = static_cast<float>(x);
    const float py = static_cast<float>(y);
    return (px >= rect.x && px < rect.x + rect.w && py >= rect.y && py < rect.y + rect.h);
}
