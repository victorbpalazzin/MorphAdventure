#ifndef MORPH_WORLDS_INTERNAL_H
#define MORPH_WORLDS_INTERNAL_H
#include "worlds.h"
#include "raylib.h"
#define WORLD_LAYER_WIDTH 1920
typedef struct WorldScene {
    RenderTexture2D layers[3];
    bool ready;
    int world;
    double visual_steps;
    unsigned cache_builds;
    WorldVisualEnvironment environment;
} WorldScene;
void WorldBuildLayer(int world,int depth);
void WorldDrawBackground(const WorldScene *scene,const GameState *state);
/* Bounded vector helpers; no retained raylib render state. */
void WPoly(const Vector2 *points,int count,Color color);
void WOval(float x,float y,float rx,float ry,Color color,float angle);
void WLine(const Vector2 *points,int count,Color color,float width);
void WCurve(Vector2 a,Vector2 b,Vector2 c,Vector2 d,Color color,float width);
void WCloud(float x,float y,float scale,Color color);
void WCrystal(float x,float y,float size,Color color);
#define WP(color,...) WPoly((Vector2[]){__VA_ARGS__},(int)(sizeof((Vector2[]){__VA_ARGS__})/sizeof(Vector2)),color)
#define WL(color,width,...) WLine((Vector2[]){__VA_ARGS__},(int)(sizeof((Vector2[]){__VA_ARGS__})/sizeof(Vector2)),color,width)
#define WC(hex) GetColor(hex##ffu)
#endif
