/* Isolated player lab, NOT the campaign or replacement modules. */
#include "raylib.h"
#include "player/player.h"
#include "morph_check.h"
#include <string.h>

static const char *names[]={"Passarinho","Peixinho","Borboleta","Pinguim","Raposa voadora","Dragaozinho"};
static void DrawPure(PlayerApi p,GameState *g,ItemsView *items)
{
    PlayerView before=p.view(p.life.context);p.draw(p.life.context,g,items);
    PlayerView after=p.view(p.life.context);
    MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
}
static void Atlas(PlayerApi p,GameState *g,ItemsView *items)
{
    const char *columns[]={"Asa: fase 0","Intangivel","Escudo","Ima","Dano / hitbox"};
    DrawText("MORPH ADVENTURE | Pessoa 2 | personagens isolados",24,14,22,RAYWHITE);
    DrawText("Mesmas formas do HTML. Verde: colisao 18 | Azul: raio visual 23",24,43,16,LIGHTGRAY);
    for(int col=0;col<5;col++) DrawText(columns[col],178+col*200,78,17,SKYBLUE);
    for(int world=0;world<6;world++) {
        DrawText(names[world],20,138+world*96,16,RAYWHITE);
        for(int col=0;col<5;col++) {
            g->world=world;g->simulation_steps=col==1?7:0;g->mode=GAME_PLAY;
            p.life.reset_world(p.life.context,g,NULL);
            if(col==4) { /* blink at 70/7 (even), without altering lives */
                p.apply_hit(p.life.context);FrameContext f={0};f.normal_steps=1;
                GameInput input={0};for(int i=0;i<5;i++) p.step(p.life.context,g,&f,&input,0);
            }
            p.set_portal_pose(p.life.context,(PortalPose){true,{240+col*200.0f,145+world*96.0f},1.15f,0});
            *items=(ItemsView){0};items->shield=col==2;
            if(col==3){items->boost_count=1;items->boosts[0]=(BoostView){BOOST_MAGNET,7};}
            if(col==1){items->boost_count=1;items->boosts[0]=(BoostView){BOOST_PHASE,7};}
            PlayerSetDebugHitbox(col==4);DrawPure(p,g,items);
        }
    }
}
int main(int argc,char **argv)
{
    const char *capture=argc==3 && strcmp(argv[1],"--atlas")==0?argv[2]:NULL;
    SetConfigFlags(FLAG_MSAA_4X_HINT);InitWindow(capture?1160:960,capture?700:540,"Pessoa 2 - laboratorio isolado");
    if(!IsWindowReady()) return 1;
    SetTargetFPS(60);
    PlayerApi p=PlayerModuleCreate();MORPH_CHECK(p.life.init(p.life.context));
    GameState g={0};g.mode=GAME_PLAY;ItemsView items={0};
    double accumulator=0;bool pending=false,debug=false;int rendered=0;
    while(!WindowShouldClose()) {
        if(!capture) {
            if(!IsWindowFocused()) {g.mode=GAME_PAUSED;accumulator=0;pending=false;}
            if(IsKeyPressed(KEY_P)){g.mode=g.mode==GAME_PLAY?GAME_PAUSED:GAME_PLAY;accumulator=0;pending=false;}
            if(IsKeyPressed(KEY_H)){debug=!debug;PlayerSetDebugHitbox(debug);}
            for(int i=0;i<6;i++) if(IsKeyPressed(KEY_ONE+i)){g.world=i;p.life.reset_world(p.life.context,&g,NULL);}
            if(IsKeyPressed(KEY_R)) p.life.reset_world(p.life.context,&g,NULL);
            if(g.mode==GAME_PLAY) {
                pending=pending||IsKeyPressed(KEY_SPACE)||IsKeyPressed(KEY_W)||IsKeyPressed(KEY_UP)||IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                double seconds=GetFrameTime();accumulator+=seconds>0.08?0.08:seconds;
                while(accumulator>=MORPH_STEP_SECONDS) {
                    FrameContext f={0};f.normal_steps=1;f.world_steps=1;f.normal_seconds=1.0f/60;
                    GameInput input={0};input.impulse_pressed=pending;pending=false;g.simulation_steps++;
                    p.step(p.life.context,&g,&f,&input,0);
                    PlayerView v=p.view(p.life.context);if(v.touching_floor&&!v.invulnerable)p.apply_hit(p.life.context);
                    accumulator-=MORPH_STEP_SECONDS;
                }
            }
        }
        BeginDrawing();ClearBackground((Color){23,38,56,255});
        if(capture) Atlas(p,&g,&items);
        else {
            DrawLine(0,510,960,510,SKYBLUE);DrawPure(p,&g,&items);
            DrawText("LABORATORIO - sem obstaculos, coleta ou campanha",18,12,18,RAYWHITE);
            DrawText("Espaco/W/cima/clique: voo | P: pausa | 1..6: personagem | H: hitbox | R: reset",18,36,16,LIGHTGRAY);
            DrawText(names[g.world],650,80,22,SKYBLUE);
            if(g.mode==GAME_PAUSED) DrawText("PAUSADO",400,250,24,RAYWHITE);
        }
        EndDrawing();rendered++;
        if(capture && rendered==3){TakeScreenshot(capture);break;}
    }
    p.life.shutdown(p.life.context);CloseWindow();return 0;
}
