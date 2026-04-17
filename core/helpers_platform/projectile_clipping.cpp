#include <cmath>

#include <core/helpers_platform/projectile_clipping.h>

namespace ProjectileClipping {
// for player beams
float findBeamLandscapeIntersection(float startX, float beamY, bool goingRight,
                                    const std::vector<SDL_FPoint>& landscape) {
    if (landscape.empty())
        return goingRight ? Config::Game::WORLD_WIDTH : 0.0f;
    if (!std::isfinite(startX) || !std::isfinite(beamY)) {
        return goingRight ? Config::Game::WORLD_WIDTH : 0.0f;
    }

    // clamp beamY
    if (beamY <= 0)
        return goingRight ? Config::Game::WORLD_WIDTH : 0.0f;
    const float epsilon = 1e-6f;

    if (goingRight) {
        // find first segment where x >= startX
        for (size_t i = 0; i < landscape.size() - 1; ++i) {
            float x0 = landscape[i].x;
            float x1 = landscape[i + 1].x;
            float y0 = landscape[i].y;
            float y1 = landscape[i + 1].y;
            if (!std::isfinite(x0) || !std::isfinite(x1) || !std::isfinite(y0) ||
                !std::isfinite(y1))
                continue;
            if (x1 < startX)
                continue;

            // this segment or next may contain intersection
            // if beam is above both points, beam continues
            if (beamY < y0 && beamY < y1) {
                // no intersection in this segment
                continue;
            }

            // if beam is below both, it is already on ground - shouldn't happen if projectile was
            // alive
            if (beamY >= y0 && beamY >= y1)
                return std::max(startX, x0); // beam hits ground at segment start

            // otherwise... interpolate intersection
            // find X where the horizontal beam crosses the straight line segment between (x0,y0)
            // and (x1,y1) ground line: y = y0 + t*(y1 - y0), x = x0 + t*(x1 - x0)
            const float denom = y1 - y0;
            if (!std::isfinite(denom) || std::abs(denom) < epsilon)
                continue;
            float t = (beamY - y0) / denom;
            if (!std::isfinite(t))
                continue;
            if (t >= 0.0f && t <= 1.0f) {
                float intersectX = x0 + t * (x1 - x0);
                if (!std::isfinite(intersectX))
                    continue;
                if (intersectX >= startX) {
                    return intersectX;
                }
            }
        }
        // if no intersection found, the beam goes to world edge
        return Config::Game::WORLD_WIDTH;
    } else {
        // going left: search backward
        for (size_t i = landscape.size() - 1; i > 0; --i) {
            float x0 = landscape[i - 1].x;
            float x1 = landscape[i].x;
            float y0 = landscape[i - 1].y;
            float y1 = landscape[i].y;
            if (!std::isfinite(x0) || !std::isfinite(x1) || !std::isfinite(y0) ||
                !std::isfinite(y1))
                continue;
            if (x0 > startX)
                continue;

            if (beamY < y0 && beamY < y1) {
                continue;
            }
            if (beamY >= y0 && beamY >= y1) {
                return std::min(startX, x1);
            }

            const float denom = y1 - y0;
            if (!std::isfinite(denom) || std::abs(denom) < epsilon)
                continue;
            float t = (beamY - y0) / denom;
            if (!std::isfinite(t))
                continue;
            if (t >= 0.0f && t <= 1.0f) {
                float intersectX = x0 + t * (x1 - x0);
                if (!std::isfinite(intersectX))
                    continue;
                if (intersectX <= startX) {
                    return intersectX;
                }
            }
        }
        return 0.0f;
    }
}

// for opponent projectiles
SDL_FPoint clipRayToLandscape(float startX, float startY, float endX, float endY,
                              const std::vector<SDL_FPoint>& landscape) {
    const bool startFinite = std::isfinite(startX) && std::isfinite(startY);
    const bool endFinite = std::isfinite(endX) && std::isfinite(endY);
    if (!startFinite || !endFinite) {
        const float fallbackX = endFinite ? endX : (startFinite ? startX : 0.0f);
        const float fallbackY = endFinite ? endY : (startFinite ? startY : 0.0f);
        return {fallbackX, fallbackY};
    }
    if (landscape.empty())
        return {endX, endY};

    const float epsilon = 1e-6f;

    // ray: from (startX, startY) to (endX, endY)
    float rayDx = endX - startX;
    float rayDy = endY - startY;
    float bestT = 1.0f; // full length

    // check intersection with each landscape segment
    for (size_t i = 0; i < landscape.size() - 1; ++i) {
        float x0 = landscape[i].x;
        float y0 = landscape[i].y;
        float x1 = landscape[i + 1].x;
        float y1 = landscape[i + 1].y;
        if (!std::isfinite(x0) || !std::isfinite(y0) || !std::isfinite(x1) || !std::isfinite(y1))
            continue;

        // landscape segment vector
        float segDx = x1 - x0;
        float segDy = y1 - y0;

        // solve:
        //      startX + t1*rayDx = x0 + t2*segDx
        //      startY + t1*rayDy = y0 + t2*segDy
        float denom = rayDx * segDy - rayDy * segDx;
        if (!std::isfinite(denom) || std::abs(denom) < epsilon)
            continue; // parallel

        float t2 = (rayDx * (startY - y0) - rayDy * (startX - x0)) / denom;
        if (!std::isfinite(t2))
            continue;
        if (t2 < 0.0f || t2 > 1.0f)
            continue; // intersection not on segment

        float t1 = 0.0f;
        if (std::abs(rayDx) >= epsilon) {
            t1 = (x0 + t2 * segDx - startX) / rayDx;
        } else if (std::abs(rayDy) >= epsilon) {
            t1 = (y0 + t2 * segDy - startY) / rayDy;
        } else {
            continue; // degenerate ray direction
        }
        if (!std::isfinite(t1))
            continue;

        if (t1 >= 0.0f && t1 < bestT) {
            bestT = t1;
        }
    }

    // return clipped endpoint
    return {startX + bestT * rayDx, startY + bestT * rayDy};
}
} // namespace ProjectileClipping
