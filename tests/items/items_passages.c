/* Real obstacle/item modules. Teleported collector tests opportunities, not flight skill. */
#include "items/items.h"
#include "items/items_internal.h"
#include "obstacles/obstacles_internal.h"
#include "obstacles/obstacles.h"
#include "core/game.h"
#include "morph_check.h"
static void CheckHazardsPreserveItems(ItemsApi items,ObstaclesApi obstacles)
{
    const ObstacleHazardType types[]={HAZARD_WIND,HAZARD_JELLY,HAZARD_VINE,HAZARD_ICE,HAZARD_ROCK,HAZARD_FIRE};
    for(int w=0;w<6;w++){
        GameState g={.mode=GAME_PLAY,.world=w};const WorldConfig *cfg=obstacles.config(obstacles.life.context,w);
        items.life.reset_world(items.life.context,&g,cfg);obstacles.life.reset_world(obstacles.life.context,&g,cfg);
        ItemsState *is=items.life.context;ObstaclesState *os=obstacles.life.context;
        is->fragment_timer=is->boost_timer=10000;
        is->fragments[0]=(ItemFragmentView){0,{500,250},23,0,0};is->fragment_count=1;
        is->powers[0]=(ItemPowerView){1,BOOST_SHIELD,{500,250},21};is->power_count=1;
        os->hazards[0]=(ObstacleHazard){.type=types[w],.id=1,.x=600,.y=250,.base_y=250,.previous={600,250},.age=100,.radius=15,.speed=6};os->hazard_count=1;
        FrameContext f={.normal_steps=1,.world_steps=1,.normal_seconds=1.0f/60,.pace=1};
        PlayerView p={.position={180,42},.visual_radius=23,.collision_radius=18};bool crossed=false;
        for(int tick=0;tick<35;tick++){
            obstacles.step(obstacles.life.context,&g,&f,p,false);
            GameEventBuffer events={0};items.collect_step(items.life.context,&g,&f,p,obstacles.passages(obstacles.life.context),&events);
            MORPH_CHECK(events.count==0 && !events.overflow && is->collected==0);
            MORPH_CHECK(is->fragment_count==1 && is->fragments[0].id==0 && is->fragments[0].fade==0 && is->power_count==1);
            PlayerView probe=p;probe.position=(GameVec2){os->hazards[0].x,os->hazards[0].y};
            MORPH_CHECK(obstacles.touches_player(obstacles.life.context,probe));
            if(os->hazards[0].x<500)crossed=true;
        }
        MORPH_CHECK(crossed);p.position=is->fragments[0].position;GameEventBuffer events={0};
        items.collect_step(items.life.context,&g,&f,p,obstacles.passages(obstacles.life.context),&events);
        MORPH_CHECK(events.count==2 && is->collected==1 && is->fragments[0].fade>0 && items.view(items.life.context).shield);
        events=(GameEventBuffer){0};items.collect_step(items.life.context,&g,&f,p,obstacles.passages(obstacles.life.context),&events);
        MORPH_CHECK(events.count==0 && is->collected==1);
    }
    puts("HAZARD ITEMS: all six moving hazards cross energy and boosts without deleting/collecting; only player collects once");
}
int main(void)
{
    ItemsApi items=ItemsModuleCreate();ObstaclesApi obstacles=ObstaclesModuleCreate();
    MORPH_CHECK(items.life.init(items.life.context));MORPH_CHECK(obstacles.life.init(obstacles.life.context));
    CheckHazardsPreserveItems(items,obstacles);
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
    puts("PASS items-passages: six real worlds, rejected unsafe positions, missed IDs replenished, all required opportunities collected");return 0;
}
