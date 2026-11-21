#ifndef MORPH_BOSS_H
#define MORPH_BOSS_H
#include "core/game_types.h"
void BossBegin(BossState *boss, unsigned energy_required);
void BossStep(BossState *boss, const FrameContext *frame, PlayerView player);
bool BossTouches(const BossState *boss, PlayerView player);
void BossRegisterContact(BossState *boss, PlayerView player);
bool BossFire(BossState *boss, PlayerView player);
float BossTargetY(const BossState *boss, unsigned projectile);
GameVec2 BossMouth(const BossState *boss);
#endif
