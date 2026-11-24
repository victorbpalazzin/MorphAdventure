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
static void AudioPanel(const UiAudioView *a)
{
    static const char *labels[3]={"Geral","Musica","Efeitos"};
    UiAudioRects r=UiAudioLayout();char text[32];
    Panel((Rectangle){r.panel.x,r.panel.y,r.panel.width,r.panel.height},GetColor(0x0d2233f0));
    DrawText("SOM",210,208,18,gold);
    DrawText("Tudo congelado ate voce continuar.",262,211,14,Fade(ink,.75f));
    Rectangle m={r.mute.x,r.mute.y,r.mute.width,r.mute.height};
    Panel(m,a->muted?GetColor(0x6b2a33ff):GetColor(0x1f6b4cff));
    const char *mute=a->muted?"Mudo":"Som ligado";
    DrawText(mute,(int)(m.x+(m.width-MeasureText(mute,16))/2),(int)m.y+5,16,ink);
    for(int i=0;i<3;i++){
        float y=r.minus[i].y;bool sel=a->selected==i;
        if(sel)DrawRectangleRoundedLinesEx((Rectangle){200,y-3,560,32},.3f,8,2,Fade(gold,.8f));
        DrawText(labels[i],214,(int)y+4,18,sel?gold:ink);
        for(int k=0;k<2;k++){
            GameRect b=k?r.plus[i]:r.minus[i];
            Panel((Rectangle){b.x,b.y,b.width,b.height},GetColor(0x254251ff));
            DrawText(k?"+":"-",(int)(b.x+b.width/2-(k?5:3)),(int)b.y+3,20,ink);
        }
        Rectangle bar={r.bar[i].x,r.bar[i].y,r.bar[i].width,r.bar[i].height};
        DrawRectangleRounded(bar,.5f,8,GetColor(0x0b1d2dff));
        float v=a->muted?0:a->volume[i];
        if(v>0)DrawRectangleRounded((Rectangle){bar.x,bar.y,bar.width*v,bar.height},.5f,8,a->muted?Fade(mint,.3f):mint);
        snprintf(text,sizeof(text),"%d%%",(int)(a->volume[i]*100+.5f));
        DrawText(text,692,(int)y+5,16,a->muted?Fade(ink,.5f):ink);
    }
}
void UiDraw(const UiState *s,const GameState *g,const ItemsView *v)
{
    if(g->mode==GAME_INTRO || g->mode==GAME_WON){
        UiNarrativeDraw(s,g);UiButton buttons[2];unsigned count=UiButtons(g->mode,buttons);
        for(unsigned i=0;i<count;i++)Button(buttons[i],s->focus==(int)i);
        return;
    }
    char text[160];int world=g->world>=0 && g->world<6?g->world:0;
    Panel((Rectangle){18,12,852,68},GetColor(0x10283de6));
    snprintf(text,sizeof(text),"%02d / 06  %s",world+1,names[world]);DrawText(text,32,20,18,gold);
    for(int i=0;i<(g->lives>3?g->lives:3) && i<6;i++)Heart(40+i*25,57,i<g->lives);
    snprintf(text,sizeof(text),"PONTOS  %" PRIu64,g->score);
    DrawText(text,195,24,s->score_pulse>0?22:20,s->score_pulse>0?gold:ink);
    float progress=g->minimum_distance>0?fminf(1,fmaxf(0,g->distance/g->minimum_distance)):0;
    snprintf(text,sizeof(text),"Percurso %d%%",(int)floorf(progress*100));DrawText(text,195,52,16,mint);
    bool combat=g->boss.stage!=BOSS_DORMANT;
    unsigned collected=combat?g->boss.energy.collected:v->collected;
    unsigned required=combat?g->boss.energy.required:GameEnergyRequired(g->world);bool full=required>0 && collected>=required;
    float fraction=combat?(required?(float)collected/required:0):GameEnergyFraction(v->collected,g->world);
    UiProgressLabel(g,collected,required,full?100:(int)floorf(fraction*100),text,sizeof(text));
    DrawText(text,563,20,18,full?mint:ink);
    Rectangle battery={565,47,275,22};
    if(full)DrawRectangleRounded((Rectangle){560,42,285,32},.4f,12,Fade(mint,.12f+.12f*fabsf(sinf(s->charge_pulse*.25f))));
    DrawRectangleRoundedLinesEx(battery,.2f,8,2,ink);DrawRectangle(841,53,6,10,ink);
    float width=267*fminf(fraction,fmaxf(0,combat?g->boss.battery_display:s->battery));
    if(width>0)DrawRectangleGradientH(569,51,(int)width,14,GetColor(0x52cbb3ff),GetColor(0xd7ff8aff));
    if(combat){
        for(int i=0;i<7;i++){
            float segment=267.0f/7,amount=fminf(segment,fmaxf(0,width-i*segment));
            if(amount>0)DrawRectangle((int)(569+i*segment),51,(int)amount,14,ColorFromHSV(i*48,.7f,1));
        }
        Panel((Rectangle){300,87,560,65},GetColor(0x170e2aee));
        DrawText("GROOTSPECTRO",316,95,20,GetColor(0xe2bdffff));
        snprintf(text,sizeof(text),"FASE %u / 3",g->boss.phase);DrawText(text,723,98,16,ink);
        for(unsigned i=0;i<3;i++)DrawRectangleRounded((Rectangle){316+i*176.0f,123,166,16},.3f,8,i<g->boss.segments?GetColor(0xad63e6ff):GetColor(0x35263fff));
        if(g->mode==GAME_BOSS){
            Panel((Rectangle){235,492,490,33},GetColor(0x10283de6));
            bool pad=UiGamepadHints();
            const char *hint=g->boss.stage==BOSS_DYING?"O equilíbrio está sendo restaurado...":
                g->boss.beam_active?"MEGARRAIO EM AÇÃO!":full?(pad?"MEGARRAIO PRONTO!  Aperte Quadrado":"MEGARRAIO PRONTO!  Aperte J"):
                pad?"Colete energia multicolorida  |  Quadrado dispara o megarraio":"Colete energia multicolorida  |  J dispara o megarraio";
            Center(hint,501,16,full?gold:ink);
        }
    }
    for(unsigned i=0;i<v->boost_count && i<MORPH_MAX_BOOSTS;i++){
        const ItemBoostDefinition *d=ItemsBoostDefinition(v->boosts[i].type);if(!d)continue;
        if(v->boosts[i].seconds_left<0)snprintf(text,sizeof(text),"%s | 1 impacto",d->name);
        else snprintf(text,sizeof(text),"%s | %ds",d->name,(int)ceilf(v->boosts[i].seconds_left));
        float bx=combat?18:18+i*206.0f,by=combat?87+i*36.0f:87;
        Panel((Rectangle){bx,by,198,32},GetColor(0x10283dd9));DrawText(text,(int)bx+10,(int)by+8,16,GetColor(d->color));
    }
    if(s->title_steps>0 && (g->mode==GAME_PLAY || g->mode==GAME_PORTAL)){
        float alpha=fminf(1,fminf(s->title_steps/35,(185-s->title_steps)/20));
        DrawRectangleRounded((Rectangle){350,119,260,70},.25f,12,Fade(GetColor(0x10283dff),alpha*.55f));
        snprintf(text,sizeof(text),"MUNDO %02d",world+1);Center(text,127,16,Fade(gold,alpha));Center(names[world],150,30,Fade(ink,alpha));
    }
    if(g->edu.enabled && g->edu.grace_steps>0 && (g->mode==GAME_PLAY || g->mode==GAME_BOSS)){
        snprintf(text,sizeof(text),"PROTEGIDO  %ds",(int)ceilf(g->edu.grace_steps/60));
        Panel((Rectangle){18,492,200,32},GetColor(0x10283de6));DrawText(text,32,499,18,mint);
    }
    bool modal=g->mode==GAME_PAUSED||g->mode==GAME_LOST;
    if(modal){
        DrawRectangle(0,0,960,540,GetColor(0x071322a6));Panel((Rectangle){150,98,660,382},GetColor(0x132c3ff5));
        DrawLineEx((Vector2){240,125},(Vector2){720,125},2,Fade(gold,.6f));
        Center(g->mode==GAME_PAUSED?"A aventura espera":"Mais uma tentativa?",150,30,gold);
        if(g->mode==GAME_PAUSED){
            UiAudioView a=UiGetAudioView();
            if(a.available)AudioPanel(&a);
            else{
                Center("Tudo fica congelado ate voce continuar.",230,20,ink);
                Center(UiGamepadHints()?"OPTIONS ou Circulo continua. X ativa o botao em foco.":"P continua. Espaco ativa o botao em foco.",274,20,mint);
            }
        }else{
            Center("Uma nova partida comeca no Ar, com tres vidas.",230,20,ink);
            snprintf(text,sizeof(text),"Pontuacao: %" PRIu64,g->score);Center(text,278,24,mint);
        }
    }
    UiButton b[2];unsigned n=UiButtons(g->mode,b);
    for(unsigned i=0;i<n;i++)Button(b[i],s->focus==(int)i && modal);
    if(modal)Center(UiGamepadHints()?(g->mode==GAME_PAUSED && UiGetAudioView().available?
        "X confirma  |  Circulo volta  |  L1/R1 canal  |  <- -> volume  |  Triangulo: mudo":
        "Direcional escolhe  |  X confirma  |  Circulo volta  |  OPTIONS pausa"):
        g->mode==GAME_PAUSED && UiGetAudioView().available?"Espaco ativa  |  P continua  |  1-3 e setas: volume  |  M: mudo":
        "Espaco ativa a opcao em foco  |  P pausa durante a partida",497,16,ink);
    if(UiGetAudioView().muted && (g->mode==GAME_PLAY||g->mode==GAME_PORTAL||g->mode==GAME_TRANSITION||g->mode==GAME_BOSS))
        DrawText("MUDO (M)",884,74,14,Fade(gold,.85f));
    if(g->mode==GAME_QUIZ)UiQuizDraw(s,g);
}
