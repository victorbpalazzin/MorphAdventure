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
#endif
