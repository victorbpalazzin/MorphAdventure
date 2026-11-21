#include "effects_internal.h"
#include "boss_visual.h"
#include "items/items.h"
#include <math.h>
#include <string.h>
static EffectsState state;
static bool Running(GameMode m){return m==GAME_PLAY || m==GAME_PORTAL || m==GAME_TRANSITION || m==GAME_BOSS;}
static float Random(EffectsState *s){s->seed=s->seed*1664525u+1013904223u;return (float)((s->seed>>8)/16777216.0);}
static bool Init(void *p){memset(p,0,sizeof(EffectsState));return true;}
static void Shutdown(void *p){memset(p,0,sizeof(EffectsState));}
static void Reset(void *p,const GameState *g,const WorldConfig *cfg)
{(void)cfg;EffectsState *s=p;memset(s,0,sizeof(*s));s->world=g->world;s->seed=606u+(unsigned)g->world;}
static void Burst(EffectsState *s,GameVec2 at,uint32_t color,unsigned count)
{
    for(unsigned i=0;i<count && s->count<EFFECTS_PARTICLE_CAPACITY;i++){
        float angle=Random(s)*6.2831853f,speed=.8f+Random(s)*3.2f;
        s->particles[s->count++]=(EffectParticle){at,{cosf(angle)*speed,sinf(angle)*speed},1,2+Random(s)*3,color};
    }
}
static void Event(void *p,const GameEvent *e)
{
    EffectsState *s=p;if(!e || !isfinite(e->position.x)||!isfinite(e->position.y))return;
    switch(e->type){
        case EVENT_FRAGMENT_COLLECTED:Burst(s,e->position,0xfff4a5ff,20);break;
        case EVENT_BOOST_COLLECTED:{const ItemBoostDefinition *d=ItemsBoostDefinition(e->boost);if(d)Burst(s,e->position,d->color,30);break;}
        case EVENT_DAMAGE:s->damage_flash=22;Burst(s,e->position,0xff8f97ff,16);break;
        case EVENT_SHIELD_ABSORBED:Burst(s,e->position,0x8ddfffff,30);break;
        case EVENT_PORTAL_CHARGED:s->charge_flash=35;Burst(s,e->position,0x9bffe0ff,40);break;
        case EVENT_PORTAL_ENTERED:Burst(s,e->position,0xe6ffffff,40);break;
        case EVENT_OBSTACLE_PASSED:break;
    }
}
static void Particles(void *p,const GameState *g,const FrameContext *f)
{
    EffectsState *s=p;if(!Running(g->mode)||!isfinite(f->normal_steps)||f->normal_steps<=0)return;
    float dt=f->normal_steps;s->clock+=dt;s->damage_flash=fmaxf(0,s->damage_flash-dt);s->charge_flash=fmaxf(0,s->charge_flash-dt);
    unsigned kept=0;for(unsigned i=0;i<s->count;i++){
        EffectParticle v=s->particles[i];v.position.x+=v.velocity.x*dt;v.position.y+=v.velocity.y*dt;v.life-=dt/35;
        if(v.life>0)s->particles[kept++]=v;
    }s->count=kept;
}
static void Begin(void *p,const GameState *g,PlayerView player)
{
    EffectsState *s=p;if(g->mode!=GAME_PORTAL || s->phase!=FX_IDLE)return;
    s->world=g->world;s->phase=FX_GROW;s->age=s->entry=s->transition=0;
    s->pose=(PortalPose){true,player.position,1,0};s->blackout_sent=s->done_sent=false;
}
static PortalSignal Portal(void *p,const GameState *g,const FrameContext *f)
{
    EffectsState *s=p;float dt=f->normal_steps;
    if(!isfinite(dt)||dt<=0 || (g->mode!=GAME_PORTAL && g->mode!=GAME_TRANSITION))return PORTAL_SIGNAL_NONE;
    if(g->mode==GAME_PORTAL && (s->phase==FX_GROW || s->phase==FX_ENTER)){
        s->age+=dt;
        if(s->age<65){s->pose.position.y+=(270-s->pose.position.y)*fminf(1,.025f*dt);}
        else{
            if(s->phase==FX_GROW){s->start=s->pose.position;s->phase=FX_ENTER;}
            s->entry+=dt;float t=fminf(1,s->entry/85),ease=t*t*(3-2*t);
            s->pose.position=(GameVec2){s->start.x+(870-s->start.x)*ease,s->start.y+(270-s->start.y)*ease};
            s->pose.scale=1-.92f*t;s->pose.rotation=0;
            if(t>=1){s->phase=FX_BLACK;s->transition=0;return PORTAL_SIGNAL_ENTERED;}
        }
    }else if(g->mode==GAME_TRANSITION && (s->phase==FX_BLACK || s->phase==FX_REVEAL)){
        s->transition+=dt;
        if(!s->blackout_sent && s->transition>=24){s->blackout_sent=true;return PORTAL_SIGNAL_BLACKOUT;}
        if(s->phase==FX_REVEAL && !s->done_sent && s->transition>=55){s->done_sent=true;s->phase=FX_IDLE;return PORTAL_SIGNAL_FADE_DONE;}
    }
    return PORTAL_SIGNAL_NONE;
}
static PortalPose Pose(void *p){return ((EffectsState *)p)->pose;}
static void Switch(void *p,const GameState *g,const WorldConfig *cfg)
{
    (void)cfg;EffectsState *s=p;if(!s->blackout_sent || s->phase!=FX_BLACK)return;
    s->world=g->world;s->count=0;s->damage_flash=s->charge_flash=0;s->pose=(PortalPose){0};s->phase=FX_REVEAL;
}
static void DrawPortal(void *p,const GameState *g){if(g->boss.stage!=BOSS_DORMANT) BossDrawScene(&g->boss); else EffectsDrawPortal(p);}
static void DrawParticles(void *p,const GameState *g){(void)g;EffectsDrawParticles(p);}
static void DrawTransition(void *p,const GameState *g){EffectsDrawTransition(p,g);}
EffectsApi EffectsModuleCreate(void)
{return (EffectsApi){{&state,Init,Reset,Shutdown},Event,Particles,Begin,Portal,Pose,Switch,DrawPortal,DrawParticles,DrawTransition};}
EffectsSnapshot EffectsInspect(void)
{return (EffectsSnapshot){state.world,state.phase,state.count,state.age,state.entry,state.transition,state.damage_flash,state.charge_flash,state.clock,state.pose};}
