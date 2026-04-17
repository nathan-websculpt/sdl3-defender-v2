#pragma once
#include <SDL3/SDL.h>
#include <core/config.h>
#include <core/globals.h>
#include <entities/player.h>
#include <vector>
class GameHelper
{
  public:
    explicit GameHelper(const std::vector<SDL_FPoint>& landscape);
    ~GameHelper() = default;

    // new: adding because of the const std::vector<SDL_FPoint>& member causing default copy
    // constructor/assignment to be generated
    //      copying this obj could lead to a dangling reference if landscape where to go out of
    //      scope
    GameHelper(const GameHelper&) = delete;
    GameHelper& operator=(const GameHelper&) = delete;
    GameHelper(GameHelper&&) = delete;
    GameHelper& operator=(GameHelper&&) = delete;

    float getGroundYAt(float x) const; // for landscape
    bool isOutOfWorld(const SDL_FRect& r, float mx = 100.0f, float my = 100.0f) const;
    bool rectsIntersect(const SDL_FRect& a, const SDL_FRect& b) const;
    float getBeamVisualEndX(float startX, float beamY,
                            bool goingRight) const; // landscape stops player's beam
    void keepPlayerInBounds(Player& player, SDL_FRect& pb);

  private:
    const std::vector<SDL_FPoint>& m_landscape;
};
