#ifndef MORPH_PLAYER_H
#define MORPH_PLAYER_H
#include "core/module_api.h"

/* One game instance, private static storage; init/reset are explicit.
 * PortalPose.rotation and PlayerView.rotation are radians. */
PlayerApi PlayerModuleCreate(void);
/* Development overlay only; init turns it off. Never changes collision geometry. */
void PlayerSetDebugHitbox(bool enabled);
#endif
