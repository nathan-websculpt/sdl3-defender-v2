#include <algorithm>
#include <cassert>
#include <core/config.h>
#include <core/game_state_data.h>
#include <core/globals.h>
#include <core/helpers_platform/projectile_clipping.h>
#include <core/helpers_platform_rendering/render_helper.h>
#include <core/helpers_platform_rendering/render_hud.h>
#include <core/helpers_platform_rendering/render_main.h>
#include <core/helpers_platform_rendering/render_screens.h>
#include <core/managers/texture_manager.h>
#include <cstddef>
#include <entities/health_item.h>
void RenderMain::render(const GameStateData& state)
{
    switch (state.state)
    {
    case GameStateData::State::MENU:
        RenderScreens::renderMainMenu();
        break;
    case GameStateData::State::HOW_TO_PLAY:
        RenderScreens::renderHowToPlayScreen();
        break;
    case GameStateData::State::VIEW_HIGH_SCORES:
        RenderScreens::renderViewHighScoresScreen(state);
        break;
    case GameStateData::State::PLAYING:
        renderPlaying(state);
        break;
    case GameStateData::State::GAME_OVER:
        if (state.waitingForHighScore)
        {
            RenderScreens::renderHighScoreEntryScreen(state);
        }
        else
        {
            RenderScreens::renderGameOverScreen(state);
        }
        break;
    }
    SDL_RenderPresent(globals.renderer);
}

void RenderMain::renderPlaying(const GameStateData& state)
{
#ifndef NDEBUG
    assert(state.player && "playing render requires player state");
#endif
    RenderHelper::setRenderDrawColor(RenderColors::primary);

    SDL_RenderClear(globals.renderer);

    float cameraOffsetX = state.cameraX;
    if (state.player)
    {
        renderPlayerAndProjectiles(state, cameraOffsetX);
    }
    renderOpponentsAndProjectiles(state, cameraOffsetX);
    renderParticles(state, cameraOffsetX);
    renderLandscape(state, cameraOffsetX);
    renderHealthItems(state, cameraOffsetX);

    RenderHud::renderHudBackground();
    RenderHud::renderMinimap(state);
    RenderHud::renderHealthBars(state);
    RenderHud::renderScore(state);
}

void RenderMain::renderPlayerAndProjectiles(const GameStateData& state, float cameraOffsetX)
{
    if (!state.player)
    {
        return;
    }
    const Player& player = *state.player;

    SDL_Texture* playerTexture =
        TextureManager::getInstance().getTexture(Config::Textures::PLAYER, globals.renderer);
    if (playerTexture)
    {
        SDL_FRect renderBounds = player.getBounds();
        renderBounds.x -= cameraOffsetX;

        // apply flip based on player's facing-direction
        SDL_FRect drawRect = renderBounds;
        if (player.getFacing() == Direction::LEFT)
        {
            drawRect.x += drawRect.w;
            drawRect.w = -drawRect.w;
        }
        SDL_RenderTexture(globals.renderer, playerTexture, nullptr, &drawRect);

        // render player projectiles
        const auto& pp = player.getProjectiles();
        for (const auto& p : pp)
        {
            if (p.isExpired())
                continue;

            SDL_FRect projectileBounds = p.getBounds();
            projectileBounds.x -= cameraOffsetX;

            SDL_FRect projectileVisualBounds = projectileBounds;
            constexpr float visualProjectileSize = 3.0f;
            projectileVisualBounds.x += (projectileVisualBounds.w - visualProjectileSize) / 2.0f;
            projectileVisualBounds.y += (projectileVisualBounds.h - visualProjectileSize) / 2.0f;
            projectileVisualBounds.w = visualProjectileSize;
            projectileVisualBounds.h = visualProjectileSize;

            SDL_Color color = p.getColor();
            Uint8 renderAlpha = color.a;
            if (renderAlpha < 220)
            {
                renderAlpha = 220;
            }
            SDL_SetRenderDrawColor(globals.renderer, color.r, color.g, color.b, renderAlpha);
            SDL_RenderFillRect(globals.renderer, &projectileVisualBounds);
        }
    }
}

void RenderMain::renderOpponentsAndProjectiles(const GameStateData& state, float cameraOffsetX)
{
    for (const auto& o : state.opponents)
    {
        if (!o || !o->isAlive())
            continue;

        SDL_FRect renderBounds = o->getBounds();
        renderBounds.x -= cameraOffsetX;

        // render opponent texture
        SDL_Texture* opponentTexture =
            TextureManager::getInstance().getTexture(o->getTextureKey(), globals.renderer);
        if (opponentTexture)
        {
            SDL_RenderTexture(globals.renderer, opponentTexture, nullptr, &renderBounds);
        }
        else
        {
            // fallback rect
            RenderHelper::setRenderDrawColor(RenderColors::red);
            SDL_RenderFillRect(globals.renderer, &renderBounds);
        }

        // render opponent projectiles
        const auto& op = o->getProjectiles();
        for (const auto& p : op)
        {
            if (p.isExpired())
                continue;

            // full intended endpoint
            float dx = p.getCurrentX() - p.getSpawnX();
            float dy = p.getCurrentY() - p.getSpawnY();
            float intendedEndX = p.getSpawnX() + dx * 4.0f;
            float intendedEndY = p.getSpawnY() + dy * 4.0f;

            // clip to landscape
            SDL_FPoint clipped = ProjectileClipping::clipRayToLandscape(
                p.getSpawnX(), p.getSpawnY(), intendedEndX, intendedEndY, state.landscape);

            // camera offset
            SDL_FPoint start = {p.getSpawnX() - cameraOffsetX, p.getSpawnY()};
            SDL_FPoint end = {clipped.x - cameraOffsetX, clipped.y};

            SDL_Color color = p.getColor();
            SDL_SetRenderDrawColor(globals.renderer, color.r, color.g, color.b, color.a);
            SDL_RenderLine(globals.renderer, start.x, start.y, end.x, end.y);
        }
    }
}

void RenderMain::renderParticles(const GameStateData& state, float cameraOffsetX)
{
    for (const auto& particle : state.particles)
    {
        if (particle.isAlive())
        {
            SDL_FRect renderBounds = {particle.getX(), particle.getY(), particle.getCurrentSize(),
                                      particle.getCurrentSize()};
            renderBounds.x -= cameraOffsetX; // apply camera offset

            SDL_SetRenderDrawColor(globals.renderer, particle.getR(), particle.getG(),
                                   particle.getB(), particle.getAlpha());
            SDL_RenderFillRect(globals.renderer, &renderBounds);
        }
    }
}

void RenderMain::renderLandscape(const GameStateData& state, float cameraOffsetX)
{
    if (!state.landscape.empty())
    {
        RenderHelper::setRenderDrawColor(RenderColors::gold1);
        for (size_t i = 0; i < state.landscape.size() - 1; ++i)
        {
            SDL_FPoint p1 = {state.landscape[i].x - cameraOffsetX, state.landscape[i].y};
            SDL_FPoint p2 = {state.landscape[i + 1].x - cameraOffsetX, state.landscape[i + 1].y};
            SDL_RenderLine(globals.renderer, p1.x, p1.y, p2.x, p2.y);
        }
    }
}

void RenderMain::renderHealthItems(const GameStateData& state, float cameraOffsetX)
{
    for (const auto& item : state.healthItems)
    {
        if (!item || !item->isAlive())
            continue;

        SDL_FRect renderBounds = item->getBounds();
        renderBounds.x -= cameraOffsetX;

        SDL_Texture* itemTexture =
            TextureManager::getInstance().getTexture(item->getTextureKey(), globals.renderer);
        if (itemTexture)
        {
            // handle blinking
            Uint8 originalAlpha = 255;
            if (item->isBlinking())
            {
                originalAlpha = static_cast<Uint8>(item->getBlinkAlpha());
            }
            SDL_SetTextureAlphaMod(itemTexture, originalAlpha);
            SDL_RenderTexture(globals.renderer, itemTexture, nullptr, &renderBounds);
            SDL_SetTextureAlphaMod(itemTexture, 255); // ...resets alpha for next item
        }
        else
        {
            // fallback rectangle
            RenderHelper::setRenderDrawColor(RenderColors::green);
            if (item->getType() == HealthItemType::WORLD)
            {
                RenderHelper::setRenderDrawColor(RenderColors::textPrimary);
            }
            if (item->isBlinking())
            {
                // blinking effect
                if (static_cast<int>(SDL_GetTicks() /
                                     (static_cast<int>(HealthItem::BLINK_DURATION * 1000) / 2)) %
                        2 ==
                    0)
                {
                    SDL_RenderFillRect(globals.renderer, &renderBounds);
                }
            }
            else
            {
                SDL_RenderFillRect(globals.renderer, &renderBounds);
            }
        }
    }
}
