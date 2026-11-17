#ifndef MORPH_EFFECTS_H
#define MORPH_EFFECTS_H
#include "core/module_api.h"
#define EFFECTS_PARTICLE_CAPACITY 180
typedef struct EffectsSnapshot {
    int world,phase;
    unsigned particles;
    float age,entry,transition,damage_flash,charge_flash;
    double clock;
    PortalPose pose;
} EffectsSnapshot;
EffectsApi EffectsModuleCreate(void);
EffectsSnapshot EffectsInspect(void);
#endif
