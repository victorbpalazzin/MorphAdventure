/* Real modules + real coordinator. --capture uses declared progression fixtures;
 * interactive mode is an integration laboratory, not a change to production main. */
#include "raylib.h"
#include "rlgl.h"
#include "core/game.h"
#include "core/viewport.h"
#include "player/player.h"
#include "worlds/worlds.h"
#include "obstacles/obstacles.h"
#include "items/items_internal.h"
#include "ui/ui.h"
#include "effects/effects.h"
#include "morph_check.h"
#include <string.h>
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
    Capture("01-inicio");Command(COMMAND_START);SetEnergy(2);
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
        SetEnergy(GameEnergyRequired(world));game.state.distance=game.state.minimum_distance;
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
        }else MORPH_CHECK(game.state.mode==GAME_WON);
    }
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
    Command(COMMAND_RETRY);MORPH_CHECK(game.state.world==0 && game.state.lives==3 && game.state.score==0 && EffectsInspect().particles==0);
    puts("PASS ui-flow: all real modules, six continuous portals, real defeat/retry, pause, pure draw and screenshots at three window sizes; energy/distance use explicit test setup");
}
int main(int argc,char **argv)
{
    bool capture=argc==3 && strcmp(argv[1],"--capture")==0;
    if(capture){directory=argv[2];}SetConfigFlags(FLAG_WINDOW_RESIZABLE|(capture?FLAG_WINDOW_HIDDEN:0));
    InitWindow(960,540,"Morph Adventure - laboratorio de integracao");MORPH_CHECK(IsWindowReady());SetExitKey(KEY_NULL);SetWindowMinSize(480,270);
    canvas=LoadRenderTexture(960,540);MORPH_CHECK(IsRenderTextureValid(canvas));SetTextureFilter(canvas.texture,TEXTURE_FILTER_BILINEAR);
    GameModules modules=MorphCreateModules();MORPH_CHECK(GameInit(&game,&modules) && game.ready);
    if(capture)Validate();else{
        SetTargetFPS(60);
        while(!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)){
            GameInput input={0};input.focused=IsWindowFocused() && !IsWindowMinimized();
            input.impulse_pressed=IsKeyPressed(KEY_SPACE)||IsKeyPressed(KEY_UP)||IsKeyPressed(KEY_W);
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
