## rendering pipeline overview

- render separate from simulation
- render consumes const GameStateData
- entry: RenderMain::render
- state switch on GameStateData::State
- playing → renderPlaying path
- menus/high scores via render_screens
- renderPlaying draws player, opponents, projectiles
- particles, landscape, health items drawn
- HUD layered after world draw
- HUD includes minimap, health, score
- SDL_RenderPresent at frame end
- TextureManager supplies textures
- FontManager used for text rendering
- RenderHelper caches text textures
- no gameplay-state mutation in render path
- render-side cache/SDL state can mutate (for example text cache population and texture alpha mods)
