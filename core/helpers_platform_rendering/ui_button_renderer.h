#pragma once

#include "core/ui/ui_button.h"

namespace UIButtonPresets {

[[nodiscard]] UIButtonSpec menu(const SDL_FRect& rect, const std::string& label);
[[nodiscard]] UIButtonSpec close(int windowWidth, float margin = 10.0f, float size = 20.0f);

} // namespace UIButtonPresets

namespace UIButtonRenderer {

void render(const UIButtonSpec& button);

} // namespace UIButtonRenderer
