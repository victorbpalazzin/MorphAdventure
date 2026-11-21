/* Laboratory of real modules, not the final campaign coordinator. */
#include "raylib.h"
#include "obstacles/obstacles.h"
#include "worlds/worlds.h"
#include "player/player.h"
#include "morph_check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static ObstaclesApi obstacles;
static WorldsApi worlds;
static PlayerApi player;
static GameState game;
static RenderTexture2D canvas;
static void Reset(int w)
{
    memset(&game,0,sizeof(game));game.world=w;game.mode=GAME_PLAY;
    const WorldConfig *cfg=obstacles.config(obstacles.life.context,w);
    obstacles.life.reset_world(obstacles.life.context,&game,cfg);
    worlds.life.reset_world(worlds.life.context,&game,cfg);
    player.life.reset_world(player.life.context,&game,cfg);
}
static void Step(bool impulse,float scale,bool posed)
{
    FrameContext f={0};f.normal_steps=1;f.world_steps=scale;f.world_scale=scale;
    f.normal_seconds=1.0f/60;f.world_seconds=scale/60;f.pace=1;
    f.scroll_delta=obstacles.config(obstacles.life.context,game.world)->base_speed*scale;
    game.world_steps+=scale;game.scroll+=f.scroll_delta;game.simulation_steps++;
    obstacles.step(obstacles.life.context,&game,&f,player.view(player.life.context),true);
    ObstacleEnvironmentView e=ObstaclesEnvironment();
    WorldsSetEnvironment((WorldVisualEnvironment){e.intensity,e.direction,e.warning});
    worlds.step(worlds.life.context,&game,&f);
    if(posed){PassageQuery q=obstacles.passages(obstacles.life.context);
        player.set_portal_pose(player.life.context,(PortalPose){true,{180,q.safe_y(q.context,180,260)},1,0});}
    GameInput input={0};input.impulse_pressed=impulse;
    player.step(player.life.context,&game,&f,&input,obstacles.vertical_force(obstacles.life.context));
    if(!posed && obstacles.touches_player(obstacles.life.context,player.view(player.life.context)))
        player.apply_hit(player.life.context);
    (void)obstacles.take_passed(obstacles.life.context);
}
static void Render(void)
{
    ObstaclesSnapshot before=ObstaclesInspect();GameState old=game;
    BeginTextureMode(canvas);ClearBackground(BLACK);
    worlds.draw(worlds.life.context,&game);obstacles.draw(obstacles.life.context,&game);
    ItemsView items={0};player.draw(player.life.context,&game,&items);
    DrawRectangle(0,0,960,30,(Color){0,0,0,170});
    DrawText("LAB P4 | 1-6 mundos | Espaco voo | P pausa | S slow | D hitboxes | R reinicio",12,8,15,WHITE);
    EndTextureMode();ObstaclesSnapshot after=ObstaclesInspect();
    MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
    MORPH_CHECK(memcmp(&old,&game,sizeof(game))==0);
}
int main(int argc,char **argv)
{
    bool capture=argc==3 && strcmp(argv[1],"--capture")==0;
    if(capture)SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(960,540,"Pessoa 4 - laboratorio de obstaculos");MORPH_CHECK(IsWindowReady());
    canvas=LoadRenderTexture(960,540);MORPH_CHECK(IsRenderTextureValid(canvas));
    obstacles=ObstaclesModuleCreate();worlds=WorldsModuleCreate();player=PlayerModuleCreate();
    MORPH_CHECK(obstacles.life.init(obstacles.life.context));
    MORPH_CHECK(worlds.life.init(worlds.life.context));MORPH_CHECK(player.life.init(player.life.context));
    Reset(0);
    if(capture){
        for(int w=0;w<6;w++){
            Reset(w);
            bool got_warning=false,got_active=false;
            for(int tick=1;tick<=700 && !got_active;tick++){
                Step(false,1,true);ObstaclesSnapshot snap=ObstaclesInspect();
                if(!snap.hazard_count)continue;
                bool warning=snap.hazards[0].age<ObstaclesHazardSettings(w)->warning;
                if((warning && !got_warning && snap.hazards[0].x<900)||(!warning && !got_active)){
                    if(warning)got_warning=true;else got_active=true;
                    Render();Image img=LoadImageFromTexture(canvas.texture);MORPH_CHECK(IsImageValid(img));
                    ImageFlipVertical(&img);char path[2048];
                    snprintf(path,sizeof(path),"%s/%d-%s.png",argv[2],w,warning?"warning":"active");
                    MORPH_CHECK(ExportImage(img,path));UnloadImage(img);
                }
            }
            MORPH_CHECK(got_warning && got_active);
        }
        puts("PASS obstacles-render: six real scenes and characters, warnings, active hazards, pure rendering.");
    }else{
        SetTargetFPS(60);double accumulator=0;bool slow=false,debug=false,pending=false;
        while(!WindowShouldClose()){
            if(IsKeyPressed(KEY_P)){game.mode=game.mode==GAME_PAUSED?GAME_PLAY:GAME_PAUSED;accumulator=0;}
            if(!IsWindowFocused()){game.mode=GAME_PAUSED;accumulator=0;pending=false;}
            if(IsKeyPressed(KEY_S))slow=!slow;
            if(IsKeyPressed(KEY_D)){debug=!debug;ObstaclesSetDebugGeometry(debug);}
            for(int w=0;w<6;w++)if(IsKeyPressed(KEY_ONE+w)){Reset(w);accumulator=0;pending=false;}
            if(IsKeyPressed(KEY_R)){Reset(game.world);accumulator=0;pending=false;}
            if(game.mode==GAME_PLAY){
                pending=pending||IsKeyPressed(KEY_SPACE)||IsKeyPressed(KEY_UP)||IsKeyPressed(KEY_W)||IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                accumulator+=fmin(GetFrameTime(),.08);
                while(accumulator>=1.0/60){Step(pending,slow?.68f:1,false);pending=false;accumulator-=1.0/60;}
            }else pending=false;
            Render();BeginDrawing();ClearBackground(BLACK);
            DrawTextureRec(canvas.texture,(Rectangle){0,0,960,-540},(Vector2){0},WHITE);EndDrawing();
        }
    }
    player.life.shutdown(player.life.context);worlds.life.shutdown(worlds.life.context);
    obstacles.life.shutdown(obstacles.life.context);UnloadRenderTexture(canvas);CloseWindow();return 0;
}
