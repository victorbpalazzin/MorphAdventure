#include "boss.h"
#include "player/collision.h"
#include "items/combat_energy.h"
#include <math.h>
#include <string.h>
GameVec2 BossMouth(const BossState *b)
{return (GameVec2){741,270+5*sinf(b->clock*.023f)};}
void BossBegin(BossState *b,unsigned required)
{
    memset(b,0,sizeof(*b));b->stage=BOSS_INTRO;b->segments=3;b->phase=1;
    b->attack_timer=100;CombatEnergyReset(&b->energy,required);
}
bool BossFire(BossState *b,PlayerView player)
{
    if(b->stage!=BOSS_FIGHT || b->beam_active || b->energy.collected!=b->energy.required)return false;
    b->beam_active=true;b->beam_hit=false;b->beam_age=0;b->beam_origin=player.position;
    b->warning=0;b->burst_remaining=0;b->projectile_count=0;return true;
}
float BossTargetY(const BossState *b,unsigned index)
{
    /* Keep the whole volley below the boss HUD, with distinct evasive lanes. */
    float spread=(b->phase-1)*55.0f;
    float center=fmaxf(185+spread,fminf(420-spread,b->target_y));
    return center+((float)index-(b->phase-1)*.5f)*110;
}
typedef struct BossAttackConfig {float speed,interval,spacing;unsigned count;} BossAttackConfig;
static const BossAttackConfig attacks[]={{3.8f,205,0,1},{4.6f,155,0,2},{5.4f,115,18,3}};
static void SpawnShot(BossState *b,unsigned index)
{
    if(b->projectile_count>=MORPH_BOSS_PROJECTILES)return;
    const BossAttackConfig *cfg=&attacks[b->phase-1];
    GameVec2 origin={BossMouth(b).x,BossTargetY(b,index)};
    float speed=cfg->speed+(b->phase>1?.2f*(float)((b->volley+index)%3):0);
    b->projectiles[b->projectile_count++]=(BossProjectile){.position=origin,.previous=origin,.velocity={-speed,0},.radius=12};
}
static void FireVolley(BossState *b)
{
    const BossAttackConfig *cfg=&attacks[b->phase-1];
    if(cfg->spacing>0){SpawnShot(b,0);b->burst_remaining=cfg->count-1;b->burst_index=1;b->burst_timer=cfg->spacing;}
    else for(unsigned i=0;i<cfg->count;i++)SpawnShot(b,i);
    b->volley++;b->attack_timer=cfg->interval;
}
void BossStep(BossState *b,const FrameContext *f,PlayerView player)
{
    if(b->stage==BOSS_DORMANT || b->stage==BOSS_DONE)return;
    b->clock+=f->normal_steps;b->age+=f->normal_steps;b->hurt=fmaxf(0,b->hurt-f->normal_steps);
    float fraction=CombatEnergyFraction(&b->energy);
    b->battery_display+=(fraction-b->battery_display)*(1-expf(-f->ui_seconds*6));
    if(b->stage==BOSS_INTRO){if(b->age>=180){b->stage=BOSS_FIGHT;b->age=0;}return;}
    if(b->stage==BOSS_DYING){
        if(b->beam_active){b->beam_age+=f->normal_steps;if(b->beam_age>=85){b->beam_active=false;b->energy.collected=0;b->battery_display=0;}}
        if(b->age>=210)b->stage=BOSS_DONE;
        return;
    }
    if(b->beam_active){
        b->beam_age+=f->normal_steps;
        if(b->beam_age>=35 && !b->beam_hit){
            b->beam_hit=true;b->segments--;b->hurt=65;
            if(!b->segments){b->stage=BOSS_DYING;b->age=0;b->projectile_count=0;b->warning=0;}
        }
        if(b->beam_age>=85){
            b->beam_active=false;b->phase=4-b->segments;
            unsigned required=b->energy.required;CombatEnergyReset(&b->energy,required);
            b->battery_display=0;b->attack_timer=100;
        }
        return;
    }
    if(b->burst_remaining){
        b->burst_timer-=f->world_steps;
        if(b->burst_timer<=0){SpawnShot(b,b->burst_index++);b->burst_remaining--;b->burst_timer=attacks[b->phase-1].spacing;}
    }
    else if(b->warning>0){b->warning-=f->world_steps;if(b->warning<=0)FireVolley(b);}
    else{
        b->attack_timer-=f->world_steps;
        if(b->attack_timer<=0){b->warning=65;b->target_y=player.position.y;}
    }
    unsigned kept=0;
    for(unsigned i=0;i<b->projectile_count;i++){
        BossProjectile p=b->projectiles[i];p.previous=p.position;
        p.position.x+=p.velocity.x*f->world_steps;p.position.y+=p.velocity.y*f->world_steps;
        if(p.position.x+p.radius>=0)b->projectiles[kept++]=p;
    }
    b->projectile_count=kept;
}
bool BossTouches(const BossState *b,PlayerView player)
{
    if(b->stage!=BOSS_FIGHT || b->beam_active)return false;
    for(unsigned i=0;i<b->projectile_count;i++){
        const BossProjectile *p=&b->projectiles[i];
        if(!p->spent && CollisionSweptCircle(p->previous,p->position,p->radius,player.position,player.collision_radius))return true;
    }
    return false;
}

void BossRegisterContact(BossState *b,PlayerView player)
{
    for(unsigned i=0;i<b->projectile_count;i++){
        BossProjectile *p=&b->projectiles[i];
        if(!p->spent && CollisionSweptCircle(p->previous,p->position,p->radius,player.position,player.collision_radius)){p->spent=true;return;}
    }
}
