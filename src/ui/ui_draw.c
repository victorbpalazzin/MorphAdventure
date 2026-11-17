#include "ui_internal.h"
#include "core/game.h"
#include "items/items.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
static const char *names[]={"Ar","Agua","Floresta","Gelo","Terra","Fogo"};
static const Color ink={237,246,236,255},gold={244,218,158,255},mint={156,239,213,255};
static void Center(const char *text,int y,int size,Color c)
{DrawText(text,480-MeasureText(text,size)/2,y,size,c);}
static void Panel(Rectangle r,Color color)
{DrawRectangleRounded(r,.12f,12,color);DrawRectangleRoundedLinesEx(r,.12f,12,1,GetColor(0xb5d9ca55));}
static void Heart(int x,int y,bool alive)
{
    Color c=alive?GetColor(0xf39d9bff):GetColor(0x73818b88);
    DrawCircle(x-5,y-3,6,c);DrawCircle(x+5,y-3,6,c);
    DrawTriangle((Vector2){x-11,y-2},(Vector2){x,y+12},(Vector2){x+11,y-2},c);
}
static void Button(UiButton b,bool focus)
{
    Rectangle r={b.rect.x,b.rect.y,b.rect.width,b.rect.height};
    Color fill=b.command==COMMAND_RESTART?GetColor(0x254251ff):GetColor(0xa9e5ceff);
    Panel(r,fill);if(focus)DrawRectangleRoundedLinesEx((Rectangle){r.x-4,r.y-4,r.width+8,r.height+8},.16f,12,2,gold);
    int size=b.command==COMMAND_RESTART?18:22;
    DrawText(b.label,(int)(r.x+(r.width-MeasureText(b.label,size))/2),(int)(r.y+(r.height-size)/2),size,b.command==COMMAND_RESTART?ink:GetColor(0x173443ff));
}
void UiDraw(const UiState *s,const GameState *g,const ItemsView *v)
{
    char text[160];int world=g->world>=0 && g->world<6?g->world:0;
    Panel((Rectangle){18,12,852,68},GetColor(0x10283de6));
    snprintf(text,sizeof(text),"%02d / 06  %s",world+1,names[world]);DrawText(text,32,20,18,gold);
    for(int i=0;i<3;i++)Heart(43+i*30,57,i<g->lives);
    snprintf(text,sizeof(text),"PONTOS  %" PRIu64,g->score);
    DrawText(text,195,24,s->score_pulse>0?22:20,s->score_pulse>0?gold:ink);
    float progress=g->minimum_distance>0?fminf(1,fmaxf(0,g->distance/g->minimum_distance)):0;
    snprintf(text,sizeof(text),"Percurso %d%%",(int)floorf(progress*100));DrawText(text,195,52,16,mint);
    unsigned required=GameEnergyRequired(g->world);bool full=required>0 && v->collected>=required;
    float fraction=GameEnergyFraction(v->collected,g->world);
    snprintf(text,sizeof(text),"ENERGIA  %u/%u  %d%%",v->collected,required,full?100:(int)floorf(fraction*100));
    DrawText(text,563,20,18,full?mint:ink);
    Rectangle battery={565,47,275,22};
    if(full)DrawRectangleRounded((Rectangle){560,42,285,32},.4f,12,Fade(mint,.12f+.12f*fabsf(sinf(s->charge_pulse*.25f))));
    DrawRectangleRoundedLinesEx(battery,.2f,8,2,ink);DrawRectangle(841,53,6,10,ink);
    float width=267*fminf(fraction,fmaxf(0,s->battery));
    if(width>0)DrawRectangleGradientH(569,51,(int)width,14,GetColor(0x52cbb3ff),GetColor(0xd7ff8aff));
    for(unsigned i=0;i<v->boost_count && i<MORPH_MAX_BOOSTS;i++){
        const ItemBoostDefinition *d=ItemsBoostDefinition(v->boosts[i].type);if(!d)continue;
        if(v->boosts[i].seconds_left<0)snprintf(text,sizeof(text),"%s | 1 impacto",d->name);
        else snprintf(text,sizeof(text),"%s | %ds",d->name,(int)ceilf(v->boosts[i].seconds_left));
        Panel((Rectangle){18+i*206.0f,87,198,32},GetColor(0x10283dd9));DrawText(text,28+(int)i*206,95,16,GetColor(d->color));
    }
    if(s->title_steps>0 && (g->mode==GAME_PLAY || g->mode==GAME_PORTAL)){
        float alpha=fminf(1,fminf(s->title_steps/35,(185-s->title_steps)/20));
        DrawRectangleRounded((Rectangle){350,119,260,70},.25f,12,Fade(GetColor(0x10283dff),alpha*.55f));
        snprintf(text,sizeof(text),"MUNDO %02d",world+1);Center(text,127,16,Fade(gold,alpha));Center(names[world],150,30,Fade(ink,alpha));
    }
    bool modal=g->mode==GAME_INTRO||g->mode==GAME_PAUSED||g->mode==GAME_LOST||g->mode==GAME_WON;
    if(modal){
        DrawRectangle(0,0,960,540,GetColor(0x071322a6));Panel((Rectangle){150,98,660,382},GetColor(0x132c3ff5));
        DrawLineEx((Vector2){240,125},(Vector2){720,125},2,Fade(gold,.6f));
        const char *title=g->mode==GAME_INTRO?"MORPH ADVENTURE":g->mode==GAME_PAUSED?"A aventura espera":g->mode==GAME_LOST?"Mais uma tentativa?":"Seis mundos. Uma aventura.";
        Center(title,150,g->mode==GAME_INTRO?32:30,gold);
        if(g->mode==GAME_INTRO){
            Center("Uma jornada atraves dos elementos",199,20,ink);
            Center("Tres vidas para atravessar os seis mundos.",242,20,ink);
            Center("Colete 4, 8, 12, 16, 20 e 24 fragmentos de energia.",273,18,mint);
            Center("Espaco / seta para cima / W / clique para voar",309,18,ink);
        }else if(g->mode==GAME_PAUSED){
            Center("Tudo fica congelado ate voce continuar.",230,20,ink);
            Center("P continua. Espaco ativa o botao em foco.",274,20,mint);
        }else if(g->mode==GAME_LOST){
            Center("Uma nova partida comeca no Ar, com tres vidas.",230,20,ink);
            snprintf(text,sizeof(text),"Pontuacao: %" PRIu64,g->score);Center(text,278,24,mint);
        }else{
            Center("Voce atravessou o portal do Fogo!",230,22,ink);
            snprintf(text,sizeof(text),"Pontuacao final: %" PRIu64,g->score);Center(text,278,24,mint);
        }
    }
    UiButton b[2];unsigned n=UiButtons(g->mode,b);
    for(unsigned i=0;i<n;i++)Button(b[i],s->focus==(int)i && modal);
    if(modal)Center("Espaco ativa a opcao em foco  |  P pausa durante a partida",497,16,ink);
}
