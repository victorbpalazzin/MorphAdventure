#include "worlds_internal.h"
#include <math.h>

static float Wrap(double value,double period)
{ double x=fmod(value,period);return (float)(x<0?x+period:x); }
static void Glow(float x,float y,float radius,Color color)
{ DrawCircleGradient((int)x,(int)y,radius,color,Fade(color,0)); }
static void Aurora(double clock)
{
    for(int band=0;band<4;band++) {
        Vector2 points[49];
        for(int i=0;i<49;i++) {
            float x=i*20.0f;
            points[i]=(Vector2){x,112+band*13+(float)sin(x*.006+clock*.003+band*.35)*42};
        }
        WLine(points,49,Fade(band%2?WC(0xb5f5ef):WC(0xc5b2ff),.10f),15);
        WLine(points,49,GetColor(0xd6fff81b),2);
    }
}
static void Particles(int world,double time,double scroll)
{
    for(int i=0;i<38;i++) {
        int direction=world==1 || world==5?-1:1;
        float x=Wrap(i*97.1-scroll*.2+time*(world==4?1.7:.12),1000)-20;
        float y=Wrap(i*61+time*direction*(world==3?.6:.3),510);
        if(world==1) DrawCircleLinesV((Vector2){x,y},2+i%4,GetColor(0xa1ffeb44));
        else if(world==2) {
            if(i%3==0)Glow(x,y,8,GetColor(0xeaff9520));
            WOval(x,y,i%3==0?2:3,i%3==0?2:1,GetColor(0xe4f7a888),(float)fmod(time*.01+i,2*PI));
        } else if(world==4) WL(GetColor(0xffdf9c33),1,{x,y},{x+8,y-1});
        else WOval(x,y,world==3?1.5f:1.2f,world==3?1.5f:2,world==5?GetColor(0xffba7588):GetColor(0xfffbeaaa),0);
    }
}
static void Environment(const WorldScene *scene,double time,const WorldPalette *palette)
{
    float strength=scene->environment.intensity*(scene->environment.warning?.35f:1);
    if(strength<=0)return;
    Color color=Fade(GetColor(palette->accent),strength*.12f);
    if(scene->world==0 || scene->world==1 || scene->world==3 || scene->world==4) {
        for(int i=0;i<7;i++) {
            float x=Wrap(i*157+time*.8,1100)-80,y=160+i%3*90;
            float bend=scene->environment.direction*18;
            WCurve((Vector2){x,y},(Vector2){x+30,y-bend},(Vector2){x+50,y+bend},(Vector2){x+95,y-bend},color,2);
        }
    }
    if(scene->world==3 || scene->world==4 || scene->world==5)
        DrawRectangle(0,0,MORPH_WIDTH,MORPH_HEIGHT,Fade(GetColor(palette->accent),strength*.035f));
    /* No danger markers, warning text, hitboxes or event scheduling here. */
}
void WorldDrawBackground(const WorldScene *scene,const GameState *state)
{
    const WorldPalette *a=WorldsPalette(scene->world);
    int world=scene->world;double time=scene->visual_steps;
    double scroll=isfinite(state->scroll)?state->scroll:0;
    DrawRectangleGradientV(0,0,MORPH_WIDTH,270,GetColor(a->sky[0]),GetColor(a->sky[1]));
    DrawRectangleGradientV(0,270,MORPH_WIDTH,270,GetColor(a->sky[1]),GetColor(a->sky[2]));
    if(world==0 || world==3 || world==4) {
        Glow(740,95,180,world==3?GetColor(0xd4eaff55):GetColor(0xffe9b666));
        WOval(740,95,world==4?51:33,world==4?51:33,world==3?WC(0xf1f5ff):WC(0xffe8b9),0);
    }
    if(world<=2)for(int i=0;i<5;i++)
        WP(Fade(WC(0xfff5c5),.055f),{130+i*210.0f,-10},{170+i*210.0f,-10},{320+i*210.0f,540},{190+i*210.0f,540});
    if(world==0)for(int i=0;i<7;i++) {
        float x=Wrap(i*187-scroll*.08-time*.12,1250)-140;
        WCloud(x,110+i%3*43,.65f+i%2*.35f,GetColor(0xe2f4f2a0));
    }
    if(world==3)Aurora(time);
    const double speeds[]={.12,.3,.57};const float opacity[]={.65f,.8f,1};
    for(int depth=0;depth<3;depth++) {
        float offset=Wrap(scroll*speeds[depth],WORLD_LAYER_WIDTH);
        for(int copy=0;copy<2;copy++)
            DrawTextureRec(scene->layers[depth].texture,(Rectangle){0,0,WORLD_LAYER_WIDTH,-MORPH_HEIGHT},
                           (Vector2){-offset+copy*WORLD_LAYER_WIDTH,0},Fade(WHITE,opacity[depth]));
    }
    if(world==1) {
        for(int i=0;i<14;i++) {
            float x=Wrap(i*79+time*.3-scroll*.24,1050)-40,y=200+i%5*40;
            WOval(x,y,9,3,GetColor(0x79c3c34d),0);
            WP(GetColor(0x79c3c34d),{x-7,y},{x-15,y-5},{x-15,y+5});
        }
        for(int i=0;i<9;i++) {
            float x=i*122-Wrap(scroll*.7,122);
            WCurve((Vector2){x,528},(Vector2){x-16,504},(Vector2){x+(float)sin(time*.025+i)*25,476},
                   (Vector2){x+(float)sin(time*.02+i)*18,453},GetColor(0x48ab9d88),4);
        }
    }
    if(world==3)for(int i=0;i<4;i++) {
        float x=Wrap(i*330-scroll*.09-time*.07,1320)-160;
        WOval(x,452+i%2*24,210,18,GetColor(0xe7f7ff0c),0);
    }
    if(world==4) {
        float x=Wrap(760-scroll*.18,1150)-80;
        for(int i=0;i<6;i++)WOval(x+(float)sin(time*.03+i)*10,485-i*14,10+i*6,3,GetColor(0xffe2a120),0);
    }
    if(world==5) {
        DrawRectangle(0,512,MORPH_WIDTH,28,WC(0xe4673d));
        for(int i=0;i<18;i++)WOval(Wrap(i*67-scroll*.8,1100)-40,520+(float)sin(time*.04+i)*4,29,3,i%2?WC(0xffc576):WC(0xff9a46),0);
        Glow(480,570,300,GetColor(0xff932e44));
        Glow(700,465,210,Fade(WC(0xff9b4d),(24+12*(float)sin(time*.035))/255));
        for(int i=0;i<8;i++) {
            float x=Wrap(i*163-scroll*.12,1150),y=315-Wrap(time*.25+i*29,110);
            WOval(x+(float)sin(time*.012+i)*14,y,24+(315-y)*.18f,16,GetColor(0xd3a39e09),0);
        }
        for(int i=0;i<4;i++) {
            Vector2 points[33];
            for(int j=0;j<33;j++)points[j]=(Vector2){j*30.0f,490-i*12+(float)sin(j*.6+time*.04)*3};
            WLine(points,33,GetColor(0xffcb8e15),2);
        }
    } else {
        DrawRectangle(0,518,MORPH_WIDTH,22,world==0?WC(0x8aaeb5):GetColor(a->ink));
        WL(Fade(GetColor(a->accent),.33f),2,{0,518},{960,518});
    }
    Particles(world,time,scroll);Environment(scene,time,a);
    /* Subtle edge shading, leaving the gameplay corridor readable. */
    DrawRectangleGradientV(0,0,960,38,GetColor(0x07152730),GetColor(0x07152700));
    DrawRectangleGradientH(0,0,100,540,GetColor(0x07152726),GetColor(0x07152700));
    DrawRectangleGradientH(860,0,100,540,GetColor(0x07152700),GetColor(0x07152726));
}
