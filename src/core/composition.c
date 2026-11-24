#include "module_api.h"
#include <string.h>
#include "player/player.h"
#include "worlds/worlds.h"
#include "obstacles/obstacles.h"
#include "items/items.h"
#include "ui/ui.h"
#include "effects/effects.h"
#include "audio/audio_cues.h"

static ObstaclesApi original_obstacles;
static PlayerApi original_player;
static EffectsApi original_effects;
static bool protection_failed;

/* ---- Áudio: escuta eventos reais sem alterar a lógica do jogo ---------------- */
static struct { uint32_t world_id; unsigned count; uint32_t ids[HAZARD_CAPACITY]; bool launched[HAZARD_CAPACITY];
                bool env_active, env_warning; } heard;

static void Tap(AudioTapKind kind, int param, int boost, float pan)
{
    AudioDirectorTap(AudioDirectorShared(), (AudioTap){kind, param, boost, pan});
}

static void ListenHazards(const GameState *game)
{
    ObstaclesSnapshot snap = ObstaclesInspect();
    const ObstacleHazardConfig *cfg = ObstaclesHazardSettings(snap.world);
    if (heard.world_id != game->world_id) { memset(&heard, 0, sizeof(heard)); heard.world_id = game->world_id; }
    uint32_t ids[HAZARD_CAPACITY]; bool launched[HAZARD_CAPACITY]; unsigned count = 0;
    for (unsigned i = 0; i < snap.hazard_count && i < HAZARD_CAPACITY; i++) {
        const ObstacleHazard *h = &snap.hazards[i];
        bool known = false, was_launched = false;
        for (unsigned k = 0; k < heard.count; k++) if (heard.ids[k] == h->id) { known = true; was_launched = heard.launched[k]; }
        bool now_launched = cfg && h->age >= cfg->warning;
        float pan = h->x > MORPH_WIDTH * 0.5f ? 0.6f : 0.2f;
        if (!known) Tap(TAP_HAZARD_SPAWN, (int)h->type, 0, pan);
        if (now_launched && !was_launched) Tap(TAP_HAZARD_LAUNCH, (int)h->type, 0, pan);
        ids[count] = h->id; launched[count] = now_launched; count++;
    }
    memcpy(heard.ids, ids, sizeof(ids)); memcpy(heard.launched, launched, sizeof(launched)); heard.count = count;
    ObstacleEnvironmentView env = snap.environment;
    if (env.active && env.warning && !(heard.env_active && heard.env_warning)) Tap(TAP_ENV_WARNING, snap.world, 0, 0);
    if (env.active && !env.warning && (!heard.env_active || heard.env_warning)) Tap(TAP_ENV_ACTIVE, snap.world, 0, 0);
    heard.env_active = env.active; heard.env_warning = env.warning;
}

static void StepPlayer(void *context, const GameState *game, const FrameContext *frame, const GameInput *input, float force)
{
    original_player.step(context, game, frame, input, force);
    /* Só conta como batida de asa o impulso que realmente foi aplicado. */
    if (input->impulse_pressed && (game->mode == GAME_PLAY || game->mode == GAME_BOSS) &&
        original_player.view(context).vertical_velocity < -5.0f) Tap(TAP_FLAP, 0, 0, 0);
}

static void EffectEvent(void *context, const GameEvent *event)
{
    Tap(TAP_GAME_EVENT, (int)event->type, (int)event->boost, event->position.x / MORPH_WIDTH * 2 - 1);
    original_effects.event(context, event);
}

static void StepObstacles(void *context,const GameState *game,const FrameContext *frame,
                          PlayerView player,bool spawn)
{
    ItemsSnapshot items=ItemsInspect();
    ObstacleProtectedCircle circles[MORPH_MAX_ENERGY];size_t count=0;
    for(unsigned i=0;i<items.fragment_count;i++)if(items.fragments[i].fade==0){
        ItemFragmentView item=items.fragments[i];
        circles[count++]=(ObstacleProtectedCircle){
            {item.position.x-frame->scroll_delta,item.position.y},item.radius};
    }
    protection_failed=!ObstaclesSetProtectedItems(circles,count);
    original_obstacles.step(context,game,frame,player,spawn);
    ListenHazards(game);
    ObstacleEnvironmentView environment=ObstaclesEnvironment();
    WorldsSetEnvironment((WorldVisualEnvironment){environment.intensity,environment.direction,environment.warning});
}

static PassageQuery Passages(void *context)
{
    /* The coordinator rejects missing queries and exposes GAME_ERROR rather than
     * silently continuing after invalid item-protection data. */
    if(protection_failed)return (PassageQuery){0};
    return original_obstacles.passages(context);
}

GameModules MorphCreateModules(void)
{
    GameModules modules={0};modules.contract_version=MORPH_CONTRACT_VERSION;
    modules.player=PlayerModuleCreate();original_player=modules.player;modules.player.step=StepPlayer;
    modules.worlds=WorldsModuleCreate();
    modules.obstacles=ObstaclesModuleCreate();original_obstacles=modules.obstacles;
    protection_failed=false;modules.obstacles.step=StepObstacles;modules.obstacles.passages=Passages;
    modules.items=ItemsModuleCreate();modules.ui=UiModuleCreate();modules.effects=EffectsModuleCreate();
    original_effects=modules.effects;modules.effects.event=EffectEvent;
    memset(&heard,0,sizeof(heard));AudioDirectorReset(AudioDirectorShared());
    return modules;
}
