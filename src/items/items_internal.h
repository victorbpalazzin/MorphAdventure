#ifndef MORPH_ITEMS_INTERNAL_H
#define MORPH_ITEMS_INTERNAL_H
#include "items.h"
typedef struct ActiveItemBoost { BoostType type; double seconds; } ActiveItemBoost;
typedef struct ItemsState {
    int world;
    bool life_collected,valid,collected_ids[MORPH_MAX_ENERGY];
    unsigned collected,required,fragment_count,power_count,boost_count,next_power_id;
    uint32_t seed,boost_seed;
    float life_timer,fragment_timer,boost_timer,fragment_interval;
    ActiveItemBoost boosts[MORPH_MAX_BOOSTS];
    ItemFragmentView fragments[ITEMS_FRAGMENT_CAPACITY];
    ItemPowerView powers[ITEMS_POWER_CAPACITY];
} ItemsState;
void ItemsDraw(const ItemsState *state);
#endif
