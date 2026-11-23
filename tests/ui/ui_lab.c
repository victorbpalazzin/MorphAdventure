/* Real modules + real coordinator. --capture uses declared progression fixtures;
 * interactive mode is an integration laboratory, not a change to production main. */
#include "raylib.h"
#include "rlgl.h"
#include "core/game.h"
#include "boss/boss.h"
#include "core/viewport.h"
#include "player/player.h"
#include "worlds/worlds.h"
#include "obstacles/obstacles.h"
#include "items/items_internal.h"
#include "ui/ui.h"
#include "effects/effects.h"
#include "morph_check.h"
#include <string.h>
#include <math.h>
static Game game;
GameModules MorphCreateModules(void);
static RenderTexture2D canvas;
static const char *directory;
static void Command(GameCommand command)
{GameInput input={.focused=true,.command=command};GameAdvance(&game,0,input);}
static void Tick(void){GameInput input={.focused=true};GameAdvance(&game,MORPH_STEP_SECONDS,input);MORPH_CHECK(game.state.mode!=GAME_ERROR);}
static void Render(void)
{
    GameState before=game.state;EffectsSnapshot fx=EffectsInspect();UiSnapshot ui=UiInspect();
    BeginTextureMode(canvas);ClearBackground(BLACK);GameDraw(&game);EndTextureMode();
    MORPH_CHECK(memcmp(&before,&game.state,sizeof(before))==0);
    MORPH_CHECK(EffectsInspect().clock==fx.clock && EffectsInspect().transition==fx.transition && UiInspect().battery==ui.battery);
}
static void Present(void)
{
    Render();GameViewport v=ViewportFit(GetScreenWidth(),GetScreenHeight());
    BeginDrawing();ClearBackground(BLACK);DrawTexturePro(canvas.texture,(Rectangle){0,0,960,-540},(Rectangle){v.x,v.y,v.width,v.height},(Vector2){0},0,WHITE);EndDrawing();
}
static void Capture(const char *name)
{
    Render();Image img=LoadImageFromTexture(canvas.texture);MORPH_CHECK(IsImageValid(img));ImageFlipVertical(&img);
    char path[2048];snprintf(path,sizeof(path),"%s/%s.png",directory,name);MORPH_CHECK(ExportImage(img,path));UnloadImage(img);
}
static void SetEnergy(unsigned count)
{
    /* Controlled setup for visual and transition tests, never used interactively. */
    ItemsState *s=game.modules.items.life.context;s->collected=count;
    for(unsigned i=0;i<s->required;i++)s->collected_ids[i]=i<count;
    FrameContext f={.normal_steps=1,.ui_seconds=1.0f/60};ItemsView v=game.modules.items.view(s);
    for(int i=0;i<60;i++)game.modules.ui.step(game.modules.ui.life.context,&game.state,&f,&v);
}
static void Validate(void)
{
    GameState intro=game.state;
    for(int i=0;i<90;i++)Tick();
    MORPH_CHECK(memcmp(&intro,&game.state,sizeof(intro))==0);
    Capture("01-inicio");
    GameInput start={.focused=true,.pointer_inside=true,.pointer_pressed=true,.pointer={765,460}};
    GameAdvance(&game,MORPH_STEP_SECONDS,start);MORPH_CHECK(game.state.mode==GAME_PLAY);
    SetEnergy(2);
    ItemsState *items=game.modules.items.life.context;items->boost_count=2;
    items->boosts[0]=(ActiveItemBoost){BOOST_SHIELD,-1};items->boosts[1]=(ActiveItemBoost){BOOST_DOUBLE,7};
    Capture("02-hud-parcial");
    /* A pause button click carries pointer_pressed, but must not flap. */
    PlayerView before=game.modules.player.view(game.modules.player.life.context);
    GameInput click={.focused=true,.pointer_pressed=true,.pointer_inside=true,.pointer={914,40}};
    GameAdvance(&game,MORPH_STEP_SECONDS,click);MORPH_CHECK(game.state.mode==GAME_PAUSED);
    MORPH_CHECK(game.modules.player.view(game.modules.player.life.context).vertical_velocity==before.vertical_velocity);
    Capture("03-pausa");Command(COMMAND_RESUME);
    game.state.lives=1;for(int i=0;i<300 && game.state.mode!=GAME_LOST;i++)Tick();
    MORPH_CHECK(game.state.mode==GAME_LOST);Capture("04-derrota");Command(COMMAND_RETRY);
    MORPH_CHECK(game.state.lives==3 && game.state.score==0);
    for(int world=0;world<6;world++){
        MORPH_CHECK(game.state.world==world && game.state.mode==GAME_PLAY);
        SetEnergy(GameEnergyRequired(world));
        if(world==0)Capture("05-hud-completo");
        Tick();MORPH_CHECK(game.state.mode==GAME_PORTAL);
        for(int i=1;i<=149;i++){
            Tick();
            if(i==40 && world==0){
                Command(COMMAND_TOGGLE_PAUSE);EffectsSnapshot fx=EffectsInspect();
                for(int j=0;j<100;j++)Tick();
                MORPH_CHECK(EffectsInspect().age==fx.age && EffectsInspect().clock==fx.clock);
                Capture("06-pausa-portal");Command(COMMAND_RESUME);
            }
            if(i==80){char name[64];snprintf(name,sizeof(name),"portal-%d",world);Capture(name);}
        }
        MORPH_CHECK(game.state.mode==GAME_TRANSITION && game.state.score==(uint64_t)(world+1)*100);
        for(int i=1;i<=55 && game.state.mode==GAME_TRANSITION;i++){
            Tick();
            if(i==12 && world==0){
                Capture("07-transicao");Command(COMMAND_TOGGLE_PAUSE);float t=EffectsInspect().transition;
                for(int j=0;j<40;j++){Tick();}MORPH_CHECK(EffectsInspect().transition==t);
                Capture("08-pausa-transicao");Command(COMMAND_RESUME);
            }
        }
        MORPH_CHECK(game.state.lives==3);
        if(world<5){
            MORPH_CHECK(game.state.world==world+1 && game.state.mode==GAME_PLAY);
            MORPH_CHECK(!EffectsInspect().pose.overrides_player && game.modules.items.view(game.modules.items.life.context).collected==0);
            PlayerView p=game.modules.player.view(game.modules.player.life.context);
            MORPH_CHECK_NEAR(p.position.x,180,.001);MORPH_CHECK_NEAR(p.position.y,245,.001);
        }else MORPH_CHECK(game.state.mode==GAME_BOSS);
    }
    Capture("boss-intro");
    for(int i=0;i<180;i++)Tick();
    MORPH_CHECK(game.state.boss.stage==BOSS_FIGHT);
    Capture("boss-fase-1");
    /* Controlled charges verify core transitions, not collection difficulty. */
    for(unsigned shot=0;shot<3;shot++){
        game.state.boss.energy.collected=game.state.boss.energy.required;
        GameInput fire={.focused=true,.attack_pressed=true};GameAdvance(&game,MORPH_STEP_SECONDS,fire);
        MORPH_CHECK(game.state.boss.beam_active);
        for(int i=0;i<40;i++)Tick();
        MORPH_CHECK(game.state.boss.segments==2-shot);
        char label[64];snprintf(label,sizeof(label),"boss-raio-%u",shot+1);Capture(label);
        if(shot<2){for(int i=0;i<45;i++)Tick();}
    }
    MORPH_CHECK(game.state.mode==GAME_BOSS);
    for(int i=0;i<220 && game.state.mode==GAME_BOSS;i++)Tick();
    MORPH_CHECK(game.state.mode==GAME_WON);
    GameState won=game.state;for(int i=0;i<60;i++)Tick();
    MORPH_CHECK(memcmp(&won,&game.state,sizeof(won))==0);
    Capture("09-vitoria");
    const int sizes[][2]={{480,270},{800,600},{1280,720}};
    for(int i=0;i<3;i++){
        SetWindowSize(sizes[i][0],sizes[i][1]);
        for(int settle=0;settle<4;settle++)Present();
        Render();GameViewport view=ViewportFit(GetScreenWidth(),GetScreenHeight());
        BeginDrawing();ClearBackground(BLACK);
        DrawTexturePro(canvas.texture,(Rectangle){0,0,960,-540},(Rectangle){view.x,view.y,view.width,view.height},(Vector2){0},0,WHITE);
        rlDrawRenderBatchActive(); /* Read before swapping: hidden-window back buffers are undefined afterward. */
        Image screen=LoadImageFromScreen();EndDrawing();
        MORPH_CHECK(IsImageValid(screen) && screen.width==sizes[i][0] && screen.height==sizes[i][1]);
        Color center=GetImageColor(screen,screen.width/2,screen.height/2);MORPH_CHECK(center.r+center.g+center.b>20);
        char path[2048];
        snprintf(path,sizeof(path),"%s/janela-%dx%d.png",directory,sizes[i][0],sizes[i][1]);
        MORPH_CHECK(ExportImage(screen,path));UnloadImage(screen);
    }
    GameInput menu={.focused=true,.pointer_inside=true,.pointer_pressed=true,.pointer={765,500}};
    GameAdvance(&game,MORPH_STEP_SECONDS,menu);MORPH_CHECK(game.state.mode==GAME_INTRO);
    Command(COMMAND_START);MORPH_CHECK(game.state.world==0 && game.state.lives==3 && game.state.score==0 && EffectsInspect().particles==0);
    puts("PASS ui-flow: all real modules, six continuous portals, real defeat/retry, pause, pure draw and screenshots at three window sizes; energy uses explicit test setup; progress follows real core");
}
/* Test-only lookahead pilot: ordinary input, no state edits or damage bypass.
 * Evaluate flight targets against observed shots; missed energy may be reoffered. */
static bool Pilot(const BossState *b,PlayerView p)
{
    float best=INFINITY;bool action=false;
    for(int lane=0;lane<19;lane++)for(int first=0;first<2;first++){
        float goal=85+lane*20,y=p.position.y,vy=p.vertical_velocity,score=0;bool collected=false;
        for(int t=1;t<=110;t++){
            bool flap=t==1?first!=0:y>goal+12 && vy>=0;
            if(flap){vy=-6.4f;}vy=fminf(7.2f,vy+.34f);y+=vy;
            if(y<42){y=42;vy=1;}if(y>487){score+=1000000;break;}
            for(unsigned j=0;j<b->projectile_count;j++){
                const BossProjectile *q=&b->projectiles[j];if(q->spent)continue;
                float dx=q->position.x+q->velocity.x*t-p.position.x;
                float dy=q->position.y-y;
                if(fabsf(dx)<35 && fabsf(dy)<37)score+=100000;
            }
            if(b->energy.present && !collected){
                float dx=b->energy.position.x-2.8f*t-p.position.x;
                float dy=b->energy.position.y-y;
                if(dx*dx+dy*dy<38*38){score-=1500;collected=true;}
            }
        }
        float target=b->energy.present?b->energy.position.y:265;
        score+=fabsf(y-target)+fabsf(goal-target)*.3f;
        if(score<best){best=score;action=first!=0;}
    }
    return action;
}
static void ValidateFlight(void)
{
    Command(COMMAND_RESTART);Command(COMMAND_START);
    /* Only the initial arena is prepared. Flight, collisions, pickups and shots
       below run through real production modules without teleport/refilling. */
    game.state.world=5;game.state.mode=GAME_BOSS;game.state.distance=game.state.minimum_distance;
    game.config=game.modules.obstacles.config(game.modules.obstacles.life.context,5);
    game.modules.player.life.reset_world(game.modules.player.life.context,&game.state,game.config);
    game.modules.worlds.life.reset_world(game.modules.worlds.life.context,&game.state,game.config);
    game.modules.items.life.reset_world(game.modules.items.life.context,&game.state,game.config);
    game.modules.effects.life.reset_world(game.modules.effects.life.context,&game.state,game.config);
    BossBegin(&game.state.boss,4);
    unsigned stages=0;int ticks=0;bool crossed=false;
    for(;ticks<14000 && game.state.mode==GAME_BOSS;ticks++){
        const BossState *b=&game.state.boss;
        PlayerView p=game.modules.player.view(game.modules.player.life.context);
        GameInput in={.focused=true,.impulse_pressed=Pilot(b,p),
            .attack_pressed=b->energy.collected==b->energy.required && !b->beam_active};
        GameAdvance(&game,MORPH_STEP_SECONDS,in);
        MORPH_CHECK(game.state.mode!=GAME_ERROR);
        for(unsigned j=0;j<b->projectile_count;j++)if(!crossed && b->projectiles[j].position.x<140){
            Capture("boss-tiro-apos-jogador");crossed=true;
        }
        if(b->stage==BOSS_FIGHT && !b->beam_active && b->energy.present && b->phase>stages){
            stages=b->phase;char name[64];snprintf(name,sizeof(name),"boss-combate-fase-%u",stages);Capture(name);
        }
    }
    printf("boss flight: ticks=%d lives=%d score=%llu phases=%u mode=%s\n",ticks,game.state.lives,(unsigned long long)game.state.score,stages,GameModeName(game.state.mode));
    MORPH_CHECK(game.state.mode==GAME_WON && stages==3 && game.state.lives>0);
    MORPH_CHECK(game.state.score==12*35 && crossed);
}
int main(int argc,char **argv)
{
    bool capture=argc==3 && strcmp(argv[1],"--capture")==0;
    if(capture){directory=argv[2];}SetConfigFlags(FLAG_WINDOW_RESIZABLE|(capture?FLAG_WINDOW_HIDDEN:0));
    InitWindow(960,540,"Morph Adventure - laboratorio de integracao");MORPH_CHECK(IsWindowReady());SetExitKey(KEY_NULL);SetWindowMinSize(480,270);
    canvas=LoadRenderTexture(960,540);MORPH_CHECK(IsRenderTextureValid(canvas));SetTextureFilter(canvas.texture,TEXTURE_FILTER_BILINEAR);
    GameModules modules=MorphCreateModules();MORPH_CHECK(GameInitMode(&game,&modules,false) && game.ready); /* arcade flow; educational flow: tests/edu */
    if(capture){Validate();ValidateFlight();}else{
        SetTargetFPS(60);
        while(!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)){
            GameInput input={0};input.focused=IsWindowFocused() && !IsWindowMinimized();
            input.impulse_pressed=IsKeyPressed(KEY_SPACE)||IsKeyPressed(KEY_UP)||IsKeyPressed(KEY_W);
            input.attack_pressed=IsKeyPressed(KEY_J);
            if(IsKeyPressed(KEY_P))input.command=COMMAND_TOGGLE_PAUSE;
            if(IsKeyPressed(KEY_TAB))UiQueueNavigation(IsKeyDown(KEY_LEFT_SHIFT)?-1:1,false);
            if(IsKeyPressed(KEY_ENTER))UiQueueNavigation(0,true);
            Vector2 mouse=GetMousePosition();GameViewport v=ViewportFit(GetScreenWidth(),GetScreenHeight());
            input.pointer_inside=ViewportToLogical(v,(GameVec2){mouse.x,mouse.y},&input.pointer);input.pointer_pressed=IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            GameAdvance(&game,GetFrameTime(),input);Present();
        }
    }
    GameShutdown(&game);MORPH_CHECK(WorldsInspect().live_targets==0 && EffectsInspect().particles==0);
    GameShutdown(&game);UnloadRenderTexture(canvas);CloseWindow();return 0;
}
