#ifndef MORPH_EFFECTS_INTERNAL_H
#define MORPH_EFFECTS_INTERNAL_H
#include "effects.h"
typedef struct EffectParticle { GameVec2 position,velocity; float life,radius; uint32_t color; } EffectParticle;
typedef enum EffectPortalPhase { FX_IDLE,FX_GROW,FX_ENTER,FX_BLACK,FX_REVEAL,FX_DONE } EffectPortalPhase;
typedef struct EffectsState {
    int world;
    EffectPortalPhase phase;
    unsigned count;
    uint32_t seed;
    float age,entry,transition,damage_flash,charge_flash;
    double clock;
    bool blackout_sent,done_sent;
    GameVec2 start;
    PortalPose pose;
    EffectParticle particles[EFFECTS_PARTICLE_CAPACITY];
} EffectsState;
void EffectsDrawPortal(const EffectsState *s);
void EffectsDrawParticles(const EffectsState *s);
void EffectsDrawTransition(const EffectsState *s,const GameState *g);
#endif
