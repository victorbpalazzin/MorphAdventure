#include "effects_internal.h"
#include "worlds/worlds.h"
#include "raylib.h"
#include <math.h>
static Vector2 Orbit(float r,float angle){return (Vector2){870+cosf(angle)*r*.72f,270+sinf(angle)*r};}
static void Triangle(Vector2 a,Vector2 b,Vector2 c,Color color)
{if((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)>0)DrawTriangle(a,c,b,color);else DrawTriangle(a,b,c,color);}
static void Ellipse(float radius,Color color)
{
    Vector2 previous=Orbit(radius,0);for(int i=1;i<=96;i++){
        Vector2 next=Orbit(radius,i*2*PI/96);DrawTriangle((Vector2){870,270},next,previous,color);previous=next;
    }
}
static void EllipseLine(float radius,float width,Color color)
{for(int i=0;i<96;i++)DrawLineEx(Orbit(radius,i*2*PI/96),Orbit(radius,(i+1)*2*PI/96),width,color);}
void EffectsDrawPortal(const EffectsState *s)
{
    if(s->phase!=FX_GROW && s->phase!=FX_ENTER && s->phase!=FX_BLACK)return;
    const WorldPalette *p=WorldsPalette(s->world);if(!p)return;
    float grow=fminf(1,s->age/65),r=98*grow,angle=(float)s->clock*.045f;
    if(r<=0)return;
    Color accent=GetColor(p->accent);
    for(int i=14;i>0;i--)Ellipse(r*(1+i*.025f),Fade(accent,.025f));
    Ellipse(r,GetColor(p->ink));Ellipse(r*.88f,GetColor(p->layers[1]));
    for(int i=10;i>0;i--)Ellipse(r*i*.045f,Fade(GetColor(0xe8fffffa),.05f));
    EllipseLine(r,3,accent);EllipseLine(r*.94f,1,GetColor(0xd7fff9ff));
    Vector2 previous=Orbit(0,angle);
    for(int i=1;i<150;i++){Vector2 next=Orbit(r*i/150*.8f,i*.13f+angle);DrawLineEx(previous,next,1.5f,GetColor(0xd7fff9cc));previous=next;}
    for(int i=0;i<22;i++)DrawCircleV(Orbit(r*(1.04f+.08f*sinf(angle+i)),i*2*PI/22+angle),(2+i%3)*grow,i%2?GetColor(0xfff4a8ff):GetColor(0x96ffebff));
    float outer=r*1.06f;
    if(s->world==0){
        for(int j=0;j<2;j++){Vector2 at=Orbit(outer,j?-.85f:2.35f);for(int i=-2;i<=2;i++)DrawCircleV((Vector2){at.x+i*9*grow,at.y-3*grow*(float)(i%2)},(9+(i+2)%3)*grow,GetColor(0xe3f3eeff));}
    }else if(s->world==1){
        for(int i=0;i<12;i++)DrawCircleLinesV(Orbit(outer,i*PI/6+angle/3),(4+i%3)*grow,GetColor(0x97f9e8bb));
    }else if(s->world==2){
        EllipseLine(outer,10*grow,GetColor(0x445642ff));EllipseLine(outer,3*grow,GetColor(0x92b66bff));
        for(int i=0;i<8;i++){Vector2 at=Orbit(outer,i*2*PI/8);DrawEllipse((int)at.x,(int)at.y,10*grow,5*grow,GetColor(0x9ab777ff));}
    }else if(s->world==3){
        for(int i=0;i<10;i++){float a=i*PI/5;Vector2 at=Orbit(outer,a),tip={at.x+cosf(a)*15*grow,at.y+sinf(a)*15*grow};
            Vector2 left={at.x-sinf(a)*6*grow,at.y+cosf(a)*6*grow},right={at.x+sinf(a)*6*grow,at.y-cosf(a)*6*grow};
            Triangle(tip,left,right,GetColor(0xa8dceaff));Triangle(right,left,(Vector2){at.x-cosf(a)*9*grow,at.y-sinf(a)*9*grow},GetColor(0x659dbaff));}
    }else if(s->world==4){
        for(int i=0;i<12;i++){float a=i*PI/6;Vector2 at=Orbit(outer,a);DrawRectanglePro((Rectangle){at.x,at.y,19*grow,24*grow},(Vector2){9.5f*grow,12*grow},a*180/PI,GetColor(0xc19771ff));DrawCircleV(at,2*grow,GetColor(0x6e5150ff));}
    }else{
        for(int i=0;i<12;i++){Vector2 at=Orbit(outer,i*PI/6);DrawCircleGradient((int)at.x,(int)at.y,17*grow,Fade(accent,.7f),Fade(accent,0));DrawCircleV(at,3*grow,GetColor(0xffe3a0ff));}
    }
}
void EffectsDrawParticles(const EffectsState *s)
{
    for(unsigned i=0;i<s->count;i++){const EffectParticle *p=&s->particles[i];DrawCircleV((Vector2){p->position.x,p->position.y},p->radius*p->life,Fade(GetColor(p->color),p->life));}
    /* Short translucent feedback keeps the passage visible. */
    if(s->damage_flash>0)DrawRectangle(0,0,960,540,Fade(GetColor(0xff3755ff),s->damage_flash/100));
    if(s->charge_flash>0)DrawRectangle(0,0,960,540,Fade(GetColor(0xb4ffdcff),s->charge_flash/240));
}
void EffectsDrawTransition(const EffectsState *s,const GameState *g)
{
    if(g->mode==GAME_WON || (s->phase!=FX_BLACK && s->phase!=FX_REVEAL))return;
    float alpha=s->transition<24?s->transition/24:1-(s->transition-24)/31;
    alpha=fmaxf(0,fminf(1,alpha));
    /* The clock remains frozen; soften only the overlay so pause controls stay readable. */
    if(g->mode==GAME_PAUSED)alpha*=.2f;
    DrawRectangle(0,0,960,540,Fade(GetColor(0xe6ffffff),alpha));
}
