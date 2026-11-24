#ifndef MORPH_UI_H
#define MORPH_UI_H
#include "core/module_api.h"
typedef struct UiSnapshot {
    float battery,title_steps,score_pulse,charge_pulse;
    int focus;
    unsigned collected;
    int quiz_focus;        /* alternativa em foco no desafio (-1: nenhuma) */
} UiSnapshot;
UiApi UiModuleCreate(void);
/* Input-owner adapter: Tab/Shift-Tab or menu arrows, Enter activation.
 * Queue once per rendered frame BEFORE GameAdvance. No raylib input in UI. */
void UiQueueNavigation(int direction,bool activate);
UiSnapshot UiInspect(void);
/* Textos de ajuda: com controle conectado mostram X/Quadrado/OPTIONS/Círculo;
 * sem controle, os textos originais de teclado/mouse. Só afeta o desenho. */
void UiSetGamepadHints(bool gamepad);
bool UiGamepadHints(void);
/* HUD label of the progression battery: "LETRAS: 3/6" in the educational
 * version, original "ENERGIA  3/6  50%" in arcade and in the boss combat. */
void UiProgressLabel(const GameState *game,unsigned collected,unsigned required,int percent,char *out,size_t size);

/* Painel de som na pausa. A UI não conhece o módulo de áudio: main.c informa a
 * vista atual e consome os pedidos feitos por clique (teclado é tratado em main.c). */
typedef struct UiAudioView {
    bool available;        /* false: painel mostra que o som está indisponível */
    float volume[3];       /* geral, música, efeitos (0..1) */
    bool muted;
    int selected;          /* linha escolhida para o teclado (0..2) */
} UiAudioView;
typedef struct UiAudioRequest {
    int channel;           /* -1 nenhum; 0 geral, 1 música, 2 efeitos */
    int delta;             /* -1 ou +1 */
    bool toggle_mute;
} UiAudioRequest;
void UiSetAudioView(UiAudioView view);
UiAudioView UiGetAudioView(void);
UiAudioRequest UiTakeAudioRequest(void);
#endif
