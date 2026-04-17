#include <core/helpers_game/game_helper.h>
#include <core/helpers_platform/projectile_clipping.h>

GameHelper::GameHelper(const std::vector<SDL_FPoint>& landscape) : m_landscape(landscape) {}

bool GameHelper::isOutOfWorld(const SDL_FRect& r, float mx, float my) const {
    return (r.x + r.w < -mx || r.x > Config::Game::WORLD_WIDTH + mx || r.y + r.h < -my ||
            r.y > globals.windowHeight + my);
}

float GameHelper::getGroundYAt(float x) const {
    const auto& land = m_landscape;
    if (land.empty())
        return static_cast<float>(globals.windowHeight);

    // clamp x to landscape bounds
    if (x <= land.front().x)
        return land.front().y;
    if (x >= land.back().x)
        return land.back().y;

    for (size_t i = 0; i < land.size() - 1; ++i) {
        if (x >= land[i].x && x <= land[i + 1].x) {
            // linear interpolation between land[i] and land[i+1]
            float t = (x - land[i].x) / (land[i + 1].x - land[i].x);
            return land[i].y + t * (land[i + 1].y - land[i].y);
        }
    }
    return land.back().y; // fallback
}

bool GameHelper::rectsIntersect(const SDL_FRect& a, const SDL_FRect& b) const {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

float GameHelper::getBeamVisualEndX(float startX, float beamY, bool goingRight) const {
    return ProjectileClipping::findBeamLandscapeIntersection(startX, beamY, goingRight,
                                                             m_landscape);
}

void GameHelper::keepPlayerInBounds(Player& player, SDL_FRect& pb) {
    // keeps player beneath HUD, above landscape, and in-world

    float desiredX = pb.x;
    float desiredY = pb.y;

    // left and right (world) boundaries
    if (desiredX < 0)
        desiredX = 0;
    if (desiredX + pb.w > Config::Game::WORLD_WIDTH)
        desiredX = Config::Game::WORLD_WIDTH - pb.w;

    // top (HUD) boundary
    desiredY = std::max(desiredY, static_cast<float>(Config::Game::HUD_HEIGHT));

    // landscape constraint (bottom)
    float playerBottomX = desiredX + pb.w / 2.0f;
    float groundYAtPlayerX = getGroundYAt(playerBottomX);
    float absoluteWorldBottom = globals.windowHeight - pb.h; // absolute bottom of the world

    // player's bottom Y should not exceed the landscape height at their X position
    float effectiveGroundY = std::min(groundYAtPlayerX, absoluteWorldBottom);
    float maxAllowedY = effectiveGroundY - pb.h;

    // ensure desiredY is not below the calculated maximum
    desiredY = std::min(desiredY, maxAllowedY);

    if (pb.x != desiredX || pb.y != desiredY) {
        player.setPosition(desiredX, desiredY); // apply the final calculated position
    }
}
