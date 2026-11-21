#include "worlds_internal.h"
#include <math.h>
#include <string.h>

static WorldScene scene;
static const WorldPalette palettes[MORPH_WORLD_COUNT]={
    {"Ceu encantado",{0x286caaff,0x8fd9e4ff,0xffe1acff},{0x548daeff,0x6caac1ff,0xafcfcdff},0x233c68ff,0xfff0b0ff},
    {"Oceano profundo",{0x076e8cff,0x0e546fff,0x092f4bff},{0x11677bff,0x145469ff,0x1b7180ff},0x082b44ff,0x75ffdcff},
    {"Floresta encantada",{0x163f43ff,0x3d7969ff,0xb3bb72ff},{0x2b625bff,0x24584dff,0x183e3bff},0x112e32ff,0xe3ffa3ff},
    {"Reino congelado",{0x3a4c8dff,0x9fb9deff,0xe3edefff},{0x677eafff,0x8caacbff,0xb8d6e2ff},0x2c436cff,0xb8fbffff},
    {"Deserto ancestral",{0xb66b75ff,0xefac79ff,0xffde9eff},{0xb97770ff,0xc88867ff,0xdfaa74ff},0x613a42ff,0xffe2a1ff},
    {"Reino vulcanico",{0x231c3bff,0x743846ff,0xd17a50ff},{0x573043ff,0x4c2737ff,0x301e32ff},0x201a30ff,0xffbb66ff}
};
const WorldPalette *WorldsPalette(int world)
{ return world>=0 && world<MORPH_WORLD_COUNT?&palettes[world]:NULL; }
static void Shutdown(void *context)
{
    WorldScene *s=context;
    for(int i=0;i<3;i++) {
        if(s->layers[i].id) UnloadRenderTexture(s->layers[i]);
        s->layers[i]=(RenderTexture2D){0};
    }
    s->ready=false;s->world=-1;
}
static bool Init(void *context)
{
    WorldScene *s=context;
    if(s->ready) Shutdown(s);
    memset(s,0,sizeof(*s));s->world=-1;
    if(!IsWindowReady()) return false;
    for(int i=0;i<3;i++) {
        s->layers[i]=LoadRenderTexture(WORLD_LAYER_WIDTH,MORPH_HEIGHT);
        if(!IsRenderTextureValid(s->layers[i])) { Shutdown(s);return false; }
        SetTextureFilter(s->layers[i].texture,TEXTURE_FILTER_BILINEAR);
    }
    s->ready=true;
    return true;
}
static void Reset(void *context,const GameState *state,const WorldConfig *config)
{
    (void)config;WorldScene *s=context;
    s->visual_steps=0;s->environment=(WorldVisualEnvironment){0};
    if(!s->ready || !WorldsPalette(state->world)) return;
    /* Reuse fixed targets, even across world changes/restarts. */
    if(s->world==state->world) return;
    s->world=state->world;
    for(int i=0;i<3;i++) {
        BeginTextureMode(s->layers[i]);ClearBackground(BLANK);
        WorldBuildLayer(s->world,i);EndTextureMode();
    }
    s->cache_builds++;
}
static void Step(void *context,const GameState *state,const FrameContext *frame)
{
    WorldScene *s=context;
    if(!s->ready || (state->mode!=GAME_PLAY && state->mode!=GAME_PORTAL && state->mode!=GAME_TRANSITION && state->mode!=GAME_BOSS)) return;
    if(isfinite(frame->world_steps) && frame->world_steps>0) s->visual_steps+=frame->world_steps;
}
static void Draw(void *context,const GameState *state)
{
    const WorldScene *s=context;
    if(s->ready && s->world==state->world) WorldDrawBackground(s,state);
}
WorldsApi WorldsModuleCreate(void)
{ return (WorldsApi){{&scene,Init,Reset,Shutdown},Step,Draw}; }
void WorldsSetEnvironment(WorldVisualEnvironment environment)
{
    environment.intensity=isfinite(environment.intensity)?fmaxf(0,fminf(1,environment.intensity)):0;
    environment.direction=isfinite(environment.direction)?fmaxf(-1,fminf(1,environment.direction)):0;
    scene.environment=environment;
}
WorldsDiagnostics WorldsInspect(void)
{
    WorldsDiagnostics d={0};d.ready=scene.ready;d.world=scene.world;
    d.visual_steps=scene.visual_steps;d.cache_builds=scene.cache_builds;
    for(int i=0;i<3;i++){d.target_ids[i]=scene.layers[i].id;if(d.target_ids[i])d.live_targets++;}
    return d;
}
