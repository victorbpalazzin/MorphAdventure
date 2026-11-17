#include "module_api.h"
#include "player/player.h"
#include "worlds/worlds.h"
#include "obstacles/obstacles.h"
#include "items/items.h"
#include "ui/ui.h"
#include "effects/effects.h"

static ObstaclesApi original_obstacles;
static bool protection_failed;

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
    modules.player=PlayerModuleCreate();modules.worlds=WorldsModuleCreate();
    modules.obstacles=ObstaclesModuleCreate();original_obstacles=modules.obstacles;
    protection_failed=false;modules.obstacles.step=StepObstacles;modules.obstacles.passages=Passages;
    modules.items=ItemsModuleCreate();modules.ui=UiModuleCreate();modules.effects=EffectsModuleCreate();
    return modules;
}
