#include "player/player.h"
#include "morph_check.h"
#include <string.h>

static GameState game;
static FrameContext frame;
static PlayerApi player;
static void Reset(void)
{
    game=(GameState){0};game.mode=GAME_PLAY;game.lives=2;game.score=91;
    frame=(FrameContext){0};frame.normal_steps=1;frame.normal_seconds=1.0f/60;
    frame.world_steps=1;frame.world_scale=1;
    player.life.reset_world(player.life.context,&game,NULL);
}
static PlayerView Tick(bool impulse,float force)
{
    GameInput input={0};input.impulse_pressed=impulse;
    GameState before=game;
    player.step(player.life.context,&game,&frame,&input,force);
    MORPH_CHECK(memcmp(&game,&before,sizeof(game))==0);
    return player.view(player.life.context);
}
static void CheckPhysics(void)
{
    Reset();PlayerView v=player.view(player.life.context);
    MORPH_CHECK(v.position.x==180 && v.position.y==245 && v.scale==1);
    MORPH_CHECK(v.visual_radius==23 && v.collision_radius==18 && !v.invulnerable);
    v=Tick(false,0);MORPH_CHECK_NEAR(v.position.y,245.34,0.0001);MORPH_CHECK_NEAR(v.vertical_velocity,0.34,0.00001);
    Reset();v=Tick(true,0);MORPH_CHECK_NEAR(v.vertical_velocity,-6.06,0.00001);MORPH_CHECK_NEAR(v.position.y,238.94,0.0001);
    v=Tick(false,0);MORPH_CHECK_NEAR(v.vertical_velocity,-5.72,0.00001); /* not repeated */
    Reset();for(int i=0;i<12;i++) v=Tick(i==0,0);
    /* Closed-form semi-implicit Euler sum: y=245-6.4*n+.34*n*(n+1)/2. */
    MORPH_CHECK_NEAR(v.position.y,194.72,0.001);
    Reset();for(int i=0;i<22;i++) v=Tick(false,0);
    MORPH_CHECK_NEAR(v.vertical_velocity,7.2,0.00001);
    for(int i=0;i<100;i++) v=Tick(false,0);
    MORPH_CHECK(v.position.y==487 && v.touching_floor && !v.invulnerable);
    player.apply_hit(player.life.context);v=player.view(player.life.context);
    MORPH_CHECK_NEAR(v.vertical_velocity,-4.5,0.00001);MORPH_CHECK(v.invulnerable);
    v=Tick(false,0);MORPH_CHECK(v.position.y<487 && !v.touching_floor);
    Reset();for(int i=0;i<100;i++) v=Tick(true,0);
    MORPH_CHECK(v.position.y==42 && v.vertical_velocity==1 && !v.touching_floor);
    Reset();v=Tick(false,0.105f);MORPH_CHECK_NEAR(v.vertical_velocity,0.445,0.00001);
    Reset();v=Tick(false,-0.105f);MORPH_CHECK_NEAR(v.vertical_velocity,0.235,0.00001);
    Reset();frame.world_scale=frame.world_steps=0.68f;v=Tick(true,0.105f);
    MORPH_CHECK_NEAR(v.vertical_velocity,-5.955,0.00001);
}
static void CheckControlAndDamage(void)
{
    Reset();player.apply_hit(player.life.context);
    for(int i=0;i<74;i++) MORPH_CHECK(Tick(false,0).invulnerable);
    MORPH_CHECK(!Tick(false,0).invulnerable);
    Reset();player.apply_hit(player.life.context);PlayerView before=player.view(player.life.context);
    game.mode=GAME_PAUSED;for(int i=0;i<100;i++) Tick(true,0.1f);
    PlayerView after=player.view(player.life.context);MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
    game.mode=GAME_PLAY;for(int i=0;i<74;i++) MORPH_CHECK(Tick(false,0).invulnerable);
    MORPH_CHECK(!Tick(false,0).invulnerable);
    Reset();PortalPose pose={true,{700,270},0.2f,1.2f};
    player.set_portal_pose(player.life.context,pose);before=player.view(player.life.context);
    after=Tick(true,0.1f);MORPH_CHECK(memcmp(&before,&after,sizeof(before))==0);
    MORPH_CHECK(after.position.x==700 && after.scale==0.2f && after.rotation==1.2f);
    game.world=1;player.life.reset_world(player.life.context,&game,NULL);
    player.set_portal_pose(player.life.context,(PortalPose){0});after=player.view(player.life.context);
    MORPH_CHECK(after.position.x==180 && after.position.y==245 && after.scale==1 && !after.invulnerable);
    GameInput input={0};input.pointer_pressed=true;input.pointer_inside=true;
    player.step(player.life.context,&game,&frame,&input,0);
    MORPH_CHECK(player.view(player.life.context).vertical_velocity>0); /* core owns pointer conversion */
}
static void CheckFrameRates(void)
{
    const int rates[]={30,60,120,144};PlayerView expected={0};
    for(size_t r=0;r<sizeof(rates)/sizeof(rates[0]);r++) {
        Reset();double accumulator=0;int steps=0;
        for(int render=0;render<rates[r]*4;render++) {
            accumulator+=1.0/rates[r];
            while(accumulator+1e-10>=MORPH_STEP_SECONDS) {
                Tick(steps%28==0,steps<120?0.065f:-0.08f);steps++;
                accumulator-=MORPH_STEP_SECONDS;
            }
        }
        MORPH_CHECK(steps==240);
        PlayerView actual=player.view(player.life.context);
        if(r==0) expected=actual;
        MORPH_CHECK_NEAR(actual.position.y,expected.position.y,0.00001);
        MORPH_CHECK_NEAR(actual.vertical_velocity,expected.vertical_velocity,0.00001);
    }
}
int main(void)
{
    player=PlayerModuleCreate();MORPH_CHECK(player.life.init(player.life.context));
    CheckPhysics();CheckControlAndDamage();CheckFrameRates();
    player.life.shutdown(player.life.context);player.life.shutdown(player.life.context);
    puts("PASS player-physics: trajectory, input edge, bounds, damage, 75 steps, pause, slow, portal, 30/60/120/144 Hz.");
    return 0;
}
