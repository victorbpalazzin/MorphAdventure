#ifndef MORPH_ITEMS_H
#define MORPH_ITEMS_H
#include "core/module_api.h"
#define ITEMS_FRAGMENT_CAPACITY MORPH_MAX_ENERGY
#define ITEMS_POWER_CAPACITY 8
typedef struct ItemBoostDefinition {
    BoostType type;
    const char *name;
    float duration_seconds; /* negative: one shield charge, no timeout */
    uint32_t color;
} ItemBoostDefinition;
typedef struct ItemEnergyView { unsigned collected,required; float fraction; bool full; } ItemEnergyView;
typedef struct ItemFragmentView { unsigned id; GameVec2 position; float radius,spin,fade; } ItemFragmentView;
typedef struct ItemPowerView { unsigned id; BoostType type; GameVec2 position; float radius; } ItemPowerView;
typedef struct ItemsSnapshot {
    int world;
    unsigned fragment_count,power_count;
    ItemFragmentView fragments[ITEMS_FRAGMENT_CAPACITY];
    ItemPowerView powers[ITEMS_POWER_CAPACITY];
    float fragment_timer,boost_timer;
    bool configuration_valid;
} ItemsSnapshot;
ItemsApi ItemsModuleCreate(void);
ItemEnergyView ItemsEnergy(void); /* copy: UI cannot mutate the module */
ItemsSnapshot ItemsInspect(void); /* copy for integration/debugging */
const ItemBoostDefinition *ItemsBoostDefinition(BoostType type);
void ItemsSetSeed(uint32_t seed); /* session seed; combined with run_id/world, reproducible in tests */
/* Modo educativo: letra exibida no fragmento de progressão `id` do mundo atual.
 * Retorna '\0' no arcade (game->edu.enabled=false): o raio original é desenhado.
 * Só muda a representação: geração, IDs, colisão, magnetismo e pontos são os mesmos.
 * A energia de combate do Grootspectro nunca usa letras. */
char ItemsFragmentLetter(const GameState *game, unsigned id);
const char *ItemsWorldLetters(int world); /* conjunto próprio de cada mundo */
#endif
