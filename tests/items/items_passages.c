/* Real obstacle/item modules. Teleported collector tests opportunities, not flight skill. */
#include "items/items.h"
#include "obstacles/obstacles.h"
#include "core/game.h"
#include "morph_check.h"
int main(void)
{
    ItemsApi items=ItemsModuleCreate();ObstaclesApi obstacles=ObstaclesModuleCreate();
    MORPH_CHECK(items.life.init(items.life.context));MORPH_CHECK(obstacles.life.init(obstacles.life.context));
    for(int w=0;w<6;w++){
        GameState g={.mode=GAME_PLAY,.world=w};const WorldConfig *cfg=obstacles.config(obstacles.life.context,w);
        g.minimum_distance=cfg->base_speed*cfg->duration_steps;
        items.life.reset_world(items.life.context,&g,cfg);obstacles.life.reset_world(obstacles.life.context,&g,cfg);
        unsigned count=0,missed=0;bool seen[MORPH_MAX_ENERGY]={0};int tick;
        for(tick=0;tick<18000 && count<GameEnergyRequired(w);tick++){
            FrameContext f={.normal_steps=1,.normal_seconds=1.0f/60,.pace=1};items.step_timers(items.life.context,&f);
            f.world_scale=items.view(items.life.context).world_scale;f.world_steps=f.world_scale;f.world_seconds=f.world_scale/60;
            f.scroll_delta=cfg->base_speed*f.world_scale;g.world_steps+=f.world_steps;g.scroll+=f.scroll_delta;
            g.distance=fminf(g.minimum_distance,g.distance+f.scroll_delta);
            ItemsSnapshot snapshot=ItemsInspect();ObstacleProtectedCircle protected_items[MORPH_MAX_ENERGY];size_t n=0;
            for(unsigned i=0;i<snapshot.fragment_count;i++)if(snapshot.fragments[i].fade==0){
                ItemFragmentView t=snapshot.fragments[i];protected_items[n++]=(ObstacleProtectedCircle){{t.position.x-f.scroll_delta,t.position.y},t.radius};
            }
            MORPH_CHECK(ObstaclesSetProtectedItems(protected_items,n));
            PlayerView p={.position={180,42},.visual_radius=23,.collision_radius=18};
            obstacles.step(obstacles.life.context,&g,&f,p,true);
            for(unsigned i=0;i<snapshot.fragment_count;i++){
                ItemFragmentView t=snapshot.fragments[i];if(t.fade>0)continue;
                if(t.position.x>=180 && t.position.x-f.scroll_delta<180){
                    if(missed<4)missed++;else p.position=t.position;
                }
            }
            GameEventBuffer events={0};items.collect_step(items.life.context,&g,&f,p,obstacles.passages(obstacles.life.context),&events);
            MORPH_CHECK(!events.overflow);
            for(size_t i=0;i<events.count;i++)if(events.values[i].type==EVENT_FRAGMENT_COLLECTED){
                unsigned id=events.values[i].entity_id;MORPH_CHECK(id<GameEnergyRequired(w) && !seen[id]);seen[id]=true;count++;
            }
            MORPH_CHECK(items.view(items.life.context).collected==count);
        }
        printf("PASSAGES world=%d collected=%u missed=%u steps=%d\n",w,count,missed,tick);
        MORPH_CHECK(count==GameEnergyRequired(w) && missed==4);
    }
    items.life.shutdown(items.life.context);obstacles.life.shutdown(obstacles.life.context);
    puts("PASS items-passages: six real worlds, rejected unsafe positions, missed IDs replenished, all 84 opportunities collected");return 0;
}
