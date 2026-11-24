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
/* Painel de som (pausa): linha 0..2 -> botões "-" e "+"; mute em row=-1. */
typedef struct UiAudioRects { GameRect panel,minus[3],plus[3],bar[3],mute; } UiAudioRects;
UiAudioRects UiAudioLayout(void);
void UiNarrativeDraw(const UiState *state,const GameState *game);
void UiDraw(const UiState *state,const GameState *game,const ItemsView *items);
#endif
