#include "obstacles_internal.h"
#include "worlds/worlds.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static float Cross(Vector2 a,Vector2 b,Vector2 c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
static void Triangle(Vector2 a,Vector2 b,Vector2 c,Color color)
{if(Cross(a,b,c)>0)DrawTriangle(a,c,b,color);else DrawTriangle(a,b,c,color);}
static void Poly(const GameVec2 *points,size_t count,Color color)
{
    int ids[OBSTACLE_POLYGON_CAPACITY];float area=0;
    if(count<3 || count>OBSTACLE_POLYGON_CAPACITY)return;
    for(size_t i=0;i<count;i++){ids[i]=(int)i;area+=points[i].x*points[(i+1)%count].y-points[(i+1)%count].x*points[i].y;}
    float sign=area>=0?1.0f:-1.0f;int left=(int)count;
    while(left>2) {
        bool clipped=false;
        for(int k=0;k<left;k++) {
            int prev=(k+left-1)%left,next=(k+1)%left;
            GameVec2 ga=points[ids[prev]],gb=points[ids[k]],gc=points[ids[next]];
            Vector2 a={ga.x,ga.y},b={gb.x,gb.y},c={gc.x,gc.y};
            if(Cross(a,b,c)*sign<=0)continue;
            bool occupied=false;
            for(int j=0;j<left;j++)if(j!=prev&&j!=k&&j!=next) {
                Vector2 q={points[ids[j]].x,points[ids[j]].y};
                if(Cross(a,b,q)*sign>=0&&Cross(b,c,q)*sign>=0&&Cross(c,a,q)*sign>=0)occupied=true;
            }
            if(occupied)continue;
            Triangle(a,b,c,color);
            for(int j=k;j+1<left;j++)ids[j]=ids[j+1];
            left--;clipped=true;break;
        }
        if(!clipped)break;
    }
}
static Vector2 Local(const ObstacleColumn *c,bool top,float x,float y)
{return (Vector2){c->x+x,top?y:MORPH_HEIGHT-y};}
static void Segment(const ObstacleColumn *c,bool top,float x1,float y1,float x2,float y2,float width,Color color)
{DrawLineEx(Local(c,top,x1,y1),Local(c,top,x2,y2),width,color);}
static void Oval(const ObstacleColumn *c,bool top,float x,float y,float rx,float ry,Color color)
{
    Vector2 center=Local(c,top,x,y),prev={center.x+rx,center.y};
    for(int i=1;i<=32;i++){float a=i*2*PI/32;Vector2 next={center.x+cosf(a)*rx,center.y+sinf(a)*ry};Triangle(center,prev,next,color);prev=next;}
}
static void Column(const ObstaclesState *s,const ObstacleColumn *c,bool top)
{
    static const unsigned colors[]={0x52617fff,0x286e7cff,0x655e41ff,0x86bcd4ff,0xb17c58ff,0x493142ff};
    const WorldPalette *palette=WorldsPalette(s->world);
    GameVec2 points[OBSTACLE_POLYGON_CAPACITY];size_t count=ObstaclesColumnPolygon(c,top,points,OBSTACLE_POLYGON_CAPACITY);
    Poly(points,count,GetColor(colors[s->world]));
    float h=top?c->gap_y-c->gap/2:MORPH_HEIGHT-c->gap_y-c->gap/2;
    /* Decoration stays >=12 px inside both jagged sides and away from the tip. */
    for(float y=22;y<h-30;y+=32) {
        if(s->world==0) {
            Oval(c,top,c->width*.5f,y,c->width*.5f-13,12,GetColor(0x71829b88));
            Oval(c,top,c->width*.42f,y-6,12,10,GetColor(0x91a1b044));
        } else if(s->world==1) {
            Oval(c,top,c->width*.5f,y,c->width*.5f-14,15,GetColor(0x4a90937f));
            Segment(c,top,18,y-4,c->width-20,y+3,2,GetColor(0x83c5be88));
        } else if(s->world==2) {
            for(int j=0;j<3;j++)Segment(c,top,16+j*15,y-14,19+j*15,y+12,2,GetColor(0xb09d6388));
            Oval(c,top,c->width*.4f,y,7,3,GetColor(0x293c31ff));
        } else if(s->world==3) {
            Vector2 a=Local(c,top,14,y-12),b=Local(c,top,c->width-15,y-12),d=Local(c,top,c->width*.55f,y+13);
            Triangle(a,b,d,GetColor(0xe9ffff66));
            Segment(c,top,15,y-9,c->width*.55f,y+13,1,GetColor(0xf0ffffbb));
        } else if(s->world==4) {
            Segment(c,top,13,y,c->width*.4f,y+5,3,GetColor(0xf1bb8499));
            Segment(c,top,c->width*.4f,y+5,c->width-13,y-3,3,GetColor(0xf1bb8499));
        } else {
            Segment(c,top,20,y-10,31,y+2,2,GetColor(0xffac62ff));
            Segment(c,top,31,y+2,23,y+12,2,GetColor(0xff835bcc));
            Segment(c,top,31,y+2,c->width-17,y+6,1,GetColor(0xff835b77));
        }
    }
    if(s->world==0 && h>70) {
        Segment(c,top,42,h-67,28,h-45,3,GetColor(0xd0d7ffff));
        Segment(c,top,28,h-45,42,h-45,3,GetColor(0xd0d7ffff));
        Segment(c,top,42,h-45,32,h-24,3,GetColor(0xd0d7ffff));
    }
    if(s->world==1 && h>60)for(int i=-2;i<=2;i++) {
        Segment(c,top,c->width*.5f,h-23,c->width*.5f+i*6,h-47+abs(i)*5,3,GetColor(0xe9a49dff));
    }
    if(s->world==2) {
        for(float y=25;y<h-30;y+=15)Segment(c,top,c->width-17+3*sinf(y*.08f),y,c->width-17+3*sinf((y+15)*.08f),y+15,3,GetColor(0xa7bd6bff));
        Oval(c,top,c->width*.5f,h-23,13,5,GetColor(0x8ba862ff));
    }
    for(size_t i=0;i<count;i++) {
        Vector2 a={points[i].x,points[i].y},b={points[(i+1)%count].x,points[(i+1)%count].y};
        DrawLineEx(a,b,s->debug?1:2,s->debug?LIME:GetColor(palette->ink));
    }
    Segment(c,top,c->width*.34f,h-2,c->width*.78f,h-5,2,GetColor(palette->accent));
}
static void Hazard(const ObstacleHazard *h,const WorldPalette *palette,double clock,bool debug)
{
    Vector2 center={h->x,h->y};bool warning=h->age<85;float alpha=warning?.4f:1;
    if(warning) {
        Color color=Fade(GetColor(palette->accent),.55f+.2f*(float)sin(clock*.15));
        for(int i=0;i<8;i++)DrawRing(center,29,30,i*45.0f,i*45.0f+23,6,color);
        DrawText("!",(int)h->x-3,(int)h->y-47,20,color);
        if(h->type==HAZARD_ICE)DrawLineEx((Vector2){h->x,h->y-20},(Vector2){h->x,h->y+40},1,Fade(color,.45f));
    }
    if(h->type==HAZARD_ICE) {
        GameVec2 points[4];ObstaclesIcePolygon(h,points,4);Poly(points,4,Fade(GetColor(0xc2f1ffff),alpha));
        DrawLineEx((Vector2){h->x,h->y-h->radius},(Vector2){h->x,h->y+h->radius},1,Fade(WHITE,alpha));
        if(debug)for(int i=0;i<4;i++)DrawLineV((Vector2){points[i].x,points[i].y},(Vector2){points[(i+1)%4].x,points[(i+1)%4].y},LIME);
    } else {
        Color color=h->type==HAZARD_JELLY?GetColor(0xbcb6eeff):GetColor(0xffb451ff);
        DrawCircleGradient((int)h->x,(int)h->y,30,Fade(color,.18f*alpha),Fade(color,0));
        /* Circular visible core is the circular collider; halo is decoration. */
        DrawCircleV(center,h->radius,Fade(color,alpha));
        if(h->type==HAZARD_JELLY) {
            DrawCircleV((Vector2){h->x-3,h->y-4},7,Fade(GetColor(0xf2dbfaff),alpha));
            for(int i=-1;i<=1;i++)DrawLineEx((Vector2){h->x+i*6,h->y+3},(Vector2){h->x+i*5,h->y+11},2,Fade(GetColor(0x7b65aaff),alpha));
        } else {
            Triangle((Vector2){h->x-8,h->y+7},(Vector2){h->x+2,h->y-11},(Vector2){h->x+9,h->y+6},Fade(GetColor(0xffed9dff),alpha));
        }
        if(debug)DrawCircleLinesV(center,h->radius,LIME);
    }
}
void ObstaclesDraw(const ObstaclesState *s,const GameState *game)
{
    (void)game;const WorldPalette *p=WorldsPalette(s->world);if(!p)return;
    for(unsigned i=0;i<s->column_count;i++) {
        const ObstacleColumn *c=&s->columns[i];Column(s,c,true);Column(s,c,false);
        if(c->x>400 && c->motion>0) {
            Vector2 center={c->x+c->width/2,c->gap_y-c->gap/2+22};
            DrawLineEx((Vector2){center.x,center.y-6},(Vector2){center.x,center.y+6},2,Fade(GetColor(p->accent),.7f));
            Triangle((Vector2){center.x-3,center.y-3},(Vector2){center.x,center.y-7},(Vector2){center.x+3,center.y-3},GetColor(p->accent));
        }
    }
    for(unsigned i=0;i<s->hazard_count;i++)Hazard(&s->hazards[i],p,s->clock,s->debug);
    if(s->environment.active) {
        const char *names[]={"Rajada de vento","Corrente maritima","Galhos em movimento","Nevasca e gelo em queda","Redemoinho de areia","Erupcao vulcanica"};
        DrawRectangleRounded((Rectangle){302,461,356,30},.3f,6,GetColor(0x10283de0));
        char label[96];snprintf(label,sizeof(label),"%s%s",s->environment.warning?"Atencao: ":"",names[s->world]);
        DrawText(label,480-MeasureText(label,14)/2,469,14,GetColor(p->accent));
        if(s->world==0||s->world==1||s->world==3||s->world==4) {
            float opacity=s->environment.warning?.10f:.18f;
            for(int i=0;i<7;i++) {
                float x=(float)fmod(i*150+s->clock*.8,1100)-40,y=150+i%3*95,bend=s->environment.direction*15;
                DrawSplineSegmentBezierCubic((Vector2){x,y},(Vector2){x+30,y-bend},(Vector2){x+50,y+bend},(Vector2){x+80,y-bend},2,Fade(GetColor(p->accent),opacity));
            }
        }
    }
}
