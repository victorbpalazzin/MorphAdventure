#ifndef MORPH_GAME_H
#define MORPH_GAME_H
#include "module_api.h"

/* Build educativo: definido pelo CMake (MORPH_EDUCATIONAL=ON/OFF) em morph_core,
 * portanto igual para o jogo e para os testes. Padrão: versão arcade. */
#ifndef MORPH_EDUCATIONAL
#define MORPH_EDUCATIONAL 0
#endif

typedef struct Game {
    GameState state;
    GameModules modules;
    const WorldConfig *config;
    double accumulator;
    bool life_awarded, pending_attack, pending_impulse, ready, transition_switched;
    unsigned missing_modules, initialized_modules;
    char error[192];
} Game;

/* False means an actual contract/init failure. Missing modules leave a valid,
 * non-playable diagnostic shell: ready=false and missing_modules set. */
bool GameInit(Game *game, const GameModules *modules); /* mode from MORPH_EDUCATIONAL */
/* Same as GameInit with an explicit mode; tests use it to cover both versions. */
bool GameInitMode(Game *game, const GameModules *modules, bool educational);
bool GameEducationalBuild(void);
void GameSetQuizSeed(uint32_t seed); /* session seed, mixed with run_id per adventure */
void GameShutdown(Game *game);
void GameAdvance(Game *game, double seconds, GameInput input);
void GameDraw(Game *game);
unsigned GameEnergyRequired(int world);
float GameEnergyFraction(unsigned collected, int world);
bool GameEnqueueEvent(GameEventBuffer *buffer, GameEvent event);
const char *GameModeName(GameMode mode);

#endif
