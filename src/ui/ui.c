#include "ui_internal.h"
#include "core/game.h"
#include <math.h>
#include <string.h>
static UiState state;
static bool Init(void *p){memset(p,0,sizeof(UiState));return true;}
static void Shutdown(void *p){memset(p,0,sizeof(UiState));}
static void Reset(void *p,const GameState *g,const WorldConfig *c)
{(void)c;UiState *s=p;memset(s,0,sizeof(*s));s->score=g->score;s->title_steps=185;s->input_mode=g->mode;}
unsigned UiButtons(GameMode mode,UiButton b[2])
{
    if(mode==GAME_PLAY || mode==GAME_PORTAL || mode==GAME_TRANSITION){b[0]=(UiButton){{888,18,54,52},COMMAND_TOGGLE_PAUSE,"II"};return 1;}
    if(mode!=GAME_INTRO && mode!=GAME_PAUSED && mode!=GAME_LOST && mode!=GAME_WON)return 0;
    b[0]=(UiButton){{300,354,360,52},mode==GAME_INTRO?COMMAND_START:mode==GAME_PAUSED?COMMAND_RESUME:COMMAND_RETRY,
        mode==GAME_INTRO?"Comecar aventura":mode==GAME_PAUSED?"Continuar":mode==GAME_LOST?"Tentar novamente":"Jogar novamente"};
    b[1]=(UiButton){{300,419,360,40},COMMAND_RESTART,"Voltar ao inicio"};return mode==GAME_INTRO?1:2;
}
static bool Inside(GameVec2 p,GameRect r)
{return p.x>=r.x && p.y>=r.y && p.x<r.x+r.width && p.y<r.y+r.height;}
static UiDecision Input(void *p,const GameState *g,const GameInput *in)
{
    UiState *s=p;UiDecision result={0};UiButton buttons[2];unsigned n=UiButtons(g->mode,buttons);
    if(s->input_mode!=g->mode){s->input_mode=g->mode;s->focus=0;}
    int nav=s->navigation;bool activate=s->activate;s->navigation=0;s->activate=false;
    if(!in->focused || !n)return result;
    if(nav)s->focus=(s->focus+(nav>0?1:(int)n-1))%(int)n;
    if(s->focus<0 || s->focus>=(int)n)s->focus=0;
    bool modal=g->mode==GAME_INTRO||g->mode==GAME_PAUSED||g->mode==GAME_LOST||g->mode==GAME_WON;
    if(in->pointer_pressed && in->pointer_inside){
        result.consumes_pointer=modal;
        for(unsigned i=0;i<n;i++)if(Inside(in->pointer,buttons[i].rect)){
            s->focus=(int)i;result.command=buttons[i].command;result.consumes_pointer=true;return result;
        }
    }
    if(activate || (modal && in->impulse_pressed))result.command=buttons[s->focus].command;
    return result;
}
static void Step(void *p,const GameState *g,const FrameContext *f,const ItemsView *v)
{
    UiState *s=p;if(g->mode!=GAME_PLAY && g->mode!=GAME_PORTAL && g->mode!=GAME_TRANSITION)return;
    float dt=f->ui_seconds*60;if(!isfinite(dt)||dt<=0)return;
    s->score_pulse=fmaxf(0,s->score_pulse-dt);s->charge_pulse=fmaxf(0,s->charge_pulse-dt);
    if(g->score!=s->score){s->score=g->score;s->score_pulse=17;}
    unsigned required=GameEnergyRequired(g->world);
    if(v->collected==required && s->collected<required)s->charge_pulse=35;
    s->collected=v->collected;float target=GameEnergyFraction(v->collected,g->world);
    s->battery+=(target-s->battery)*(1-expf(-dt/10));
    if(fabsf(target-s->battery)<.0001f)s->battery=target;
    if(g->mode!=GAME_TRANSITION)s->title_steps=fmaxf(0,s->title_steps-dt);
}
static void Draw(void *p,const GameState *g,const ItemsView *v){UiDraw(p,g,v);}
UiApi UiModuleCreate(void){return (UiApi){{&state,Init,Reset,Shutdown},Input,Step,Draw};}
void UiQueueNavigation(int direction,bool activate){if(direction)state.navigation=direction;state.activate=state.activate||activate;}
UiSnapshot UiInspect(void){return (UiSnapshot){state.battery,state.title_steps,state.score_pulse,state.charge_pulse,state.focus,state.collected};}
