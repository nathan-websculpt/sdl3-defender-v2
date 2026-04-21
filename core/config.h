#pragma once
#include <string>
enum class FontSize { SMALL, MEDIUM, LARGE, GRANDELOCO };

// note
// inline:      single definition across all translation units
// constexpr:   must be evaluated at compile time

namespace Config {
namespace Textures {
const std::string PLAYER = "assets/defender.png";
const std::string BASIC_OPPONENT = "assets/bang.png";
const std::string AGGRESSIVE_OPPONENT = "assets/aggressive_opponent.png";
const std::string SNIPER_OPPONENT = "assets/sniper_opponent.png";
const std::string PLAYER_HEALTH_ITEM = "assets/health_player.png";
const std::string WORLD_HEALTH_ITEM = "assets/health_world.png";
} // namespace Textures

namespace Fonts {
const std::string DEFAULT_FONT_FILE = "assets/Audiowide-Regular.ttf";
}

namespace Game {
inline constexpr int WORLD_WIDTH = 6400;
inline constexpr int HUD_HEIGHT = 80;
const std::string HIGH_SCORES_RESOURCE_PATH = "resources/highscores.txt";
const std::string HIGH_SCORES_FILENAME = "highscores.txt";
} // namespace Game

namespace Sounds {
const std::string GAME_START = "assets/audio/431243__audiopapkin__futuristic-organic-effect-39.wav";
const std::string GAME_OVER = "assets/audio/159408__noirenex__life-lost-game-over.wav";
const std::string PLAYER_SHOOT = "assets/audio/580308__colorscrimsontears__lazer-shot-2.wav";
const std::string OPPONENT_EXPLODE = "assets/audio/372182__supersound23__pop.wav";
} // namespace Sounds
} // namespace Config
