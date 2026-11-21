#ifndef MORPH_OBSTACLES_H
#define MORPH_OBSTACLES_H
#include "core/module_api.h"
#define OBSTACLE_CAPACITY 16
#define HAZARD_CAPACITY 8
#define OBSTACLE_POLYGON_CAPACITY 64
#define OBSTACLE_PROTECTED_CAPACITY 32
typedef enum ObstacleHazardType { HAZARD_JELLY, HAZARD_ICE, HAZARD_FIRE, HAZARD_WIND, HAZARD_VINE, HAZARD_ROCK } ObstacleHazardType;
typedef struct ObstacleColumn {
    int world;
    uint32_t id;
    float x,width,base_y,gap_y,gap,motion,phase;
    bool passed;
} ObstacleColumn;
typedef struct ObstacleHazard {
    ObstacleHazardType type;
    uint32_t id;
    float x,y,age,radius,base_y;
    GameVec2 previous;
    float phase,speed; /* deterministic per-spawn motion; world-step units */
} ObstacleHazard;
typedef struct ObstacleHazardConfig {
    float speed, interval, amplitude, frequency, warning;
    unsigned max_active;
} ObstacleHazardConfig;
const ObstacleHazardConfig *ObstaclesHazardSettings(int world);
typedef struct ObstacleProtectedCircle { GameVec2 center; float radius; } ObstacleProtectedCircle;
typedef struct ObstacleEnvironmentView {
    bool active,warning;
    float intensity,direction,age,duration;
} ObstacleEnvironmentView;
typedef struct ObstaclesSnapshot {
    int world;
    unsigned column_count,hazard_count,generated,capacity_delays;
    double clock;
    ObstacleEnvironmentView environment;
    ObstacleColumn columns[OBSTACLE_CAPACITY];
    ObstacleHazard hazards[HAZARD_CAPACITY];
} ObstaclesSnapshot;

ObstaclesApi ObstaclesModuleCreate(void); /* one private static instance */
/* Deterministic seed used by subsequent reset_world, not wall time/rand(). */
void ObstaclesSetSeed(uint32_t seed);
/* Copied, bounded item positions; no dependency on ItemsApi internals.
 * Integrator supplies current items before obstacles.step; return false if invalid.
 * Without this optional feed, items must consult PassageQuery when placing/moving. */
bool ObstaclesSetProtectedItems(const ObstacleProtectedCircle *circles,size_t count);
ObstacleEnvironmentView ObstaclesEnvironment(void);
ObstaclesSnapshot ObstaclesInspect(void);
/* Exact outline used by rendering AND collision. Output capacity must be >=64. */
size_t ObstaclesColumnPolygon(const ObstacleColumn *column,bool top,GameVec2 *out,size_t capacity);
size_t ObstaclesIcePolygon(const ObstacleHazard *hazard,GameVec2 *out,size_t capacity);
void ObstaclesSetDebugGeometry(bool enabled);
#endif
