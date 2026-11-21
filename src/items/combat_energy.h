#ifndef MORPH_COMBAT_ENERGY_H
#define MORPH_COMBAT_ENERGY_H
#include "core/game_types.h"
void CombatEnergyReset(CombatEnergy *energy, unsigned required);
bool CombatEnergyStep(CombatEnergy *energy, const FrameContext *frame, PlayerView player, bool magnet);
float CombatEnergyFraction(const CombatEnergy *energy);
#endif
