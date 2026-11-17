/* Real worlds + real player; no fake campaign/obstacles/items/UI modules. */
#include "raylib.h"
#include "core/viewport.h"
#include "worlds/worlds.h"
#include "player/player.h"
#include "morph_check.h"
#include <stdio.h>
#include <string.h>

double WorldsTestCpuSeconds(void);
static WorldsApi worlds;
static PlayerApi player;
static GameState game;
static RenderTexture2D canvas;
static const char *names[]={"Ar","Agua","Floresta","Gelo","Terra","Fogo"};
static void Reset(int world)
{
    game.world=world;game.scroll=600;game.distance=100;game.minimum_distance=100;
    game.mode=GAME_PLAY;game.simulation_steps=180;
    worlds.life.reset_world(worlds.life.context,&game,NULL);
    player.life.reset_world(player.life.context,&game,NULL);
}
static void Advance(float scale,int count)
{
    FrameContext frame={0};frame.world_steps=scale;frame.normal_steps=1;
    frame.world_scale=scale;frame.normal_seconds=1.0f/60;frame.world_seconds=scale/60;
    GameState before=game;
    for(int i=0;i<count;i++)worlds.step(worlds.life.context,&game,&frame);
    MORPH_CHECK(memcmp(&before,&game,sizeof(game))==0);
}
static Image Render(bool with_player,bool sentinel)
{
    WorldsDiagnostics before=WorldsInspect();GameState state_before=game;
    BeginTextureMode(canvas);ClearBackground(MAGENTA);
    worlds.draw(worlds.life.context,&game);
    if(with_player){ItemsView items={0};player.draw(player.life.context,&game,&items);}
    if(sentinel)DrawRectangle(4,4,8,8,MAGENTA);
    EndTextureMode();
    WorldsDiagnostics after=WorldsInspect();
    MORPH_CHECK(before.visual_steps==after.visual_steps && before.cache_builds==after.cache_builds);
    MORPH_CHECK(memcmp(&game,&state_before,sizeof(game))==0);
    Image result=LoadImageFromTexture(canvas.texture);MORPH_CHECK(IsImageValid(result));
    ImageFlipVertical(&result);
    if(sentinel){Color c=GetImageColor(result,7,7);MORPH_CHECK(c.r==MAGENTA.r && c.g==MAGENTA.g && c.b==MAGENTA.b);}
    return result;
}
static uint64_t HashImage(Image image)
{
    Color *pixels=LoadImageColors(image);MORPH_CHECK(pixels);
    uint64_t hash=UINT64_C(1469598103934665603);
    for(int i=0;i<image.width*image.height;i++) {
        hash^=((uint32_t)pixels[i].r<<24)|((uint32_t)pixels[i].g<<16)|((uint32_t)pixels[i].b<<8)|pixels[i].a;
        hash*=UINT64_C(1099511628211);
    }
    UnloadImageColors(pixels);return hash;
}
static uint64_t Snapshot(void)
{ Image image=Render(false,false);uint64_t hash=HashImage(image);UnloadImage(image);return hash; }
static void Save(Image image,const char *dir,int world,const char *suffix)
{
    char path[2048];snprintf(path,sizeof(path),"%s/%02d-%s-%s.png",dir,world+1,names[world],suffix);
    MORPH_CHECK(ExportImage(image,path));
}
static void Present(bool with_player)
{
    GameViewport v=ViewportFit(GetScreenWidth(),GetScreenHeight());
    BeginDrawing();
    BeginTextureMode(canvas);ClearBackground(BLACK);worlds.draw(worlds.life.context,&game);
    if(with_player){ItemsView items={0};player.draw(player.life.context,&game,&items);}
    EndTextureMode();ClearBackground(BLACK);
    DrawTexturePro(canvas.texture,(Rectangle){0,0,960,-540},(Rectangle){v.x,v.y,v.width,v.height},(Vector2){0},0,WHITE);
    EndDrawing();
}
static void Validate(const char *directory)
{
    uint64_t hashes[6];unsigned ids[3];WorldsDiagnostics start=WorldsInspect();
    MORPH_CHECK(start.live_targets==3);
    for(int i=0;i<3;i++)ids[i]=start.target_ids[i];
    MORPH_CHECK(!WorldsPalette(-1) && !WorldsPalette(6));
    for(int w=0;w<6;w++) {
        Reset(w);MORPH_CHECK(WorldsPalette(w));
        Advance(.68f,100);MORPH_CHECK_NEAR(WorldsInspect().visual_steps,68,0.0001);
        game.mode=GAME_PAUSED;double frozen=WorldsInspect().visual_steps;
        Advance(1,100);MORPH_CHECK(WorldsInspect().visual_steps==frozen);
        game.mode=GAME_PLAY;Reset(w);Advance(1,180);
        hashes[w]=Snapshot();MORPH_CHECK(hashes[w]==Snapshot());
        Image image=Render(true,true);UnloadImage(image); /* sentinel only in test */
        image=Render(true,false);Save(image,directory,w,"personagem");UnloadImage(image);
        image=Render(false,false);Save(image,directory,w,"cenario");UnloadImage(image);
        /* Distance is already 100%; parallax follows scroll, never distance. */
        game.scroll+=120;MORPH_CHECK(Snapshot()!=hashes[w]);game.scroll=600;
        Advance(1,30);MORPH_CHECK(Snapshot()!=hashes[w]);
        image=Render(true,false);Save(image,directory,w,"fase2");UnloadImage(image);
        WorldsSetEnvironment((WorldVisualEnvironment){1,-1,false});
        image=Render(false,false);Save(image,directory,w,"ambiente");UnloadImage(image);
        Reset(w);Advance(1,180);MORPH_CHECK(Snapshot()==hashes[w]);
        WorldsDiagnostics status=WorldsInspect();
        MORPH_CHECK(status.live_targets==3);
        for(int i=0;i<3;i++)MORPH_CHECK(status.target_ids[i]==ids[i]);
        for(int old=0;old<w;old++)MORPH_CHECK(hashes[old]!=hashes[w]);

        /* Includes CPU driver submission; not a GPU-only timing or gameplay FPS. */
        SetTargetFPS(0);double wall=GetTime(),cpu=WorldsTestCpuSeconds();
        for(int i=0;i<120;i++){Advance(1,1);game.scroll+=4;Present(true);}
        wall=GetTime()-wall;cpu=WorldsTestCpuSeconds()-cpu;
        printf("MEASURE world=%s frames=120 wall_s=%.4f cpu_process_ms_per_frame=%.4f observed_fps=%.2f\n",
               names[w],wall,cpu*1000/120,120/wall);
    }
    /* Repeated transitions reuse the same GPU objects and deterministic geometry. */
    for(int pass=0;pass<3;pass++)for(int w=0;w<6;w++) {
        Reset(w);Advance(1,180);MORPH_CHECK(Snapshot()==hashes[w]);
        WorldsDiagnostics d=WorldsInspect();
        for(int i=0;i<3;i++)MORPH_CHECK(d.target_ids[i]==ids[i]);
    }
    /* Non-native window aspect ratios must not alter logical drawing. */
    const int sizes[][2]={{1100,700},{800,600},{960,540}};
    for(int i=0;i<3;i++){SetWindowSize(sizes[i][0],sizes[i][1]);Present(true);MORPH_CHECK(Snapshot()==hashes[5]);}
    worlds.life.shutdown(worlds.life.context);MORPH_CHECK(WorldsInspect().live_targets==0);
    worlds.life.shutdown(worlds.life.context);MORPH_CHECK(WorldsInspect().live_targets==0);
    MORPH_CHECK(worlds.life.init(worlds.life.context));Reset(0);Advance(1,180);
    MORPH_CHECK(Snapshot()==hashes[0]);
    puts("PASS worlds-render: six scenes, scaled clock, pause, pure draw, reset, scrolling at 100%, resize, bounded cache and shutdown/reinit.");
}
int main(int argc,char **argv)
{
    bool validate=argc==3 && strcmp(argv[1],"--validate")==0;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);InitWindow(960,540,"Pessoa 3 - laboratorio visual");
    if(!IsWindowReady())return 1;
    SetTargetFPS(60);canvas=LoadRenderTexture(960,540);MORPH_CHECK(IsRenderTextureValid(canvas));
    worlds=WorldsModuleCreate();player=PlayerModuleCreate();
    MORPH_CHECK(worlds.life.init(worlds.life.context));MORPH_CHECK(player.life.init(player.life.context));
    Reset(0);
    if(validate) Validate(argv[2]);
    else {
        bool slow=false,environment=false;double accumulator=0;
        while(!WindowShouldClose()) {
            if(IsKeyPressed(KEY_P)){game.mode=game.mode==GAME_PAUSED?GAME_PLAY:GAME_PAUSED;accumulator=0;}
            if(!IsWindowFocused()){game.mode=GAME_PAUSED;accumulator=0;}
            if(IsKeyPressed(KEY_S))slow=!slow;
            if(IsKeyPressed(KEY_E)){environment=!environment;WorldsSetEnvironment((WorldVisualEnvironment){environment?1:0,1,false});}
            for(int w=0;w<6;w++)if(IsKeyPressed(KEY_ONE+w)){Reset(w);environment=false;accumulator=0;}
            if(game.mode==GAME_PLAY) {
                accumulator+=fmin((double)GetFrameTime(),.08);
                while(accumulator>=MORPH_STEP_SECONDS) {
                    float scale=slow?.68f:1;Advance(scale,1);game.scroll+=4.2f*scale;game.simulation_steps++;
                    accumulator-=MORPH_STEP_SECONDS;
                }
            }
            Present(true);
        }
    }
    player.life.shutdown(player.life.context);worlds.life.shutdown(worlds.life.context);
    UnloadRenderTexture(canvas);CloseWindow();return 0;
}
