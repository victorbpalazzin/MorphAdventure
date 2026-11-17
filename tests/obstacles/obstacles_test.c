#include "obstacles/obstacles.h"
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
    unsigned warning_seen=0,active_seen=0;
    for(int w=0;w<6;w++) {
        Reset(w,123);Generate();ObstaclesSnapshot s=ObstaclesInspect();MORPH_CHECK(s.column_count==1);
        ObstacleColumn c=s.columns[0];PlayerView p=player;p.position=(GameVec2){c.x+c.width/2,1};
        MORPH_CHECK(api.touches_player(api.life.context,p));p.position.y=c.gap_y;
        MORPH_CHECK(!api.touches_player(api.life.context,p));
        PassageQuery q=api.passages(api.life.context);
        MORPH_CHECK(q.is_safe(q.context,p.position,23));
        MORPH_CHECK_NEAR(q.safe_y(q.context,p.position.x,100),c.base_y,.001);
        MORPH_CHECK(!q.is_safe(q.context,(GameVec2){p.position.x,1},23));
        for(int step=0;step<1300;step++) {
            Generate();s=ObstaclesInspect();ObstacleEnvironmentView env=ObstaclesEnvironment();
            if(env.warning)MORPH_CHECK(api.vertical_force(api.life.context)==0);
            MORPH_CHECK(fabsf(api.vertical_force(api.life.context))<=fabsf(config->vertical_force)+.00001f);
            for(unsigned i=0;i<s.hazard_count;i++) {
                ObstacleHazard h=s.hazards[i];p.position=(GameVec2){h.x,h.y};
                if(h.age<85){warning_seen++;MORPH_CHECK(!api.touches_player(api.life.context,p));}
                else {active_seen++;MORPH_CHECK(api.touches_player(api.life.context,p));}
                MORPH_CHECK(!q.is_safe(q.context,p.position,23));
            }
        }
    }
    MORPH_CHECK(warning_seen>0 && active_seen>0);
    printf("WARNINGS: %u warning samples, %u active samples checked\n",warning_seen,active_seen);
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
                for(unsigned j=0;j<s.column_count;j++)if(s.columns[j].id==h->anchor_id)
                    MORPH_CHECK(fabsf(h->y-s.columns[j].gap_y)>=40+h->radius+1.999f);
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
static void CheckProtection(void)
{
    Reset(1,10);for(int i=0;i<400 && !ObstaclesInspect().hazard_count;i++)Generate();
    ObstaclesSnapshot s=ObstaclesInspect();MORPH_CHECK(s.hazard_count>0);
    ObstacleProtectedCircle item={{s.hazards[0].x,s.hazards[0].y},23};
    MORPH_CHECK(ObstaclesSetProtectedItems(&item,1));item.center=(GameVec2){-500,-500};
    Generate();MORPH_CHECK(ObstaclesInspect().hazard_count==0); /* copied, not borrowed */
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
        for(int tick=0;tick<2400;tick++) {
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
        printf("ROUTE world=%d seed=%u steps=2400 survivors=%u\n",world,seed,count);
    }
}
int main(void)
{
    api=ObstaclesModuleCreate();MORPH_CHECK(api.life.init(api.life.context));
    MORPH_CHECK(!api.config(api.life.context,-1) && !api.config(api.life.context,6));
    CheckGeometryAndWarnings();CheckGeneration();CheckPauseSlowCapacity();CheckProtection();CheckReachability();
    api.life.shutdown(api.life.context);api.life.shutdown(api.life.context);
    puts("PASS obstacles-rules: actual geometry, warning gates, protected items, corridor envelopes, rewards, clock, bounded capacity and sampled routes.");
    return 0;
}
