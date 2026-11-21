/* Reuse person 7's test-only coordinator fixture, replacing its items with real code. */
#define main ExistingCoreFixtureMain
#include "../core/core_check.c"
#undef main
#include "items/items_internal.h"
static unsigned hud_collected,event_count;
static void ObserveUi(void *p,const GameState *s,const FrameContext *c,const ItemsView *v)
{(void)p;(void)s;(void)c;hud_collected=v->collected;}
static void ObserveEvent(void *p,const GameEvent *e)
{Event(p,e);if(e->type==EVENT_FRAGMENT_COLLECTED)event_count++;}
static void CheckPhaseIntegration(void)
{
    Game g;Fixture f={0};GameModules m=Modules(&f);m.items=ItemsModuleCreate();
    CHECK(GameInit(&g,&m));Cmd(&g,COMMAND_START);ItemsState *s=m.items.life.context;
    s->powers[0]=(ItemPowerView){1,BOOST_PHASE,{184,245},21};s->power_count=1;Tick(&g);
    s->powers[0]=(ItemPowerView){2,BOOST_SHIELD,{184,245},21};s->power_count=1;Tick(&g);
    f.collision=true;Tick(&g);CHECK(g.state.lives==3 && f.hits==0 && m.items.view(s).shield);
    double remaining=s->boosts[0].seconds;Cmd(&g,COMMAND_TOGGLE_PAUSE);
    for(int i=0;i<600;i++){Tick(&g);}CHECK(s->boosts[0].seconds==remaining);
    Cmd(&g,COMMAND_RESUME);
    for(int i=0;i<416;i++){Tick(&g);CHECK(g.state.lives==3 && f.hits==0);}
    Tick(&g);CHECK(f.hits==0);Tick(&g);CHECK(f.hits==1 && g.state.lives==3 && !m.items.view(s).shield);
    f.player.invulnerable=false;Tick(&g);CHECK(g.state.lives==2);
    GameShutdown(&g);
}
int main(void)
{
    Game g;Fixture f={0};GameModules m=Modules(&f);m.items=ItemsModuleCreate();m.ui.step=ObserveUi;m.effects.event=ObserveEvent;
    CHECK(GameInit(&g,&m));Cmd(&g,COMMAND_START);ItemsState *s=m.items.life.context;
    s->power_count=1;s->powers[0]=(ItemPowerView){1,BOOST_DOUBLE,{184,245},21};Tick(&g);
    CHECK(m.items.view(s).score_multiplier==2);
    s->fragment_count=1;s->fragments[0]=(ItemFragmentView){0,{184,245},23,0,0};f.passed=1;Tick(&g);
    CHECK(g.state.score==90 && hud_collected==1 && event_count==1);
    double seconds=s->boosts[0].seconds;Cmd(&g,COMMAND_TOGGLE_PAUSE);for(int i=0;i<300;i++)Tick(&g);
    CHECK(s->boosts[0].seconds==seconds);Cmd(&g,COMMAND_RESUME);
    s->power_count=1;s->powers[0]=(ItemPowerView){2,BOOST_SHIELD,{184,245},21};Tick(&g);
    f.collision=true;Tick(&g);CHECK(g.state.lives==3 && !m.items.view(s).shield);
    f.player.invulnerable=false;Tick(&g);CHECK(g.state.lives==2);f.collision=false;
    /* Controlled completion fixture: actual portal reward path in coordinator. */
    for(unsigned i=0;i<4;i++){s->collected_ids[i]=true;}s->collected=4;
    f.clear=true;Tick(&g);CHECK(g.state.mode==GAME_PORTAL && g.state.distance==g.state.minimum_distance);
    seconds=s->boosts[0].seconds;f.signal=PORTAL_SIGNAL_ENTERED;Tick(&g);
    CHECK(g.state.score==290 && s->boosts[0].seconds==seconds);
    GameShutdown(&g);CheckPhaseIntegration();puts("PASS items-core: real x2 rewards 20/70/200, shield, pause, same-step HUD/effect notifications");return 0;
}
