#ifndef MORPH_OBSTACLES_INTERNAL_H
#define MORPH_OBSTACLES_INTERNAL_H
#include "obstacles.h"
typedef struct ObstaclesState {
    int world;
    GameMode mode;
    unsigned column_count,hazard_count,passed,generated,capacity_delays;
    uint32_t seed;
    double clock;
    float spawn_timer,event_timer;
    bool debug;
    ObstacleEnvironmentView environment;
    ObstacleColumn columns[OBSTACLE_CAPACITY];
    ObstacleHazard hazards[HAZARD_CAPACITY];
    ObstacleProtectedCircle protected_items[OBSTACLE_PROTECTED_CAPACITY];
    size_t protected_count;
} ObstaclesState;
extern const WorldConfig ObstacleWorldConfigs[MORPH_WORLD_COUNT];
void ObstaclesDraw(const ObstaclesState *state,const GameState *game);
#endif
