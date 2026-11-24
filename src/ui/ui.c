#include "ui_internal.h"
#include "core/game.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static UiState state;
static bool Init(void *p){memset(p,0,sizeof(UiState));return true;}
static void Shutdown(void *p){memset(p,0,sizeof(UiState));}
static void Reset(void *p,const GameState *g,const WorldConfig *c)
{(void)c;UiState *s=p;memset(s,0,sizeof(*s));s->score=g->score;s->title_steps=185;s->input_mode=g->mode;s->quiz_focus=-1;s->quiz_serial=g->edu.quiz.serial;}
unsigned UiButtons(GameMode mode,UiButton b[2])
{
    if(mode==GAME_INTRO || mode==GAME_WON){
        b[0]=(UiButton){{605,mode==GAME_INTRO?438:430,320,46},mode==GAME_INTRO?COMMAND_START:COMMAND_RETRY,mode==GAME_INTRO?"Iniciar Aventura":"Jogar Novamente"};
        b[1]=(UiButton){{605,485,320,36},COMMAND_RESTART,"Menu Principal"};
        return mode==GAME_INTRO?1:2;
    }
    if(mode==GAME_PLAY || mode==GAME_PORTAL || mode==GAME_TRANSITION || mode==GAME_BOSS){b[0]=(UiButton){{888,18,54,52},COMMAND_TOGGLE_PAUSE,"II"};return 1;}
    if(mode!=GAME_PAUSED && mode!=GAME_LOST)return 0;
    b[0]=(UiButton){{300,354,360,52},mode==GAME_PAUSED?COMMAND_RESUME:COMMAND_RETRY,
        mode==GAME_PAUSED?"Continuar":"Tentar novamente"};
    b[1]=(UiButton){{300,419,360,40},COMMAND_RESTART,"Voltar ao inicio"};return 2;
}
unsigned UiQuizButtons(const QuizState *q,UiButton b[4])
{
    if(q->kind==QUIZ_NONE)return 0;
    if(q->phase==QUIZ_FEEDBACK){
        const char *label=q->kind==QUIZ_REVIVE?(q->result==QUIZ_CORRECT?"Continuar aventura":"Ver resultado"):
            q->result==QUIZ_CORRECT?"Abrir portal":"Próxima pergunta";
        b[0]=(UiButton){{320,420,320,50},COMMAND_CONTINUE,label};return 1;
    }
    static const GameCommand answers[4]={COMMAND_ANSWER_A,COMMAND_ANSWER_B,COMMAND_ANSWER_C,COMMAND_ANSWER_D};
    float top=q->kind==QUIZ_REVIVE?290:262;
    for(unsigned i=0;i<4;i++)b[i]=(UiButton){{96,top+i*52.0f,768,44},answers[i],""};
    return 4;
}
void UiProgressLabel(const GameState *g,unsigned collected,unsigned required,int percent,char *out,size_t size)
{
    if(!out || !size)return;
    if(g && g->edu.enabled && g->boss.stage==BOSS_DORMANT)snprintf(out,size,"LETRAS: %u/%u",collected,required);
    else snprintf(out,size,"ENERGIA  %u/%u  %d%%",collected,required,percent);
}
static bool Inside(GameVec2 p,GameRect r)
{return p.x>=r.x && p.y>=r.y && p.x<r.x+r.width && p.y<r.y+r.height;}
static UiDecision QuizInput(UiState *s,const GameState *g,const GameInput *in,int nav,bool activate)
{
    UiDecision result={0};UiButton b[4];const QuizState *q=&g->edu.quiz;unsigned n=UiQuizButtons(q,b);
    if(s->quiz_serial!=q->serial){s->quiz_serial=q->serial;s->quiz_focus=q->phase==QUIZ_FEEDBACK?0:-1;}
    if(!in->focused || !n)return result;
    if(nav)s->quiz_focus=s->quiz_focus<0?(nav>0?0:(int)n-1):(s->quiz_focus+(nav>0?1:(int)n-1))%(int)n;
    if(s->quiz_focus>=(int)n)s->quiz_focus=-1;
    if(in->pointer_pressed && in->pointer_inside){
        result.consumes_pointer=true; /* a click on the challenge never becomes a flap */
        for(unsigned i=0;i<n;i++)if(Inside(in->pointer,b[i].rect)){s->quiz_focus=(int)i;result.command=b[i].command;return result;}
    }
    if((activate || in->impulse_pressed) && s->quiz_focus>=0)result.command=b[s->quiz_focus].command;
    return result;
}
static UiAudioView audio_view;
static UiAudioRequest audio_request={-1,0,false};
UiAudioRects UiAudioLayout(void)
{
    UiAudioRects r;memset(&r,0,sizeof(r));
    r.panel=(GameRect){190,198,580,138};r.mute=(GameRect){646,206,112,24};
    for(int i=0;i<3;i++){
        float y=238+i*31.0f;
        r.minus[i]=(GameRect){352,y,30,26};r.bar[i]=(GameRect){392,y+8,250,10};r.plus[i]=(GameRect){652,y,30,26};
    }
    return r;
}
void UiSetAudioView(UiAudioView view){audio_view=view;}
UiAudioView UiGetAudioView(void){return audio_view;}
UiAudioRequest UiTakeAudioRequest(void){UiAudioRequest r=audio_request;audio_request=(UiAudioRequest){-1,0,false};return r;}
static bool AudioClick(GameVec2 p)
{
    if(!audio_view.available)return false;
    UiAudioRects r=UiAudioLayout();
    if(Inside(p,r.mute)){audio_request.toggle_mute=true;return true;}
    for(int i=0;i<3;i++){
        if(Inside(p,r.minus[i])){audio_request.channel=i;audio_request.delta=-1;return true;}
        if(Inside(p,r.plus[i])){audio_request.channel=i;audio_request.delta=1;return true;}
    }
    return false;
}
static UiDecision Input(void *p,const GameState *g,const GameInput *in)
{
    UiState *s=p;UiDecision result={0};UiButton buttons[2];unsigned n=UiButtons(g->mode,buttons);
    if(s->input_mode!=g->mode){s->input_mode=g->mode;s->focus=0;}
    int nav=s->navigation;bool activate=s->activate;s->navigation=0;s->activate=false;
    if(g->mode==GAME_QUIZ)return QuizInput(s,g,in,nav,activate);
    if(!in->focused || !n)return result;
    if(nav)s->focus=(s->focus+(nav>0?1:(int)n-1))%(int)n;
    if(s->focus<0 || s->focus>=(int)n)s->focus=0;
    bool modal=g->mode==GAME_INTRO||g->mode==GAME_PAUSED||g->mode==GAME_LOST||g->mode==GAME_WON;
    if(in->pointer_pressed && in->pointer_inside){
        result.consumes_pointer=modal;
        if(g->mode==GAME_PAUSED && AudioClick(in->pointer)){result.consumes_pointer=true;return result;}
        for(unsigned i=0;i<n;i++)if(Inside(in->pointer,buttons[i].rect)){
            s->focus=(int)i;result.command=buttons[i].command;result.consumes_pointer=true;return result;
        }
    }
    if(activate || (modal && in->impulse_pressed))result.command=buttons[s->focus].command;
    return result;
}
static void Step(void *p,const GameState *g,const FrameContext *f,const ItemsView *v)
{
    UiState *s=p;
    if(g->mode==GAME_INTRO || g->mode==GAME_WON){
        if(s->narrative_mode!=g->mode){s->narrative_seconds=0;s->narrative_mode=g->mode;}
        if(isfinite(f->ui_seconds) && f->ui_seconds>0)s->narrative_seconds+=fminf(f->ui_seconds,.1f);
        return;
    }
    if(g->mode!=GAME_PLAY && g->mode!=GAME_PORTAL && g->mode!=GAME_TRANSITION && g->mode!=GAME_BOSS)return;
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
static bool gamepad_hints;
void UiSetGamepadHints(bool gamepad){gamepad_hints=gamepad;}
bool UiGamepadHints(void){return gamepad_hints;}
void UiQueueNavigation(int direction,bool activate){if(direction)state.navigation=direction;state.activate=state.activate||activate;}
UiSnapshot UiInspect(void){return (UiSnapshot){state.battery,state.title_steps,state.score_pulse,state.charge_pulse,state.focus,state.collected};}
