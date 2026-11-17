#ifndef MORPH_WORLDS_H
#define MORPH_WORLDS_H
#include "core/module_api.h"

/* Colors are 0xRRGGBBAA, compatible with raylib GetColor(). No raylib types
 * leak into consumers. Returned palettes have static lifetime. */
typedef struct WorldPalette {
    const char *title;
    uint32_t sky[3], layers[3], ink, accent;
} WorldPalette;
const WorldPalette *WorldsPalette(int world); /* NULL for invalid world */
WorldsApi WorldsModuleCreate(void); /* one private instance, explicit lifecycle */

/* Optional visual input for the integrator. Does not generate events/damage.
 * Set once per environment update, before worlds.step; reset clears it.
 * intensity [0,1], direction [-1,1], warning dims the decorative overlay.
 * Until obstacles supplies this signal, background animation works normally. */
typedef struct WorldVisualEnvironment { float intensity, direction; bool warning; } WorldVisualEnvironment;
void WorldsSetEnvironment(WorldVisualEnvironment environment);

/* Read-only diagnostics for lifecycle/timing tests, no texture ownership transfer. */
typedef struct WorldsDiagnostics {
    bool ready;
    int world;
    double visual_steps;
    unsigned cache_builds, live_targets;
    unsigned target_ids[3];
} WorldsDiagnostics;
WorldsDiagnostics WorldsInspect(void);
#endif
