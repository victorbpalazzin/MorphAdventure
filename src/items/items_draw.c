#include "items_internal.h"
#include "raylib.h"
#include <math.h>
static Vector2 Point(GameVec2 p,float x,float y,float scale,float angle)
{return (Vector2){p.x+scale*(x*cosf(angle)-y*sinf(angle)),p.y+scale*(x*sinf(angle)+y*cosf(angle))};}
static void Line(GameVec2 p,float x,float y,float xx,float yy,float width,Color c)
{DrawLineEx((Vector2){p.x+x,p.y+y},(Vector2){p.x+xx,p.y+yy},width,c);}
static void Icon(BoostType type,GameVec2 p,Color c)
{
    if(type==BOOST_SHIELD){
        Vector2 v[]={{p.x-10,p.y-10},{p.x+10,p.y-10},{p.x+8,p.y+5},{p.x,p.y+13},{p.x-8,p.y+5}};
        for(int i=0;i<5;i++){DrawTriangle((Vector2){p.x,p.y},v[(i+1)%5],v[i],Fade(c,.3f));DrawLineEx(v[i],v[(i+1)%5],2,c);}
        Line(p,-5,0,-1,5,2,WHITE);Line(p,-1,5,6,-4,2,WHITE);
    }else if(type==BOOST_MAGNET){
        DrawRing((Vector2){p.x,p.y+1},7,12,0,180,24,c);
        Line(p,-9,1,-9,-10,6,c);Line(p,9,1,9,-10,6,c);
        Line(p,-9,-10,-9,-6,6,WHITE);Line(p,9,-10,9,-6,6,WHITE);
    }else if(type==BOOST_SLOW){
        Line(p,-10,-12,10,-12,3,c);Line(p,-10,12,10,12,3,c);
        Line(p,-8,-10,8,10,2,c);Line(p,8,-10,-8,10,2,c);
        DrawTriangle((Vector2){p.x-5,p.y+9},(Vector2){p.x+5,p.y+9},(Vector2){p.x,p.y+2},c);
    }else{DrawText("x2",(int)p.x-11,(int)p.y-10,20,c);}
}
void ItemsDraw(const ItemsState *s)
{
    for(unsigned i=0;i<s->fragment_count;i++){
        const ItemFragmentView *t=&s->fragments[i];float alpha=t->fade>0?fmaxf(0,1-t->fade/22):1;
        float scale=t->fade>0?1+t->fade/25:1,angle=sinf(t->spin)*.14f;
        Color edge=Fade(GetColor(0xfff4aaff),alpha),fill=Fade(GetColor(0x233d75ff),alpha);
        DrawCircleGradient((int)t->position.x,(int)t->position.y,37*scale,Fade(edge,.2f),Fade(edge,0));
        for(int j=0;j<6;j++){
            float a=j*PI/3,b=(j+1)*PI/3;
            Vector2 v=Point(t->position,cosf(a)*t->radius,sinf(a)*t->radius,scale,angle);
            Vector2 w=Point(t->position,cosf(b)*t->radius,sinf(b)*t->radius,scale,angle);
            DrawTriangle((Vector2){t->position.x,t->position.y},w,v,fill);DrawLineEx(v,w,3*scale,edge);
        }
        const float bolt[][2]={{5,-13},{-6,1},{2,1},{-4,13},{8,-3},{0,-3}};
        for(int j=0;j<5;j++)DrawLineEx(Point(t->position,bolt[j][0],bolt[j][1],scale,angle),Point(t->position,bolt[j+1][0],bolt[j+1][1],scale,angle),3*scale,Fade(WHITE,alpha));
    }
    for(unsigned i=0;i<s->power_count;i++){
        const ItemPowerView *p=&s->powers[i];const ItemBoostDefinition *d=ItemsBoostDefinition(p->type);if(!d)continue;
        Color color=GetColor(d->color);Vector2 center={p->position.x,p->position.y};
        DrawCircleGradient((int)center.x,(int)center.y,38,Fade(color,.22f),Fade(color,0));
        DrawCircleV(center,p->radius+2,GetColor(0x132945ff));DrawRing(center,p->radius+1,p->radius+3,0,360,48,color);
        Icon(p->type,p->position,color);
    }
}
