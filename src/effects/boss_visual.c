#include "boss_visual.h"
#include "boss/boss.h"
#include "raylib.h"
#include <math.h>
static const Color wood={62,43,56,255},bark={114,77,83,255},violet={188,100,255,255};
static Vector2 At(float x,float y,float ox,float sway){return (Vector2){x+ox+sway*(520-y)/400,y};}
static void Limb(Vector2 a,Vector2 b,Vector2 c,float width,Color color)
{
    Vector2 prev=a;
    for(int i=1;i<=14;i++){
        float t=i/14.0f,u=1-t;Vector2 p={u*u*a.x+2*u*t*b.x+t*t*c.x,u*u*a.y+2*u*t*b.y+t*t*c.y};
        float w=width*(1-.82f*t);DrawLineEx(prev,p,w,color);DrawCircleV(p,w*.48f,color);prev=p;
    }
}
static void Tri(Vector2 a,Vector2 b,Vector2 c,Color color)
{
    if((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)>0)DrawTriangle(c,b,a,color);
    else DrawTriangle(a,b,c,color);
}
static void DrawScene(const BossState *b,bool portrait,bool defeated)
{
    float intro=b->stage==BOSS_INTRO?fminf(1,b->age/150):1;
    float death=b->stage==BOSS_DYING?fminf(1,b->age/210):b->stage==BOSS_DONE?1:0;
    float alpha=intro*(1-death)*(defeated?.62f:1),ox=(1-intro)*240-30,sway=defeated?0:5*sinf(b->clock*.023f);
    if(!portrait)DrawRectangle(0,80,960,460,Fade((Color){15,5,31,255},.61f*intro*(1-death)));
    for(int i=0;i<38+(int)b->phase*9;i++){
        float a=i*2.39996f+b->clock*.004f,r=65+(i%11)*13;
        float x=780+ox+cosf(a)*r,y=305+sinf(a)*r*1.15f;
        Color c=death>.3f?ColorFromHSV(i*37.0f, .55f,1):violet;
        DrawCircleGradient((int)x,(int)y,20+(i%7)*2,Fade(c,.09f*intro*(defeated?.12f:1)),BLANK);
        DrawCircle((int)x,(int)y,1.5f+(i%3),Fade(c,.5f*intro*(defeated?.12f:1)));
    }
    // Three faint spirits echo the supplied spectral forest reference.
    for(int i=0;i<(defeated?0:3);i++){
        float x=635+i*120+ox+sin(b->clock*.014f+i)*15,y=170+(i%2)*210+sinf(b->clock*.02f+i)*17;
        Color c=Fade(violet,.25f*alpha);
        DrawCircle((int)x,(int)y,18,c);Limb((Vector2){x-13,y},(Vector2){x-30,y+55},(Vector2){x+22,y+88},28,c);
        DrawEllipse((int)x-6,(int)y,3,6,Fade(WHITE,.5f*alpha));DrawEllipse((int)x+6,(int)y,3,6,Fade(WHITE,.5f*alpha));
    }
    Color dark=Fade((Color){24,20,31,255},alpha),base=Fade(wood,alpha),light=Fade(bark,alpha);
    // Roots and clawed branches are tapered quadratic curves, never a flat sprite.
    for(int side=-1;side<=1;side+=2){
        for(int i=0;i<5;i++){
            Vector2 a=At(780+side*15,370+i*10,ox,sway);
            Vector2 c=At(780+side*(70+i*26),505-i*5,ox,sway);
            Vector2 mid=At(760+side*(80+i*13),420-i*9,ox,sway);
            Limb(a,mid,c,48-i*5,dark);Limb(a,mid,c,35-i*4,base);
            Limb(a,mid,(Vector2){c.x-3,c.y-3},5,light);
        }
        Vector2 shoulder=At(780+side*27,267,ox,sway),elbow=At(780+side*117,225,ox,sway),hand=At(780+side*145,345,ox,sway);
        Limb(shoulder,elbow,hand,51,dark);Limb(shoulder,elbow,hand,38,base);
        Limb(shoulder,(Vector2){elbow.x+4,elbow.y+4},hand,5,light);
        for(int i=0;i<4;i++)Limb(hand,At(780+side*(175+i*9),350+i*12,ox,sway),At(780+side*(145+i*13),398+i*9,ox,sway),15-i*2,base);
        for(int i=0;i<5;i++){
            Vector2 root=At(780+side*22,220,ox,sway),tip=At(780+side*(45+i*29),105+(i%2)*27,ox,sway);
            Limb(root,At(780+side*(45+i*27),175,ox,sway),tip,34-i*4,dark);
            Limb(root,At(780+side*(43+i*27),173,ox,sway),tip,24-i*3,base);
            Limb(tip,At(tip.x-ox+side*15,tip.y-20,0,0),At(tip.x-ox+side*32,tip.y-38,ox,0),8,light);
            for(int j=0;j<5;j++){
                float x=tip.x+side*(j*7-12),y=tip.y+sin(j*2.0f)*14;
                DrawEllipse((int)x,(int)y,7,3,Fade((Color){99+(unsigned char)(j*12),39,63,255},alpha));
            }
        }
    }
    for(int i=0;i<7;i++){
        float x=737+i*13;Limb(At(x,465,ox,sway),At(x+sin(i*1.9f)*58,330,ox,sway),At(750+i*9,185,ox,sway),44,dark);
        Limb(At(x,460,ox,sway),At(x+sin(i*1.9f)*51,330,ox,sway),At(750+i*9,185,ox,sway),30,base);
        Limb(At(x-7,452,ox,sway),At(x+sin(i*1.9f)*49,326,ox,sway),At(748+i*9,185,ox,sway),4,light);
    }
    float glow=defeated?0:(.65f+.15f*sinf(b->clock*.11f))*(1-death);
    for(int i=0;i<6;i++){
        float x=755+(i%3)*22,y=285+(i/3)*70;
        Vector2 a=At(x,y,ox,sway),mid=At(x+15,y+24,ox,sway),c=At(x-4,y+55,ox,sway);
        DrawLineEx(a,mid,8,Fade(violet,.16f*alpha*b->phase*(defeated?expf(-b->clock/120):1)));DrawLineEx(mid,c,8,Fade(violet,.16f*alpha*b->phase*(defeated?expf(-b->clock/120):1)));
        DrawLineEx(a,mid,2+b->phase*.5f,Fade(violet,glow*alpha));DrawLineEx(mid,c,2,Fade(WHITE,glow*alpha));
    }
    if(!defeated){
    float eye_alpha=alpha*(portrait?(.8f+.2f*sinf(b->clock*.11f)):1);
    Vector2 left=At(744,223,ox,sway),right=At(802,223,ox,sway);
    Tri(left,At(768,233,ox,sway),At(758,239,ox,sway),Fade(violet,eye_alpha));
    Tri(right,At(778,233,ox,sway),At(788,239,ox,sway),Fade(violet,eye_alpha));
    DrawCircleGradient((int)left.x+12,(int)left.y+9,24,Fade(violet,.5f*eye_alpha),BLANK);
    DrawCircleGradient((int)right.x-12,(int)right.y+9,24,Fade(violet,.5f*eye_alpha),BLANK);
    Tri(At(750,226,ox,sway),At(765,234,ox,sway),At(757,235,ox,sway),Fade(WHITE,eye_alpha));
    Tri(At(796,226,ox,sway),At(781,234,ox,sway),At(789,235,ox,sway),Fade(WHITE,eye_alpha));
    }
    Tri(At(744,249,ox,sway),At(771,309,ox,sway),At(804,248,ox,sway),dark);
    Tri(At(753,254,ox,sway),At(771,299,ox,sway),At(795,254,ox,sway),Fade(violet,glow*alpha));
    for(int i=0;i<6;i++){
        float x=748+i*9;Tri(At(x,249,ox,sway),At(x+3,270+i%2*9,ox,sway),At(x+7,251,ox,sway),Fade((Color){224,210,230,255},alpha));
    }
    if(b->hurt>0)DrawCircleGradient(775,280,110,Fade(WHITE,b->hurt/240*alpha),BLANK);
    if(death>0)for(int i=0;i<95;i++){
        float a=i*2.4f,travel=death*(100+i%20*8);
        DrawCircle((int)(780+cosf(a)*travel),(int)(310+sinf(a)*travel),2+i%3,Fade(ColorFromHSV(i*37,.6f,1),sinf(death*3.14159f)));
    }
    if(b->warning>0 || b->burst_remaining){
        GameVec2 m=BossMouth(b);DrawCircleLines((int)m.x,(int)m.y,22+8*sinf(b->clock*.2f),violet);
        for(unsigned i=0;i<b->phase;i++){
            float y=BossTargetY(b,i);
            DrawCircleGradient((int)m.x,(int)y,24,Fade(violet,.5f),BLANK);
            for(int j=0;j<18;j++){
                float t=j/18.0f;DrawCircle((int)(m.x+(180-m.x)*t),(int)y,2,Fade(violet,.32f));
            }
        }
        DrawText("RAJADA ESPECTRAL",590,162,16,violet);
    }
    for(unsigned i=0;i<b->projectile_count;i++){
        BossProjectile p=b->projectiles[i];Vector2 at={p.position.x,p.position.y};
        DrawLineEx(at,(Vector2){at.x-p.velocity.x*8,at.y-p.velocity.y*8},10,Fade(violet,.25f));
        DrawCircleGradient((int)at.x,(int)at.y,24,Fade(violet,.5f),BLANK);
        DrawCircleV(at,p.radius,(Color){117,43,182,255});DrawCircleLinesV(at,p.radius,violet);DrawCircleV(at,4,WHITE);
    }
    if(b->energy.present){
        Vector2 p={b->energy.position.x,b->energy.position.y};
        DrawCircleGradient((int)p.x,(int)p.y,34,Fade(SKYBLUE,.35f),BLANK);
        for(int i=0;i<6;i++){
            float a=i*1.0472f+b->clock*.03f;
            DrawCircle((int)(p.x+cosf(a)*16),(int)(p.y+sinf(a)*16),4,ColorFromHSV(i*60, .7f,1));
        }
        DrawPoly(p,4,13,0,WHITE);DrawPoly(p,4,8,0,SKYBLUE);
    }
    if(b->beam_active){
        Vector2 a={b->beam_origin.x+22,b->beam_origin.y};GameVec2 mouth=BossMouth(b);Vector2 end={mouth.x,mouth.y};
        float charge=fminf(1,b->beam_age/35);
        DrawCircleGradient((int)a.x,(int)a.y,25+charge*28,Fade(WHITE,.7f),BLANK);
        if(b->beam_age>=35){
            float fade=fminf(1,(85-b->beam_age)/20);
            DrawLineEx(a,end,65,Fade(SKYBLUE,.15f*fade));
            for(int i=0;i<7;i++)DrawLineEx((Vector2){a.x,a.y+(i-3)*5},(Vector2){end.x,end.y+(i-3)*5},6,Fade(ColorFromHSV(i*48,.7f,1),fade));
            DrawLineEx(a,end,3,Fade(WHITE,fade));
            for(int i=0;i<20;i++){float t=fmodf(i*.053f+b->clock*.025f,1);DrawCircle((int)(a.x+(end.x-a.x)*t),(int)(a.y+(end.y-a.y)*t+sinf(i+b->clock)*22),3,Fade(WHITE,fade));}
        }
    }
    if(b->stage==BOSS_INTRO){
        DrawRectangleRounded((Rectangle){200,140,560,62},.2f,12,Fade((Color){20,9,34,255},.86f));
        const char *text="GROOTSPECTRO DESPERTOU!";DrawText(text,480-MeasureText(text,29)/2,157,29,(Color){226,189,255,255});
        if(b->age<24)DrawRectangle(0,0,960,540,Fade(WHITE,1-b->age/24));
    }
}

void BossDrawScene(const BossState *boss){DrawScene(boss,false,false);}
void BossDrawPortrait(float seconds,bool defeated)
{
    BossState b={0};b.stage=BOSS_FIGHT;b.phase=1;b.clock=seconds*60;
    DrawScene(&b,true,defeated);
}
