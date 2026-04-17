#include <core/game_state_data.h>
#include <core/helpers_game/collision_handler.h>
#include <limits>
#include <memory>
using namespace CollisionHandler;

namespace
{
// UB fix: avoids signed-int overflow
void addScoreSaturating(GameStateData& state, int delta)
{
    if (delta <= 0)
    {
        return;
    }

    const int maxScore = std::numeric_limits<int>::max();
    if (state.playerScore > maxScore - delta)
    {
        state.playerScore = maxScore;
        return;
    }

    state.playerScore += delta;
}

bool isBeyondBeamVisualRange(const SDL_FRect& opponentBounds, float visualEndX, bool goingRight)
{
    const float oppMinX = opponentBounds.x;
    const float oppMaxX = opponentBounds.x + opponentBounds.w;

    if (goingRight)
    {
        return oppMinX > visualEndX;
    }

    return oppMaxX < visualEndX;
}

// player projectiles hitting opponents
void handlePlayerProjectiles(GameStateData& state, const GameHelper& helpers,
                             Random::RngEngine& fxRng)
{
    if (!state.player)
        return;
    auto& pp = state.player->getProjectiles();
    for (auto p_it = pp.begin(); p_it != pp.end();)
    {
        SDL_FRect pb = p_it->getBounds();
        bool projectileHit = false;

        // for horizontal beams, find visual end X
        float beamY = p_it->getSpawnY();
        float startX = p_it->getSpawnX();
        bool goingRight = (p_it->getVelocity().x > 0);
        // landscape stops beam
        float visualEndX = helpers.getBeamVisualEndX(startX, beamY, goingRight);

        for (auto& o : state.opponents)
        { // o is std::unique_ptr<BaseOpponent>&
            if (!o || !o->isAlive())
                continue;
            SDL_FRect ob = o->getBounds();
            // skip if opponent is beyond the beam's visual range (landscape stopped it)
            if (isBeyondBeamVisualRange(ob, visualEndX, goingRight))
                continue;

            if (helpers.rectsIntersect(ob, pb))
            {
                o->takeDamage(1);
                if (!o->isAlive())
                {
                    addScoreSaturating(state, o->getScoreVal());
                    o->explode(state.particles, fxRng);
                }
                projectileHit = true;
                break; // break inner loop
            }
        }
        if (projectileHit)
        {
            p_it = pp.erase(p_it); // erase using projectile iterator, assign returned iterator
        }
        else
        {
            ++p_it;
        }
    }
}

// player collisions with opponents and opponents projectiles collision with player
// returns true if processing completed normally, false if it resulted in game-over
bool handleOpponentsAndPlayer(GameStateData& state, const GameHelper& helpers,
                              const SDL_FRect& playerBounds, Random::RngEngine& fxRng)
{
    if (!state.player || !state.player->isAlive())
        return true;
    for (auto o_it = state.opponents.begin(); o_it != state.opponents.end();)
    {
        auto& o = *o_it;
        if (!o || !o->isAlive())
        {
            ++o_it; // skip dead opponents
            continue;
        }

        SDL_FRect oppBounds = o->getBounds();
        // check player/opponent collision
        if (helpers.rectsIntersect(playerBounds, oppBounds))
        {
            state.player->takeDamage(1);
            o->explode(state.particles, fxRng);
            addScoreSaturating(state, o->getScoreVal());
            o_it = state.opponents.erase(o_it);
            if (!state.player->isAlive())
            {
                return false; // exit early if player dies
            }
            continue; // skip projectile check if opponent was destroyed by collision
        }

        // check if opponent's projectiles hit player
        auto& op = o->getProjectiles();
        for (auto op_it = op.begin(); op_it != op.end();)
        {
            SDL_FRect projBounds = op_it->getBounds();

            // collision check ... projectile and player
            if (helpers.rectsIntersect(projBounds, playerBounds))
            {
                state.player->takeDamage(1);
                // erase the projectile that hit the player using the iterator
                op_it = op.erase(op_it);
                if (!state.player->isAlive())
                {
                    return false; // exit early if player dies
                }
            }
            else
            {
                ++op_it;
            }
        }

        ++o_it;
    }
    return true;
}

// player collisions with health items
void handleHealthItems(GameStateData& state, const GameHelper& helpers,
                       const SDL_FRect& playerBounds)
{
    if (!state.player)
        return;
    for (auto it = state.healthItems.begin(); it != state.healthItems.end();)
    {
        auto& item = *it;
        if (!item || !item->isAlive() || item->isBlinking())
        { // don't collide if blinking or dead
            ++it;
            continue;
        }
        if (helpers.rectsIntersect(playerBounds, item->getBounds()))
        {
            if (item->getType() == HealthItemType::PLAYER)
            {
                state.player->restoreHealth();
            }
            else if (item->getType() == HealthItemType::WORLD)
            {
                state.worldHealth = state.maxWorldHealth;
            }
            it = state.healthItems.erase(it);
            continue;
        }
        ++it;
    }
}

} // namespace

bool CollisionHandler::processAllCollisions(GameStateData& state, const GameHelper& helpers,
                                            const HighScores& highScores,
                                            const SDL_FRect& playerBounds, Random::RngEngine& fxRng)
{
    (void)highScores;
    if (!state.player)
        return true;

    handlePlayerProjectiles(state, helpers, fxRng);

    if (!handleOpponentsAndPlayer(state, helpers, playerBounds, fxRng))
        return false; // player died inside the handler

    handleHealthItems(state, helpers, playerBounds);
    return true;
}
