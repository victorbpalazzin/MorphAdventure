#include "combat_energy.h"
#include <math.h>
#include <string.h>
void CombatEnergyReset(CombatEnergy *e,unsigned required)
{
    memset(e,0,sizeof(*e));e->required=required?required:MORPH_COMBAT_ENERGY_DEFAULT;
    e->spawn_timer=70;
}
float CombatEnergyFraction(const CombatEnergy *e)
{return e->required?fminf(1,(float)e->collected/e->required):0;}
bool CombatEnergyStep(CombatEnergy *e,const FrameContext *f,PlayerView p,bool magnet)
{
    if(e->collected>=e->required)return false;
    if(!e->present){
        e->spawn_timer-=f->world_steps;
        if(e->spawn_timer<=0){
            static const float heights[]={265,205,305,245,325,225};
            e->position=(GameVec2){590,heights[e->serial++%6]};e->present=true;
        }
    }
    if(!e->present)return false;
    e->position.x-=2.8f*f->world_steps;
    float dx=p.position.x-e->position.x,dy=p.position.y-e->position.y,distance=hypotf(dx,dy);
    if(magnet && distance>0 && distance<210){
        float move=fminf(distance,8*f->world_steps);
        e->position.x+=dx/distance*move;e->position.y+=dy/distance*move;
    }
    if(hypotf(p.position.x-e->position.x,p.position.y-e->position.y)<p.visual_radius+16){
        e->present=false;e->collected++;e->spawn_timer=65;return true;
    }
    if(e->position.x<-30){e->present=false;e->spawn_timer=25;}
    return false;
}
