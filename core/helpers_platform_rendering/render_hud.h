#pragma once
#include <SDL3/SDL.h>
struct GameStateData;

class RenderHud {
  public:
    static void renderHudBackground();
    static void renderHealthBars(const GameStateData& state);
    static void renderMinimap(const GameStateData& state);
    static void renderScore(const GameStateData& state);

  private:
    static void renderHealthBar(const char* label, int x, int y, int width, int height,
                                float healthRatio,
                                const SDL_Color& labelColor = {255, 255, 255, 255});
};
