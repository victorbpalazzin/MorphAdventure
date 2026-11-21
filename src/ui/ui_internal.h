#ifndef MORPH_UI_INTERNAL_H
#define MORPH_UI_INTERNAL_H
#include "ui.h"
typedef struct UiState {
    float narrative_seconds;
    GameMode narrative_mode;
    float battery,title_steps,score_pulse,charge_pulse;
    uint64_t score;
    unsigned collected;
    int focus,navigation;
    bool activate;
    GameMode input_mode;
} UiState;
typedef struct UiButton { GameRect rect; GameCommand command; const char *label; } UiButton;
unsigned UiButtons(GameMode mode,UiButton out[2]);
void UiNarrativeDraw(const UiState *state,const GameState *game);
void UiDraw(const UiState *state,const GameState *game,const ItemsView *items);
#endif
