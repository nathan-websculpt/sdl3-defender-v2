#include <algorithm>
#include <cmath>
#include <core/helpers_platform_rendering/render_helper.h>
#include <core/helpers_platform_rendering/ui_button_renderer.h>
#include <limits>
namespace
{

[[nodiscard]] int toIntClamped(float value)
{
    if (!std::isfinite(value))
    {
        return 0;
    }

    if (value > static_cast<float>(std::numeric_limits<int>::max()))
    {
        return std::numeric_limits<int>::max();
    }

    if (value < static_cast<float>(std::numeric_limits<int>::lowest()))
    {
        return std::numeric_limits<int>::lowest();
    }

    return static_cast<int>(value);
}

[[nodiscard]] int estimateGlyphWidth(FontSize size)
{
    switch (size)
    {
    case FontSize::SMALL:
        return 12;
    case FontSize::MEDIUM:
        return 14;
    case FontSize::LARGE:
        return 20;
    case FontSize::GRANDELOCO:
        return 28;
    }

    return 14;
}

[[nodiscard]] int estimateTextHeight(FontSize size)
{
    switch (size)
    {
    case FontSize::SMALL:
        return 20;
    case FontSize::MEDIUM:
        return 24;
    case FontSize::LARGE:
        return 36;
    case FontSize::GRANDELOCO:
        return 52;
    }

    return 24;
}

[[nodiscard]] int boundedTextWidth(const std::string& text, FontSize size)
{
    const int glyphWidth = estimateGlyphWidth(size);
    if (glyphWidth <= 0)
    {
        return 0;
    }

    const std::size_t maxSafeChars =
        static_cast<std::size_t>(std::numeric_limits<int>::max() / glyphWidth);
    const std::size_t safeChars = std::min(text.size(), maxSafeChars);
    return static_cast<int>(safeChars) * glyphWidth;
}

} // namespace

UIButtonSpec UIButtonPresets::menu(const SDL_FRect& rect, const std::string& label)
{
    UIButtonSpec button{};
    button.rect = rect;
    button.label = label;
    button.style.background = RenderColors::blueLight;
    button.style.border = RenderColors::white;
    button.style.text = RenderColors::white;
    button.style.fontSize = FontSize::MEDIUM;
    button.style.drawBorder = true;
    return button;
}

UIButtonSpec UIButtonPresets::close(int windowWidth, float margin, float size)
{
    UIButtonSpec button{};
    button.rect = UIButtonLayout::closeButtonRect(windowWidth, margin, size);
    button.label = "X";
    button.style.background = RenderColors::greySlate;
    button.style.border = RenderColors::white;
    button.style.text = RenderColors::white;
    button.style.fontSize = FontSize::SMALL;
    button.style.drawBorder = true;
    return button;
}

void UIButtonRenderer::render(const UIButtonSpec& button)
{
    RenderHelper::setRenderDrawColor(button.style.background);
    SDL_RenderFillRect(globals.renderer, &button.rect);

    if (button.style.drawBorder)
    {
        RenderHelper::setRenderDrawColor(button.style.border);
        SDL_RenderRect(globals.renderer, &button.rect);
    }

    if (button.label.empty())
    {
        return;
    }

    const int textWidth = boundedTextWidth(button.label, button.style.fontSize);
    const int textHeight = estimateTextHeight(button.style.fontSize);

    const float centeredTextX =
        button.rect.x + ((button.rect.w - static_cast<float>(textWidth)) * 0.5f);
    const float centeredTextY =
        button.rect.y + ((button.rect.h - static_cast<float>(textHeight)) * 0.5f);

    const int textX = toIntClamped(centeredTextX);
    const int textY = toIntClamped(centeredTextY);

    RenderHelper::renderText(button.label.c_str(), textX, textY, button.style.text,
                             button.style.fontSize);
}
