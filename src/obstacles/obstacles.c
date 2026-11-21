#include "obstacles_internal.h"
#include "player/collision.h"
#include <math.h>
#include <string.h>

const WorldConfig ObstacleWorldConfigs[MORPH_WORLD_COUNT]={
    {"Ar",4.2f,210,78.2f,12,.019f,1500,440,.065f},
    {"Agua",4.6f,202,73.1f,16,.021f,2100,410,.080f},
    {"Floresta",5,194,68,20,.023f,2700,380,0},
    {"Gelo",5.4f,186,62.9f,24,.025f,3300,350,.090f},
    {"Terra",5.8f,178,57.8f,28,.027f,3900,320,.105f},
    {"Fogo",6.2f,170,52.7f,32,.029f,4500,290,0}
};
static const ObstacleHazardConfig hazard_configs[MORPH_WORLD_COUNT]={
    {5.58f,540,4,.035f,75,2}, {6.12f,540,6,.045f,72,2},
    {6.66f,540,5,.05f,70,2}, {7.20f,540,2,.055f,68,2},
    {7.74f,540,4,.05f,65,3}, {8.28f,540,6,.06f,62,3}
};
const ObstacleHazardConfig *ObstaclesHazardSettings(int world)
{return world>=0 && world<MORPH_WORLD_COUNT?&hazard_configs[world]:NULL;}
static ObstaclesState state;
static uint32_t reset_seed=907;
static float Clamp(float v,float lo,float hi){return fmaxf(lo,fminf(v,hi));}
static float Random(ObstaclesState *s)
{s->seed=s->seed*1664525u+1013904223u;return (float)((double)s->seed/4294967296.0);}
static const WorldConfig *Config(void *context,int world)
{(void)context;return world>=0 && world<MORPH_WORLD_COUNT?&ObstacleWorldConfigs[world]:NULL;}
static void Reset(void *context,const GameState *game,const WorldConfig *config)
{
    (void)config;ObstaclesState *s=context;bool debug=s->debug;
    memset(s,0,sizeof(*s));s->debug=debug;s->world=game->world;s->mode=game->mode;
    s->seed=reset_seed^(0x9e3779b9u*(unsigned)(game->world+1));s->event_timer=260;s->hazard_timer=180;
}
static bool Init(void *context){memset(context,0,sizeof(ObstaclesState));return true;}
static void Shutdown(void *context){memset(context,0,sizeof(ObstaclesState));}
void ObstaclesSetSeed(uint32_t seed){reset_seed=seed;}
void ObstaclesSetDebugGeometry(bool enabled){state.debug=enabled;}

size_t ObstaclesColumnPolygon(const ObstacleColumn *c,bool top,GameVec2 *out,size_t capacity)
{
    if(!c || !out || capacity<OBSTACLE_POLYGON_CAPACITY)return 0;
    float height=top?c->gap_y-c->gap/2:MORPH_HEIGHT-(c->gap_y+c->gap/2);
    height=Clamp(height,0,MORPH_HEIGHT);size_t n=0;
    out[n++]=(GameVec2){0,-6};out[n++]=(GameVec2){c->width,-6};
    for(float y=20;y<height-20;y+=28)out[n++]=(GameVec2){c->width-(c->world==0?5+sinf(y*.1f)*5:3+fmodf(y,3)*2),y};
    out[n++]=(GameVec2){c->width-5,height-12};out[n++]=(GameVec2){c->width*.78f,height-3};
    out[n++]=(GameVec2){c->width*.55f,height-9};out[n++]=(GameVec2){c->width*.34f,height};
    out[n++]=(GameVec2){c->width*.12f,height-7};out[n++]=(GameVec2){3,height-18};
    for(float y=height-35;y>10;y-=28)out[n++]=(GameVec2){c->world==0?5+sinf(y*.11f)*5:3+fmodf(y,4),y};
    for(size_t i=0;i<n;i++){out[i].x+=c->x;if(!top)out[i].y=MORPH_HEIGHT-out[i].y;}
    return n;
}
size_t ObstaclesIcePolygon(const ObstacleHazard *h,GameVec2 *out,size_t capacity)
{
    if(!h || !out || capacity<4)return 0;
    out[0]=(GameVec2){h->x-h->radius,h->y};out[1]=(GameVec2){h->x+3,h->y-h->radius*.7f};
    out[2]=(GameVec2){h->x+h->radius,h->y};out[3]=(GameVec2){h->x+3,h->y+h->radius*.7f};return 4;
}
static bool ColumnHit(const ObstacleColumn *c,GameVec2 center,float radius)
{
    if(center.x+radius<c->x || center.x-radius>c->x+c->width)return false;
    GameVec2 points[OBSTACLE_POLYGON_CAPACITY];
    size_t n=ObstaclesColumnPolygon(c,true,points,OBSTACLE_POLYGON_CAPACITY);
    if(CollisionCirclePolygon(center,radius,points,n))return true;
    n=ObstaclesColumnPolygon(c,false,points,OBSTACLE_POLYGON_CAPACITY);
    return CollisionCirclePolygon(center,radius,points,n);
}
static bool CircleHit(GameVec2 a,float ar,GameVec2 b,float br)
{double x=(double)a.x-b.x,y=(double)a.y-b.y,r=(double)ar+br;return x*x+y*y<r*r;}
static bool HazardHit(const ObstacleHazard *h,PlayerView p)
{
    return CollisionSweptCircle(h->previous,(GameVec2){h->x,h->y},h->radius,p.position,p.collision_radius);
}
static void Spawn(ObstaclesState *s,float travel)
{
    if(s->column_count==OBSTACLE_CAPACITY){s->capacity_delays++;return;}
    const WorldConfig *cfg=&ObstacleWorldConfigs[s->world];unsigned index=s->generated;
    unsigned pattern=(index/5)%4;
    float target=pattern==0?220+(index%5)*23:pattern==1?260+sinf(index*.7f)*65:pattern==2?260+(index%2?45:-45):260;
    float max_step=fminf(44,cfg->gap-cfg->motion*2-80),center=260;
    if(s->column_count){float previous=s->columns[s->column_count-1].base_y;center=Clamp(target,previous-max_step,previous+max_step);}
    ObstacleColumn c={0};c.world=s->world;c.id=++s->generated;c.x=MORPH_WIDTH+50-travel;
    c.width=s->world==4?65+Random(s)*19:72;c.base_y=c.gap_y=center;c.gap=cfg->gap;
    c.phase=index*.6f;c.motion=pattern==3?cfg->motion*.6f:cfg->motion;
    s->columns[s->column_count++]=c;
}
static void BeginEnvironment(ObstaclesState *s)
{
    const WorldConfig *cfg=&ObstacleWorldConfigs[s->world];
    s->environment=(ObstacleEnvironmentView){true,true,0,sin(s->clock)>0?-1:1,0,fminf(170,cfg->event_interval-100)};
}
static bool SpawnHazard(ObstaclesState *s,PlayerView player,const FrameContext *frame)
{
    const ObstacleHazardConfig *cfg=&hazard_configs[s->world];
    if(s->hazard_count>=cfg->max_active)return false;
    float speed=cfg->speed*(.96f+.08f*Random(s));
    float x=MORPH_WIDTH+20,arrival=(x-player.position.x)/speed;
    float column_speed=frame->world_steps>0?frame->scroll_delta/frame->world_steps:ObstacleWorldConfigs[s->world].base_speed;
    float lo=100,hi=435;
    /* Intersect full vertical envelopes of columns expected during the crossing.
       Generation can defer; a launched projectile is never culled to make space. */
    for(unsigned i=0;i<s->column_count;i++){
        const ObstacleColumn *c=&s->columns[i];float predicted=c->x-column_speed*arrival;
        if(predicted<player.position.x+100 && predicted+c->width>player.position.x-100){
            lo=fmaxf(lo,c->base_y-c->gap/2+c->motion);
            hi=fminf(hi,c->base_y+c->gap/2-c->motion);
        }
    }
    float radius=15,pad=radius+cfg->amplitude+4;
    if(hi-lo<2*radius+2*cfg->amplitude+56)return false;
    bool upper=player.position.y<(lo+hi)*.5f;
    if(s->next_hazard%3==2)upper=!upper;
    float y=upper?Clamp(player.position.y,lo+pad,hi-52-radius-cfg->amplitude):
        Clamp(player.position.y,lo+52+radius+cfg->amplitude,hi-pad);
    for(size_t i=0;i<s->protected_count;i++){
        if(CircleHit((GameVec2){x,y},radius+cfg->amplitude+8,s->protected_items[i].center,s->protected_items[i].radius))return false;
    }
    static const ObstacleHazardType types[]={HAZARD_WIND,HAZARD_JELLY,HAZARD_VINE,HAZARD_ICE,HAZARD_ROCK,HAZARD_FIRE};
    ObstacleHazard h={0};h.type=types[s->world];h.id=++s->next_hazard;
    h.x=x;h.base_y=y;h.phase=Random(s)*6.2831853f;h.speed=speed;h.radius=radius;
    h.y=y+cfg->amplitude*sinf(h.phase);h.previous=(GameVec2){h.x,h.y};
    s->hazards[s->hazard_count++]=h;return true;
}
static void Step(void *context,const GameState *game,const FrameContext *frame,PlayerView player,bool spawn)
{
    ObstaclesState *s=context;
    if(game->mode!=GAME_PLAY && game->mode!=GAME_PORTAL)return;
    if(!isfinite(frame->world_steps) || frame->world_steps<0 || !isfinite(frame->scroll_delta) || frame->scroll_delta<0)return;
    s->mode=game->mode;
    float dt=frame->world_steps,travel=frame->scroll_delta;
    const WorldConfig *cfg=Config(s,s->world);if(!cfg)return;
    if(game->mode==GAME_PORTAL){s->environment=(ObstacleEnvironmentView){0};s->hazard_count=0;spawn=false;}
    else s->clock=game->world_steps; /* authoritative core clock; pace comes from FrameContext */
    unsigned kept=0;
    for(unsigned i=0;i<s->column_count;i++) {
        ObstacleColumn c=s->columns[i];c.x-=travel;
        if(game->mode==GAME_PLAY)c.gap_y=c.base_y+(float)sin(s->clock*cfg->angular_frequency+c.phase)*c.motion*Clamp((MORPH_WIDTH-c.x)/160,0,1);
        if(!c.passed && c.x+c.width<player.position.x){c.passed=true;s->passed++;}
        if(c.x>-100)s->columns[kept++]=c;
    }
    s->column_count=kept;
    if(game->mode==GAME_PORTAL)return;
    s->spawn_timer-=dt;
    if(spawn && s->spawn_timer<=0) {
        Spawn(s,travel);
        float pace=isfinite(frame->pace)?Clamp(frame->pace,.8f,1.3f):1;
        s->spawn_timer=cfg->obstacle_interval/(.98f+(pace-1)*.55f);
    }
    s->event_timer-=dt;
    if(spawn && s->event_timer<=0 && !s->environment.active){BeginEnvironment(s);s->event_timer=cfg->event_interval;}
    if(s->environment.active) {
        s->environment.age+=dt;s->environment.warning=s->environment.age<85;
        s->environment.intensity=s->environment.warning?0:sinf(Clamp((s->environment.age-85)/s->environment.duration,0,1)*3.14159265359f);
        if(s->environment.age>85+s->environment.duration)s->environment=(ObstacleEnvironmentView){0};
    }
    s->hazard_timer-=dt;
    if(spawn && s->hazard_timer<=0)s->hazard_timer=SpawnHazard(s,player,frame)?hazard_configs[s->world].interval:20;
    kept=0;
    for(unsigned i=0;i<s->hazard_count;i++) {
        ObstacleHazard h=s->hazards[i];h.previous=(GameVec2){h.x,h.y};h.age+=dt;
        const ObstacleHazardConfig *hc=&hazard_configs[s->world];
        h.x-=h.speed*dt;h.y=h.base_y+hc->amplitude*sinf(h.age*hc->frequency+h.phase);
        if(h.x+h.radius>=0)s->hazards[kept++]=h;
    }
    s->hazard_count=kept;
}
static float Force(void *context)
{
    const ObstaclesState *s=context;
    if(s->mode!=GAME_PLAY || !s->environment.active || s->environment.warning)return 0;
    return ObstacleWorldConfigs[s->world].vertical_force*s->environment.intensity*s->environment.direction;
}
static bool Touches(void *context,PlayerView player)
{
    const ObstaclesState *s=context;if(s->mode!=GAME_PLAY)return false;
    for(unsigned i=0;i<s->column_count;i++)if(ColumnHit(&s->columns[i],player.position,player.collision_radius))return true;
    for(unsigned i=0;i<s->hazard_count;i++)if(s->hazards[i].age>=hazard_configs[s->world].warning && HazardHit(&s->hazards[i],player))return true;
    return false;
}
static bool Clear(void *context,PlayerView player)
{
    const ObstaclesState *s=context;
    for(unsigned i=0;i<s->column_count;i++)if(s->columns[i].x+s->columns[i].width>=player.position.x-player.collision_radius)return false;
    for(unsigned i=0;i<s->hazard_count;i++)if(s->hazards[i].x+s->hazards[i].radius>=player.position.x-player.collision_radius)return false;
    return !s->environment.active;
}
static unsigned Passed(void *context){ObstaclesState *s=context;unsigned passed=s->passed;s->passed=0;return passed;}
static float SafeY(void *context,float x,float fallback)
{
    const ObstaclesState *s=context;float lo=90,hi=450,target=Clamp(fallback,90,450),nearest=INFINITY;
    for(unsigned i=0;i<s->column_count;i++) {
        const ObstacleColumn *c=&s->columns[i];float distance=fabsf(c->x+c->width/2-x);
        if(distance<140) {
            lo=fmaxf(lo,c->base_y+c->motion-c->gap/2+30);hi=fminf(hi,c->base_y-c->motion+c->gap/2-30);
            if(distance<nearest){target=c->base_y;nearest=distance;}
        }
    }
    return lo<=hi?Clamp(target,lo,hi):Clamp(target,90,450);
}
static bool IsSafe(void *context,GameVec2 center,float radius)
{
    const ObstaclesState *s=context;
    if(!isfinite(center.x)||!isfinite(center.y)||!isfinite(radius)||radius<=0 || center.y-radius<42 || center.y+radius>487)return false;
    for(unsigned i=0;i<s->column_count;i++) {
        const ObstacleColumn *c=&s->columns[i];
        if(center.x+radius>=c->x && center.x-radius<=c->x+c->width &&
           (center.y-radius<=c->base_y+c->motion-c->gap/2 || center.y+radius>=c->base_y-c->motion+c->gap/2))return false;
        if(ColumnHit(c,center,radius))return false;
    }
    /* This query places items in open terrain. Moving hazards can cross items
       without destroying them; player danger is checked separately by Touches. */
    return true;
}
static PassageQuery Passages(void *context){return (PassageQuery){context,SafeY,IsSafe};}
static void Draw(void *context,const GameState *game){ObstaclesDraw(context,game);}
ObstaclesApi ObstaclesModuleCreate(void)
{return (ObstaclesApi){{&state,Init,Reset,Shutdown},Config,Step,Force,Touches,Clear,Passed,Passages,Draw};}
bool ObstaclesSetProtectedItems(const ObstacleProtectedCircle *circles,size_t count)
{
    if(count>OBSTACLE_PROTECTED_CAPACITY || (count && !circles))return false;
    for(size_t i=0;i<count;i++)if(!isfinite(circles[i].center.x)||!isfinite(circles[i].center.y)||!isfinite(circles[i].radius)||circles[i].radius<=0)return false;
    if(count){memcpy(state.protected_items,circles,count*sizeof(*circles));}
    state.protected_count=count;return true;
}
ObstacleEnvironmentView ObstaclesEnvironment(void){return state.environment;}
ObstaclesSnapshot ObstaclesInspect(void)
{
    ObstaclesSnapshot v={0};v.world=state.world;v.column_count=state.column_count;v.hazard_count=state.hazard_count;
    v.generated=state.generated;v.capacity_delays=state.capacity_delays;v.clock=state.clock;v.environment=state.environment;
    memcpy(v.columns,state.columns,sizeof(v.columns));memcpy(v.hazards,state.hazards,sizeof(v.hazards));return v;
}
