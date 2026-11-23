#include "obstacles/obstacles.h"
#include "obstacles/obstacles_internal.h"
#include "player/collision.h"
#include "morph_check.h"
#include <stdlib.h>
#include <string.h>

static ObstaclesApi api;
static GameState game;
static PlayerView player;
static const WorldConfig *config;
static void Reset(int world,uint32_t seed)
{
    game=(GameState){0};game.mode=GAME_PLAY;game.world=world;game.lives=3;game.score=0;
    player=(PlayerView){0};player.position=(GameVec2){180,245};player.collision_radius=18;
    config=api.config(api.life.context,world);MORPH_CHECK(config);
    ObstaclesSetSeed(seed);api.life.reset_world(api.life.context,&game,config);
}
static void Tick(float scale,bool spawn,float pace,float scroll)
{
    FrameContext frame={0};frame.world_steps=scale;frame.normal_steps=1;frame.world_scale=scale;frame.pace=pace;
    frame.scroll_delta=scroll;game.world_steps+=scale;game.scroll+=scroll;
    GameState before=game;
    api.step(api.life.context,&game,&frame,player,spawn);
    MORPH_CHECK(memcmp(&game,&before,sizeof(game))==0);
}
static void Generate(void){Tick(1,true,1,config->base_speed);}
static void CheckGeometryAndWarnings(void)
{
    const ObstacleHazardType expected[]={HAZARD_WIND,HAZARD_JELLY,HAZARD_VINE,HAZARD_ICE,HAZARD_ROCK,HAZARD_FIRE};
    for(int w=0;w<6;w++) {
        Reset(w,123);const ObstacleHazardConfig *hc=ObstaclesHazardSettings(w);
        MORPH_CHECK_NEAR(hc->interval/60.0f,8.5f,.00001f); /* 510 passos = 8,5 s */
        if(w){MORPH_CHECK(hc->speed>ObstaclesHazardSettings(w-1)->speed);MORPH_CHECK(hc->interval==ObstaclesHazardSettings(w-1)->interval);}
        for(int i=0;i<600 && !ObstaclesInspect().hazard_count;i++)Generate();
        ObstaclesState *state=api.life.context;MORPH_CHECK(state->hazard_count==1);
        state->column_count=0; /* prove it survives removal of every totem */
        ObstacleHazard initial=state->hazards[0];MORPH_CHECK(initial.type==expected[w]);
        unsigned warnings=0,active=0;bool crossed=false;
        while(state->hazard_count){
            ObstacleHazard before=state->hazards[0];Tick(1,false,1,config->base_speed);
            if(!state->hazard_count){MORPH_CHECK(before.x+before.radius-before.speed<0);break;}
            ObstacleHazard h=state->hazards[0];MORPH_CHECK(h.id==initial.id && h.x<before.x);
            MORPH_CHECK(fabsf(h.y-h.base_y)<=hc->amplitude+.001);
            PlayerView at=player;at.position=(GameVec2){h.x,h.y};
            if(h.age<hc->warning){warnings++;MORPH_CHECK(!api.touches_player(api.life.context,at));}
            else{active++;MORPH_CHECK(api.touches_player(api.life.context,at));}
            if(h.x<player.position.x)crossed=true;
            PassageQuery q=api.passages(api.life.context);MORPH_CHECK(q.is_safe(q.context,at.position,23)); /* hazards do not invalidate items */
        }
        MORPH_CHECK(warnings && active && crossed);
        printf("HORIZONTAL world=%d warning=%u active=%u crossed-player-and-left-edge\n",w,warnings,active);
    }
}
static void CheckCrossingAndSweep(void)
{
    for(int w=0;w<6;w++){
        Reset(w,17);while(!ObstaclesInspect().hazard_count)Generate();
        ObstaclesState *s=api.life.context;ObstacleHazard *h=&s->hazards[0];
        h->x=400;h->previous=(GameVec2){400,h->y};h->age=100;
        /* A stationary solid column across the path must never absorb the shot. */
        s->column_count=1;s->columns[0].x=280;s->columns[0].width=72;
        s->columns[0].base_y=400;s->columns[0].gap_y=400;s->columns[0].gap=80;s->columns[0].motion=0;
        uint32_t id=h->id;bool overlapped=false;
        while(s->hazard_count && s->hazards[0].x>240){
            Tick(1,false,1,0);MORPH_CHECK(s->hazard_count==1 && s->hazards[0].id==id);
            if(s->hazards[0].x>=280 && s->hazards[0].x<=352)overlapped=true;
        }
        MORPH_CHECK(overlapped);s->column_count=0;
        h=&s->hazards[0];h->x=240;h->speed=120;h->base_y=245;h->phase=0;
        Tick(1,false,1,0);MORPH_CHECK(h->x==120);
        PlayerView probe=player;probe.position=(GameVec2){180,245};
        MORPH_CHECK(api.touches_player(api.life.context,probe));
        probe.position.y=400;MORPH_CHECK(!api.touches_player(api.life.context,probe));
    }
}
static void CheckSpacing(void)
{
    const float old_interval[]={92,86,80,74,68,62};
    float previous=INFINITY;
    for(int w=0;w<6;w++){
        Reset(w,3);MORPH_CHECK_NEAR(config->obstacle_interval,old_interval[w]*.85f,.001);
        MORPH_CHECK(config->obstacle_interval<previous);previous=config->obstacle_interval;
        while(ObstaclesInspect().column_count<2)Generate();
        ObstaclesSnapshot s=ObstaclesInspect();
        float spacing=s.columns[1].x-s.columns[0].x;
        float expected=config->base_speed*config->obstacle_interval/.98f;
        MORPH_CHECK(fabsf(spacing-expected)<=config->base_speed+1);
        MORPH_CHECK(spacing<config->base_speed*old_interval[w]/.98f*.88f);
    }
}
static void CheckGeneration(void)
{
    unsigned pairs=0;
    for(uint32_t seed=1;seed<=12;seed++)for(int world=0;world<6;world++) {
        Reset(world,seed);unsigned rewards=0,last_generated=0;
        for(int t=0;t<5000;t++) {
            float pace=.98f+.13f*(.5f+.5f*sinf(t*.013f));
            Tick(1,true,pace,config->base_speed*pace);
            ObstaclesSnapshot s=ObstaclesInspect();
            MORPH_CHECK(s.column_count<=OBSTACLE_CAPACITY && s.hazard_count<=HAZARD_CAPACITY);
            if(s.generated!=last_generated && s.column_count>=2) {
                const ObstacleColumn *a=&s.columns[s.column_count-2],*b=&s.columns[s.column_count-1];
                float lo=fmaxf(a->base_y-a->gap/2+a->motion,b->base_y-b->gap/2+b->motion);
                float hi=fminf(a->base_y+a->gap/2-a->motion,b->base_y+b->gap/2-b->motion);
                MORPH_CHECK(hi-lo>=79.999f);pairs++;
                MORPH_CHECK(fabsf(a->base_y-b->base_y)<=fminf(44,config->gap-2*config->motion-80)+.001f);
            }
            last_generated=s.generated;
            for(unsigned i=1;i<s.column_count;i++)MORPH_CHECK(s.columns[i-1].x+s.columns[i-1].width<s.columns[i].x);
            for(unsigned i=0;i<s.hazard_count;i++) {
                const ObstacleHazard *h=&s.hazards[i];
                MORPH_CHECK(h->x+h->radius>=0 && h->y-h->radius>42 && h->y+h->radius<487);
            }
            rewards+=api.take_passed(api.life.context);MORPH_CHECK(api.take_passed(api.life.context)==0);
        }
        unsigned total=ObstaclesInspect().generated;
        for(int t=0;t<600;t++){Tick(1,false,1,config->base_speed);rewards+=api.take_passed(api.life.context);}
        MORPH_CHECK(ObstaclesInspect().generated==total && rewards==total);
        MORPH_CHECK(ObstaclesInspect().column_count==0 && ObstaclesInspect().hazard_count==0);
        MORPH_CHECK(api.path_clear(api.life.context,player));
    }
    printf("GENERATION: %u adjacent envelope pairs across 72 seed/world cases\n",pairs);
}
static void CheckPauseSlowCapacity(void)
{
    Reset(0,42);for(int i=0;i<68;i++)Generate();ObstaclesSnapshot normal=ObstaclesInspect();
    Reset(0,42);for(int i=0;i<100;i++)Tick(.68f,true,1,config->base_speed*.68f);
    ObstaclesSnapshot slow=ObstaclesInspect();MORPH_CHECK(normal.column_count==slow.column_count);
    MORPH_CHECK_NEAR(normal.clock,slow.clock,.0001);
    MORPH_CHECK_NEAR(normal.columns[0].x,slow.columns[0].x,.02);
    MORPH_CHECK_NEAR(normal.columns[0].gap_y,slow.columns[0].gap_y,.001);
    game.mode=GAME_PAUSED;ObstaclesSnapshot before=ObstaclesInspect();
    for(int i=0;i<200;i++)Tick(1,true,1,config->base_speed);
    ObstaclesSnapshot after=ObstaclesInspect();MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
    Reset(4,99);for(int i=0;i<4000;i++)Tick(1,true,1,0);
    MORPH_CHECK(ObstaclesInspect().column_count==OBSTACLE_CAPACITY);
    MORPH_CHECK(ObstaclesInspect().capacity_delays>0);
    Reset(4,99);for(int i=0;i<240;i++)Generate();before=ObstaclesInspect();
    Reset(4,99);for(int i=0;i<240;i++)Generate();after=ObstaclesInspect();MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
    game.mode=GAME_PORTAL;Tick(1,true,1,3);
    MORPH_CHECK(!ObstaclesInspect().environment.active && ObstaclesInspect().hazard_count==0);
    MORPH_CHECK(!api.touches_player(api.life.context,player) && api.vertical_force(api.life.context)==0);
}
static void CheckHazardSlow(void)
{
    for(int w=0;w<6;w++){
        Reset(w,42);for(int i=0;i<260;i++)Generate();ObstaclesSnapshot normal=ObstaclesInspect();
        Reset(w,42);for(int i=0;i<192;i++)Generate();
        for(int i=0;i<100;i++)Tick(.68f,true,1,config->base_speed*.68f);
        ObstaclesSnapshot slow=ObstaclesInspect();
        printf("SLOW HAZARDS world=%d normal=%u slow=%u\n",w,normal.hazard_count,slow.hazard_count);
        MORPH_CHECK(normal.hazard_count>0 && normal.hazard_count==slow.hazard_count);
        MORPH_CHECK_NEAR(normal.hazards[0].x,slow.hazards[0].x,.05);
        MORPH_CHECK_NEAR(normal.hazards[0].y,slow.hazards[0].y,.1);
        game.mode=GAME_PAUSED;ObstaclesSnapshot paused=ObstaclesInspect();
        for(int i=0;i<100;i++)Tick(1,true,1,config->base_speed);
        ObstaclesSnapshot after=ObstaclesInspect();MORPH_CHECK(memcmp(&paused,&after,sizeof(after))==0);
    }
}
static void CheckProtection(void)
{
    Reset(1,10);for(int i=0;i<400 && !ObstaclesInspect().hazard_count;i++)Generate();
    ObstaclesSnapshot s=ObstaclesInspect();MORPH_CHECK(s.hazard_count>0);
    ObstacleProtectedCircle item={{s.hazards[0].x,s.hazards[0].y},23};
    MORPH_CHECK(ObstaclesSetProtectedItems(&item,1));item.center=(GameVec2){-500,-500};
    Generate();MORPH_CHECK(ObstaclesInspect().hazard_count==s.hazard_count); /* an item never erases a launched hazard */
    MORPH_CHECK(!ObstaclesSetProtectedItems(NULL,1));
    MORPH_CHECK(!ObstaclesSetProtectedItems(&item,OBSTACLE_PROTECTED_CAPACITY+1));
    MORPH_CHECK(ObstaclesSetProtectedItems(NULL,0));
}
typedef struct Flight {float y,vy,rank;} Flight;
static int Rank(const void *a,const void *b)
{float delta=((const Flight *)a)->rank-((const Flight *)b)->rank;return delta<0?-1:delta>0?1:0;}
static void CheckReachability(void)
{
    /* Beam search with the reference fixed-step physics; actual production
     * collision queries, no shield, no invulnerability and no permitted damage.
     * This is sampled reachability, not a proof for all seeds/human reactions. */
    for(uint32_t seed=20;seed<22;seed++)for(int world=0;world<6;world++) {
        Reset(world,seed);Flight beam[512]={{245,0,0}};unsigned count=1;
        int steps=(int)config->duration_steps+300;
        for(int tick=0;tick<steps;tick++) {
            Generate();Flight candidates[1024];unsigned n=0;
            PassageQuery q=api.passages(api.life.context);float target=q.safe_y(q.context,player.position.x+120,260);
            float force=api.vertical_force(api.life.context);
            for(unsigned i=0;i<count;i++)for(int flap=0;flap<2;flap++) {
                Flight next=beam[i];if(flap)next.vy=-6.4f;
                next.vy=fminf(7.2f,next.vy+.34f+force);next.y+=next.vy;
                if(next.y<42){next.y=42;next.vy=1;}if(next.y>487)continue;
                PlayerView probe=player;probe.position.y=next.y;
                if(api.touches_player(api.life.context,probe))continue;
                next.rank=fabsf(next.y-target)+fabsf(next.vy)*2;candidates[n++]=next;
            }
            if(!n){fprintf(stderr,"No sampled route: world=%d seed=%u tick=%d\n",world,seed,tick);}
            MORPH_CHECK(n>0);qsort(candidates,n,sizeof(*candidates),Rank);count=0;
            for(unsigned i=0;i<n && count<512;i++) {
                bool duplicate=false;
                for(unsigned j=0;j<count;j++)if(fabsf(beam[j].y-candidates[i].y)<2 && fabsf(beam[j].vy-candidates[i].vy)<.4f){duplicate=true;break;}
                if(!duplicate)beam[count++]=candidates[i];
            }
        }
        printf("ROUTE world=%d seed=%u steps=%d survivors=%u\n",world,seed,steps,count);
    }
}
int main(void)
{
    api=ObstaclesModuleCreate();MORPH_CHECK(api.life.init(api.life.context));
    MORPH_CHECK(!api.config(api.life.context,-1) && !api.config(api.life.context,6));
    CheckGeometryAndWarnings();CheckCrossingAndSweep();CheckSpacing();CheckGeneration();CheckPauseSlowCapacity();CheckProtection();CheckHazardSlow();CheckReachability();
    api.life.shutdown(api.life.context);api.life.shutdown(api.life.context);
    puts("PASS obstacles-rules: actual geometry, warning gates, protected items, corridor envelopes, rewards, clock, bounded capacity and sampled routes.");
    return 0;
}
