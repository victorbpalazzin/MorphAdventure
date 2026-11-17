#ifndef MORPH_PLAYER_INTERNAL_H
#define MORPH_PLAYER_INTERNAL_H
#include "player.h"
typedef struct PlayerState {
    PlayerView view;
    float invulnerability_steps;
    bool portal_override, debug_hitbox;
} PlayerState;
void PlayerDrawArt(const PlayerState *player, const GameState *game, const ItemsView *items);
#endif
