/* Paired vector illustrations. All shapes reuse gameplay character/boss art.
 * Drawing is pure; the UI presentation clock is advanced by core outside play. */
#include "ui_internal.h"
#include "player/player.h"
#include "effects/boss_visual.h"
#include "raylib.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
static Color C(unsigned hex){return GetColor(hex);}
static void Text(const char *text,int x,int y,int size,Color color)
{DrawText(text,x,y,size,color);}
static int Paragraph(const char *text,int y,int size,Color color)
{
    char line[256]={0},word[128];size_t used=0;
    while(*text){
        while(*text==' ')text++;
        size_t n=0;while(*text && *text!=' ' && n<sizeof(word)-1)word[n++]=*text++;
        word[n]=0;if(!n)break;
        char candidate[384];snprintf(candidate,sizeof(candidate),"%s%s%s",line,used?" ":"",word);
        if(used && MeasureText(candidate,size)>320){Text(line,605,y,size,color);y+=size+6;used=0;line[0]=0;}
        if(used)line[used++]=' ';
        if(used+n<sizeof(line)){memcpy(line+used,word,n+1);used+=n;}
    }
    if(used){Text(line,605,y,size,color);y+=size+6;}return y;
}
static void Trees(bool happy,float t)
{
    for(int layer=0;layer<3;layer++)for(int i=0;i<7;i++){
        float x=i*104.0f-24+layer*27,y=160+layer*34+(i%3)*30;
        Color trunk=happy?C(layer==0?0x9fc6a7ff:layer==1?0x689d86ff:0x345e61ff):C(layer==0?0x262b46ff:layer==1?0x242239ff:0x171c30ff);
        float width=10+layer*7;DrawLineEx((Vector2){x,485},(Vector2){x+12,y},width,trunk);
        for(int j=0;j<4;j++){
            float yy=y+j*46,side=j%2?1:-1;
            DrawLineEx((Vector2){x+9,yy+48},(Vector2){x+side*(28+j*6),yy},width*.4f,trunk);
            if(happy)DrawEllipse((int)(x+side*24),(int)yy,32+layer*4,14,Fade(C(0x87b983ff),.36f));
        }
    }
    for(int i=0;i<8;i++){
        float x=fmodf(i*93+t*9,680)-70,y=190+(i%3)*66;
        DrawEllipse((int)x,(int)y,110,22,Fade(happy?C(0xf9edbdff):C(0x9273baff),happy?.08f:.065f));
    }
}
static void Illustration(bool happy,float t)
{
    BeginScissorMode(0,0,580,540);
    DrawRectangleGradientV(0,0,580,540,happy?C(0xaadfd5ff):C(0x11182fff),happy?C(0xf8dfafff):C(0x51405cff));
    DrawCircleGradient(306,120,happy?200:150,Fade(happy?C(0xfff4bfff):C(0xb48cffff),.32f),BLANK);
    DrawCircle(306,120,happy?55:45,happy?C(0xfff1b5ff):C(0x8f85b4ff));
    if(!happy)DrawCircle(325,108,40,C(0x26213eff));
    Trees(happy,t);
    if(happy){
        for(int i=0;i<3;i++)DrawCircleLines(305,146,110+i*10,Fade(C(0xfff5c6ff),.3f));
    }
    rlPushMatrix();
    if(happy){rlTranslatef(300,310,0);rlRotatef(77,0,0,1);rlScalef(.68f,.68f,1);}
    else{rlTranslatef(316,207,0);rlScalef(.82f,.82f,1);}
    rlTranslatef(-750,-300,0);BossDrawPortrait(t,happy);rlPopMatrix();
    DrawEllipse(275,545,395,183,happy?C(0x628b69ff):C(0x24283aff));
    DrawEllipse(258,550,355,166,happy?C(0x8caf7aff):C(0x34344aff));
    DrawEllipse(256,556,302,154,happy?C(0xb5c78aff):C(0x444057ff));
    /* The same six heroes share two rows in both compositions. */
    const GameVec2 poses[]={{89,383},{233,392},{385,379},{152,447},{314,445},{470,441}};
    for(int i=0;i<6;i++){
        float bounce=happy?fabsf(sinf(t*2.7f+i*.8f))*10:sinf(t*1.7f+i)*2;
        GameVec2 p={poses[i].x,poses[i].y-bounce};float scale=i<3?1.35f:1.52f;
        DrawEllipse((int)p.x,(int)poses[i].y+32,34,8,Fade(C(0x121b2aff),.25f));
        PlayerDrawPortrait(i,p,scale,t+i*.13f,happy);
        if(!happy){
            DrawCircle((int)p.x-24,(int)p.y-23,3,Fade(C(0x96e0ffff),.8f));
            DrawTriangle((Vector2){p.x-27,p.y-24},(Vector2){p.x-21,p.y-24},(Vector2){p.x-24,p.y-32},C(0x96e0ffff));
        }
    }
    for(int i=0;i<38;i++){
        float x=fmodf(i*97.0f+t*(happy?13:4),550),y=fmodf(i*61.0f-t*(happy?14:6)+1620,540);
        Color color=happy?ColorFromHSV((float)(i*43%360),.45f,.95f):C(0xba99e0ff);
        float alpha=happy?.65f:.25f;
        if(y<350 || x<35 || x>535){
            DrawCircle((int)x,(int)y,i%3+1,Fade(color,alpha));
            if(happy && i%4==0){DrawLine((int)x-4,(int)y,(int)x+4,(int)y,Fade(color,.6f));DrawLine((int)x,(int)y-4,(int)x,(int)y+4,Fade(color,.6f));}
        }
    }
    for(int i=0;i<20;i++){
        float x=i*31.0f-10,y=484+(i%4)*6;
        Color leaf=happy?C(0x426d59ff):C(0x24263aff);
        DrawLineEx((Vector2){x,y+28},(Vector2){x+7,y},2,leaf);
        DrawEllipse((int)x+9,(int)y+8,8,3,leaf);
        if(happy && i%3==0){DrawCircle((int)x+7,(int)y,4,C(0xffe7b0ff));DrawCircle((int)x+7,(int)y,1,C(0xcfa365ff));}
    }
    DrawRectangleGradientV(0,486,580,54,BLANK,Fade(happy?C(0x214d44ff):C(0x101b2dff),.85f));
    Text(happy?"O EQUILÍBRIO VOLTOU.":"SEIS MUNDOS. UMA ÚNICA AVENTURA.",28,511,15,C(0xe9eedcff));
    EndScissorMode();
}
void UiNarrativeDraw(const UiState *s,const GameState *g)
{
    bool happy=g->mode==GAME_WON;float t=s->narrative_seconds;
    Illustration(happy,t);
    DrawRectangle(580,0,380,540,C(0x102435ff));
    DrawRectangleGradientH(550,0,40,540,BLANK,C(0x102435ff));
    DrawLine(580,25,580,515,Fade(happy?C(0xadddc5ff):C(0xc0a0efff),.25f));
    Color ink=C(0xe8efe6ff),accent=happy?C(0xafe6cbff):C(0xd0b5f5ff),gold=C(0xf3d89dff);
    Text(happy?"EPÍLOGO  /  O EQUILÍBRIO RESTAURADO":"PRÓLOGO  /  A SOMBRA ENTRE OS MUNDOS",605,23,11,accent);
    if(!happy){
        Text("MORPH",603,56,43,gold);Text("ADVENTURE",603,104,37,gold);
        Text("Os mundos estão em perigo!",605,161,21,accent);
        DrawLine(605,195,925,195,Fade(accent,.3f));
        int y=Paragraph("Uma força sombria conhecida como Grootspectro ameaça destruir o equilíbrio entre os mundos. Agora, nossos heróis precisam superar desafios, reunir suas forças e enfrentar essa terrível ameaça.",218,18,ink);
        Paragraph("O destino dos mundos está em suas mãos!",y+19,18,gold);
        Text("Espaço / cima / W: voar | J: megarraio",605,505,13,ink);
    }else{
        Text("VITÓRIA!",603,53,36,gold);Text("OS MUNDOS FORAM SALVOS!",605,98,19,accent);
        int y=Paragraph("Parabéns, herói!",139,18,gold);
        y=Paragraph("Depois de atravessar os mundos, superar inúmeros desafios e enfrentar o poderoso Grootspectro, você finalmente conseguiu derrotá-lo!",y+10,16,ink);
        y=Paragraph("Graças à sua coragem e determinação, o equilíbrio foi restaurado e todos os mundos estão novamente em segurança.",y+10,16,ink);
        Paragraph("Sua aventura ficará marcada para sempre!",y+10,16,accent);
        char score[80];snprintf(score,sizeof(score),"Pontuação final: %" PRIu64,g->score);Text(score,605,401,18,gold);
    }
}
