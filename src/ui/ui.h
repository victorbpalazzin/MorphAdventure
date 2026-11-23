#ifndef MORPH_UI_H
#define MORPH_UI_H
#include "core/module_api.h"
typedef struct UiSnapshot {
    float battery,title_steps,score_pulse,charge_pulse;
    int focus;
    unsigned collected;
} UiSnapshot;
UiApi UiModuleCreate(void);
/* Input-owner adapter: Tab/Shift-Tab or menu arrows, Enter activation.
 * Queue once per rendered frame BEFORE GameAdvance. No raylib input in UI. */
void UiQueueNavigation(int direction,bool activate);
UiSnapshot UiInspect(void);
/* HUD label of the progression battery: "LETRAS: 3/6" in the educational
 * version, original "ENERGIA  3/6  50%" in arcade and in the boss combat. */
void UiProgressLabel(const GameState *game,unsigned collected,unsigned required,int percent,char *out,size_t size);
#endif
