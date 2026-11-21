/* Private fixtures below are test-only. Production exposes snapshots by value. */
#include "items/items_internal.h"
#include "core/game.h"
#include "morph_check.h"
#include <string.h>
#include <math.h>
static ItemsApi api;
static ItemsState *s;
static GameState game;
static PlayerView player;
static bool allow=true;
static float SafeY(void *ctx,float x,float y){(void)ctx;(void)x;return y;}
static bool Safe(void *ctx,GameVec2 p,float r){(void)ctx;(void)p;(void)r;return allow;}
static PassageQuery query={NULL,SafeY,Safe};
static FrameContext frame={.normal_steps=1,.world_steps=1,.normal_seconds=1.0f/60,.world_seconds=1.0f/60,.world_scale=1,.pace=1};
static void Reset(int w)
{
    game=(GameState){.mode=GAME_PLAY,.world=w,.lives=3,.score=123};
    WorldConfig cfg={0};cfg.duration_steps=1500+w*600;
    api.life.reset_world(api.life.context,&game,&cfg);allow=true;
    player=(PlayerView){.position={180,245},.visual_radius=23,.collision_radius=18};
    frame.world_steps=1;frame.scroll_delta=0;
}
static GameEventBuffer Step(void)
{
    GameEventBuffer b={0};GameState before=game;
    api.collect_step(api.life.context,&game,&frame,player,query,&b);
    MORPH_CHECK(memcmp(&before,&game,sizeof(game))==0);return b;
}
static void Power(BoostType type)
{
    s->powers[0]=(ItemPowerView){++s->next_power_id,type,player.position,21};s->power_count=1;
    GameEventBuffer b=Step();MORPH_CHECK(b.count==1 && b.values[0].type==EVENT_BOOST_COLLECTED && b.values[0].boost==type);
}
static void Economy(void)
{
    unsigned total=0;
    for(int w=0;w<6;w++){
        Reset(w);const unsigned goals[]={4,6,8,10,12,16};unsigned required=goals[w];MORPH_CHECK(ItemsEnergy().required==required);
        for(unsigned id=0;id<required;id++){
            s->fragment_timer=0;player.position=(GameVec2){-1000,0};Step();
            unsigned index=s->fragment_count-1;MORPH_CHECK(s->fragments[index].id==id);
            player.position=s->fragments[index].position;
            GameEventBuffer b=Step();MORPH_CHECK(b.count>=1 && b.count<=2 && b.values[0].entity_id==id && b.values[0].multiplier==1);
            if(b.count==2)MORPH_CHECK(b.values[1].type==EVENT_BOOST_COLLECTED && b.values[1].boost==BOOST_LIFE);
            MORPH_CHECK(b.values[0].type==EVENT_FRAGMENT_COLLECTED);total++;
            MORPH_CHECK(Step().count==0);ItemEnergyView e=ItemsEnergy();
            MORPH_CHECK(e.collected==id+1 && e.full==(id+1==required));
            MORPH_CHECK_NEAR(e.fraction,(float)(id+1)/required,.00001);
            if(!e.full)MORPH_CHECK(e.fraction<1);
            for(int i=0;i<23;i++){Step();}MORPH_CHECK(s->fragment_count==0);
        }
        s->fragment_timer=0;Step();MORPH_CHECK(s->fragment_count==0);
        ItemsView copy=api.view(s);copy.collected=0;MORPH_CHECK(api.view(s).collected==required && copy.collected==0);
    }
    MORPH_CHECK(total==56);puts("ENERGY: all 56 fragment IDs, partial/full battery, fade and duplicate collection checked");
    Reset(5);game.distance=game.minimum_distance=99999;player.position=(GameVec2){-1000,0};
    for(int cycle=0;cycle<50;cycle++){
        s->fragment_timer=0;Step();MORPH_CHECK(s->fragment_count==1 && s->fragments[0].id==0);
        frame.scroll_delta=1100;Step();frame.scroll_delta=0;MORPH_CHECK(s->fragment_count==0);
    }
    allow=false;s->fragment_timer=0;Step();MORPH_CHECK(s->fragment_count==0);
    allow=true;for(int t=0;t<200;t++)Step();MORPH_CHECK(s->fragment_count>0);
    /* Queue overflow does not spend the fragment or award energy silently. */
    player.position=s->fragments[0].position;GameEventBuffer full={0};full.count=MORPH_MAX_EVENTS;
    api.collect_step(s,&game,&frame,player,query,&full);MORPH_CHECK(full.overflow && s->collected==0);
    MORPH_CHECK(Step().count==1 && s->collected==1);
}
typedef struct SpawnSequence {float first;BoostType types[12];float heights[12],delays[12];} SpawnSequence;
static SpawnSequence Sequence(int world,uint32_t run)
{
    Reset(world);game.run_id=run;WorldConfig cfg={.duration_steps=1500+world*600.0f};
    api.life.reset_world(s,&game,&cfg);player.position=(GameVec2){-1000,0};
    SpawnSequence result={0};result.first=s->boost_timer;
    for(unsigned i=0;i<12;i++){
        s->fragment_timer=10000;s->power_count=0;s->boost_timer=0;Step();
        MORPH_CHECK(s->power_count==1);result.types[i]=s->powers[0].type;
        result.heights[i]=s->powers[0].position.y;result.delays[i]=s->boost_timer;
        MORPH_CHECK(result.delays[i]>=500 && result.delays[i]<940);
    }
    return result;
}
static void RandomRuns(void)
{
    ItemsSetSeed(123456);
    for(int world=0;world<6;world++){
        unsigned seen=0;SpawnSequence previous=Sequence(world,1),replay=Sequence(world,1);
        MORPH_CHECK(memcmp(&previous,&replay,sizeof(previous))==0);
        for(uint32_t run=2;run<=13;run++){
            SpawnSequence next=Sequence(world,run);
            MORPH_CHECK(next.first>=360 && next.first<680 && next.first!=previous.first);
            MORPH_CHECK(memcmp(next.types,previous.types,sizeof(next.types))!=0);
            MORPH_CHECK(memcmp(next.heights,previous.heights,sizeof(next.heights))!=0);
            for(unsigned i=0;i<12;i++)seen|=1u<<next.types[i];
            previous=next;
        }
        MORPH_CHECK(seen==((1u<<BOOST_SHIELD)|(1u<<BOOST_MAGNET)|(1u<<BOOST_SLOW)|(1u<<BOOST_DOUBLE)|(1u<<BOOST_PHASE)));
    }
    SpawnSequence old=Sequence(0,1);ItemsSetSeed(987654);SpawnSequence fresh=Sequence(0,1);
    MORPH_CHECK(memcmp(old.types,fresh.types,sizeof(old.types))!=0);ItemsSetSeed(505);
    puts("RANDOM: reproducible explicit seed; new runs/session vary types, heights and timing; all five types in every world");
}
static void PhaseBoost(void)
{
    Reset(0);Power(BOOST_PHASE);MORPH_CHECK(s->boost_count==1);
    frame.world_steps=.68f;
    for(int i=0;i<300;i++){api.step_timers(s,&frame);}
    MORPH_CHECK_NEAR(api.view(s).boosts[0].seconds_left,2,.00001);
    Power(BOOST_PHASE);MORPH_CHECK(s->boost_count==1);
    MORPH_CHECK_NEAR(api.view(s).boosts[0].seconds_left,7,.00001);
    for(int i=0;i<419;i++){api.step_timers(s,&frame);}
    MORPH_CHECK(s->boost_count==1);api.step_timers(s,&frame);MORPH_CHECK(s->boost_count==0);
    Power(BOOST_PHASE);Power(BOOST_SHIELD);Power(BOOST_SLOW);
    MORPH_CHECK(s->boost_count==2 && s->boosts[0].type==BOOST_SHIELD && s->boosts[1].type==BOOST_SLOW);
    Reset(0);player.position=(GameVec2){-1000,0};unsigned counts[6]={0};
    for(int i=0;i<1000;i++){
        s->fragment_timer=10000;s->power_count=0;s->boost_timer=0;Step();
        MORPH_CHECK(s->power_count==1 && s->powers[0].type!=BOOST_LIFE);
        counts[s->powers[0].type]++;
    }
    const BoostType random_types[]={BOOST_SHIELD,BOOST_MAGNET,BOOST_SLOW,BOOST_DOUBLE,BOOST_PHASE};
    for(unsigned i=0;i<5;i++)MORPH_CHECK(counts[random_types[i]]>100 && counts[random_types[i]]<300);
}
static void ExtraLife(void)
{
    for(int w=0;w<6;w++){
        Reset(w);s->fragment_timer=10000;s->boost_timer=10000;
        Power(BOOST_SHIELD);Power(BOOST_MAGNET);
        s->collected=s->required/2;s->life_timer=0;player.position=(GameVec2){-1000,0};
        Step();MORPH_CHECK(s->power_count==(unsigned)(w%2));
        if(w%2==0)continue;
        MORPH_CHECK(s->powers[0].type==BOOST_LIFE);
        s->powers[0].position.x=-60;Step();MORPH_CHECK(s->power_count==0 && !s->life_collected);
        s->life_timer=0;Step();MORPH_CHECK(s->power_count==1);
        player.position=s->powers[0].position;GameEventBuffer e=Step();
        MORPH_CHECK(e.count==1 && e.values[0].boost==BOOST_LIFE && s->life_collected);
        MORPH_CHECK(s->boost_count==2 && api.view(s).shield && s->boosts[1].type==BOOST_MAGNET);
        for(int i=0;i<200;i++){Step();}MORPH_CHECK(s->power_count==0);
    }
}
static void Boosts(void)
{
    Reset(0);Power(BOOST_SHIELD);Power(BOOST_SHIELD);
    MORPH_CHECK(s->boost_count==1 && api.consume_shield(s) && !api.consume_shield(s));
    Power(BOOST_MAGNET);for(int i=0;i<60;i++)api.step_timers(s,&frame);
    Power(BOOST_SLOW);Power(BOOST_MAGNET);MORPH_CHECK_NEAR(api.view(s).boosts[0].seconds_left,7,.00001);
    Power(BOOST_DOUBLE);ItemsView v=api.view(s);MORPH_CHECK(v.boost_count==2 && v.boosts[0].type==BOOST_SLOW && v.boosts[1].type==BOOST_DOUBLE);
    MORPH_CHECK(v.world_scale==.68f && v.score_multiplier==2);
    for(int i=0;i<240;i++)api.step_timers(s,&frame);
    v=api.view(s);MORPH_CHECK(v.world_scale==1 && v.score_multiplier==2);
    for(int i=0;i<180;i++)api.step_timers(s,&frame);
    MORPH_CHECK(api.view(s).boost_count==0);
    Power(BOOST_SHIELD);for(int i=0;i<10000;i++)api.step_timers(s,&frame);MORPH_CHECK(api.view(s).shield);
    Reset(0);Power(BOOST_MAGNET);s->fragments[0]=(ItemFragmentView){0,{280,245},23,0,0};s->fragment_count=1;
    frame.world_steps=.68f;Step();MORPH_CHECK_NEAR(s->fragments[0].position.x,280-8*.68,.001);
    for(int i=0;i<300;i++){api.step_timers(s,&frame);}MORPH_CHECK(s->boost_count==1);
    for(int i=0;i<120;i++){api.step_timers(s,&frame);}MORPH_CHECK(s->boost_count==0);
    Reset(0);s->fragments[0]=(ItemFragmentView){0,player.position,23,0,0};s->fragment_count=1;
    s->powers[0]=(ItemPowerView){1,BOOST_DOUBLE,player.position,21};s->power_count=1;
    GameEventBuffer b=Step();MORPH_CHECK(b.count==2 && b.values[0].type==EVENT_FRAGMENT_COLLECTED && b.values[0].multiplier==1);
    MORPH_CHECK(api.view(s).score_multiplier==2);
    s->fragments[1]=(ItemFragmentView){1,player.position,23,0,0};s->fragment_count=2;
    b=Step();MORPH_CHECK(b.count==1 && b.values[0].multiplier==2);
    game.mode=GAME_PAUSED;ItemsSnapshot before=ItemsInspect();for(int i=0;i<20;i++)Step();ItemsSnapshot after=ItemsInspect();
    MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
    Reset(3);MORPH_CHECK(s->collected==0 && s->boost_count==0 && s->power_count==0 && s->fragment_count==0);
    Reset(0);player.position=(GameVec2){-1000,0};
    int initial=(int)ceilf(s->boost_timer);MORPH_CHECK(initial>=360 && initial<=680);
    for(int i=0;i<initial-1;i++){Step();}MORPH_CHECK(s->power_count==0);Step();MORPH_CHECK(s->power_count==1 && s->boost_timer>=500 && s->boost_timer<940);
    Reset(0);for(int i=0;i<20;i++){s->boost_timer=0;Step();}MORPH_CHECK(s->power_count==ITEMS_POWER_CAPACITY);
    GameEventBuffer bad={0};api.collect_step(s,&game,&frame,player,(PassageQuery){0},&bad);MORPH_CHECK(bad.overflow);
    puts("BOOSTS: renewal order, eviction, single shield charge, normal-clock expiration, magnet, queue and spawn timing checked");
}
int main(void)
{
    api=ItemsModuleCreate();MORPH_CHECK(api.life.init(api.life.context));s=api.life.context;
    Economy();ExtraLife();Boosts();PhaseBoost();RandomRuns();api.life.shutdown(s);api.life.shutdown(s);puts("PASS items-rules");return 0;
}
