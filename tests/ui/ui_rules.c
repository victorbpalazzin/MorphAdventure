#include "ui/ui.h"
#include "effects/effects.h"
#include "core/game.h"
#include "morph_check.h"
#include <string.h>
static void CheckInput(void)
{
    UiApi api=UiModuleCreate();MORPH_CHECK(api.life.init(api.life.context));
    GameState g={.mode=GAME_INTRO};api.life.reset_world(api.life.context,&g,NULL);
    GameInput in={.focused=true,.pointer_pressed=true,.pointer_inside=true,.pointer={480,380}};
    GameState before=g;UiDecision d=api.input(api.life.context,&g,&in);
    MORPH_CHECK(d.command==COMMAND_START && d.consumes_pointer && memcmp(&g,&before,sizeof(g))==0);
    in.pointer_inside=false;MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_NONE);
    g.mode=GAME_PLAY;in.pointer_inside=true;in.pointer=(GameVec2){915,40};d=api.input(api.life.context,&g,&in);
    MORPH_CHECK(d.command==COMMAND_TOGGLE_PAUSE && d.consumes_pointer);
    in.pointer=(GameVec2){480,300};d=api.input(api.life.context,&g,&in);MORPH_CHECK(d.command==COMMAND_NONE && !d.consumes_pointer);
    g.mode=GAME_PAUSED;in.pointer_pressed=false;UiQueueNavigation(1,true);
    MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_RESTART);
    UiQueueNavigation(-1,true);MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_RESUME);
    MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_NONE);
    g.mode=GAME_LOST;in.impulse_pressed=true;MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_RETRY);
    g.mode=GAME_WON;MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_RETRY);
    in.focused=false;UiQueueNavigation(1,true);MORPH_CHECK(api.input(api.life.context,&g,&in).command==COMMAND_NONE);
    g.mode=GAME_PLAY;g.world=5;api.life.reset_world(api.life.context,&g,NULL);
    ItemsView v={.collected=23};FrameContext f={.ui_seconds=1.0f/60};
    for(int i=0;i<120;i++)api.step(api.life.context,&g,&f,&v);
    MORPH_CHECK(UiInspect().battery<1);MORPH_CHECK_NEAR(UiInspect().battery,23.0/24,.0002);
    v.collected=24;g.score=35;api.step(api.life.context,&g,&f,&v);
    MORPH_CHECK(UiInspect().charge_pulse==35 && UiInspect().score_pulse==17);
    g.mode=GAME_PAUSED;UiSnapshot snap=UiInspect();api.step(api.life.context,&g,&f,&v);
    MORPH_CHECK(UiInspect().battery==snap.battery && UiInspect().title_steps==snap.title_steps);
    g.mode=GAME_PLAY;for(int i=0;i<120;i++)api.step(api.life.context,&g,&f,&v);
    MORPH_CHECK(UiInspect().battery==1);api.life.shutdown(api.life.context);api.life.shutdown(api.life.context);
}
static void CheckPortal(void)
{
    EffectsApi api=EffectsModuleCreate();MORPH_CHECK(api.life.init(api.life.context));
    FrameContext f={.normal_steps=1,.world_steps=.68f};GameState g={.mode=GAME_PORTAL};
    PlayerView player={.position={180,245},.scale=1};api.life.reset_world(api.life.context,&g,NULL);
    for(int world=0;world<6;world++){
        g.world=world;g.mode=GAME_PORTAL;api.begin_portal(api.life.context,&g,player);
        for(int tick=1;tick<=149;tick++){
            PortalSignal signal=api.step_portal(api.life.context,&g,&f);
            MORPH_CHECK(signal==(tick==149?PORTAL_SIGNAL_ENTERED:PORTAL_SIGNAL_NONE));
            if(tick==80){
                EffectsSnapshot before=EffectsInspect();g.mode=GAME_PAUSED;
                for(int i=0;i<100;i++){api.step_particles(api.life.context,&g,&f);MORPH_CHECK(api.step_portal(api.life.context,&g,&f)==PORTAL_SIGNAL_NONE);}
                MORPH_CHECK(EffectsInspect().age==before.age && EffectsInspect().entry==before.entry);g.mode=GAME_PORTAL;
                api.begin_portal(api.life.context,&g,player);MORPH_CHECK(EffectsInspect().age==before.age);
            }
        }
        PortalPose pose=api.portal_pose(api.life.context);MORPH_CHECK_NEAR(pose.position.x,870,.001);MORPH_CHECK_NEAR(pose.scale,.08,.001);
        MORPH_CHECK(api.step_portal(api.life.context,&g,&f)==PORTAL_SIGNAL_NONE);g.mode=GAME_TRANSITION;
        for(int i=1;i<=24;i++)MORPH_CHECK(api.step_portal(api.life.context,&g,&f)==(i==24?PORTAL_SIGNAL_BLACKOUT:PORTAL_SIGNAL_NONE));
        g.mode=GAME_PAUSED;MORPH_CHECK(api.step_portal(api.life.context,&g,&f)==PORTAL_SIGNAL_NONE);MORPH_CHECK(EffectsInspect().transition==24);
        g.mode=GAME_TRANSITION;g.world=world+1;api.switch_world_under_fade(api.life.context,&g,NULL);
        MORPH_CHECK(!api.portal_pose(api.life.context).overrides_player && EffectsInspect().transition==24);
        for(int i=25;i<=55;i++)MORPH_CHECK(api.step_portal(api.life.context,&g,&f)==(i==55?PORTAL_SIGNAL_FADE_DONE:PORTAL_SIGNAL_NONE));
        MORPH_CHECK(api.step_portal(api.life.context,&g,&f)==PORTAL_SIGNAL_NONE);
    }
    g.world=0;g.mode=GAME_PLAY;api.life.reset_world(api.life.context,&g,NULL);
    GameEvent event={.type=EVENT_DAMAGE,.position={180,245}};
    for(int i=0;i<30;i++)api.event(api.life.context,&event);
    MORPH_CHECK(EffectsInspect().particles==EFFECTS_PARTICLE_CAPACITY);
    for(int i=0;i<36;i++)api.step_particles(api.life.context,&g,&f);
    MORPH_CHECK(EffectsInspect().particles==0 && EffectsInspect().damage_flash==0);
    api.event(api.life.context,&event);api.life.reset_world(api.life.context,&g,NULL);MORPH_CHECK(EffectsInspect().particles==0);
    api.life.shutdown(api.life.context);api.life.shutdown(api.life.context);
}
int main(void){CheckInput();CheckPortal();puts("PASS ui-rules: commands, focus, battery, six continuous portals, one-shot signals, frozen pause and bounded particles");return 0;}
