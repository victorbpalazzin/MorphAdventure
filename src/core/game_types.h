#ifndef MORPH_GAME_TYPES_H
#define MORPH_GAME_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MORPH_CONTRACT_VERSION 6u
#define MORPH_WIDTH 960
#define MORPH_HEIGHT 540
#define MORPH_WORLD_COUNT 6
#define MORPH_MAX_ENERGY 24
#define MORPH_MAX_BOOSTS 2
#define MORPH_MAX_EVENTS 128
#define MORPH_STEP_SECONDS (1.0 / 60.0)
#define MORPH_MAX_CATCHUP_STEPS 5

/* Pure C types: the coordinator and tests do not depend on a graphics context. */
typedef struct GameVec2 { float x, y; } GameVec2;
typedef struct GameRect { float x, y, width, height; } GameRect;
typedef enum GameMode {
    GAME_INTRO, GAME_PLAY, GAME_PAUSED, GAME_PORTAL,
    GAME_TRANSITION, GAME_LOST, GAME_WON, GAME_ERROR, GAME_BOSS,
    GAME_QUIZ /* educational build only: logic challenge, simulation frozen */
} GameMode;
typedef enum GameCommand {
    COMMAND_NONE, COMMAND_START, COMMAND_TOGGLE_PAUSE,
    COMMAND_RESUME, COMMAND_RESTART, COMMAND_RETRY,
    /* Educational challenge: choose alternative A..D, then continue after feedback. */
    COMMAND_ANSWER_A, COMMAND_ANSWER_B, COMMAND_ANSWER_C, COMMAND_ANSWER_D,
    COMMAND_CONTINUE
} GameCommand;
typedef enum BoostType { BOOST_SHIELD, BOOST_MAGNET, BOOST_SLOW, BOOST_DOUBLE, BOOST_LIFE, BOOST_PHASE } BoostType;

typedef struct WorldConfig {
    const char *name;
    float base_speed;          /* logical px / reference step */
    float gap, obstacle_interval, motion, angular_frequency;
    float duration_steps, event_interval, vertical_force;
} WorldConfig;

/* Bounded combat state; core owns mutations through specialized helpers. */
#define MORPH_BOSS_PROJECTILES 24
#define MORPH_COMBAT_ENERGY_DEFAULT 4u
typedef struct CombatEnergy {
    unsigned collected, required, serial;
    bool present;
    GameVec2 position;
    float spawn_timer;
} CombatEnergy;
typedef enum BossStage { BOSS_DORMANT, BOSS_INTRO, BOSS_FIGHT, BOSS_DYING, BOSS_DONE } BossStage;
typedef struct BossProjectile { GameVec2 position, velocity; float radius; GameVec2 previous; bool spent; } BossProjectile;
typedef struct BossState {
    BossStage stage;
    unsigned segments, phase, volley, projectile_count, burst_remaining, burst_index;
    float burst_timer;
    float age, clock, attack_timer, warning, target_y, beam_age, hurt, battery_display;
    bool beam_active, beam_hit;
    GameVec2 beam_origin;
    CombatEnergy energy;
    BossProjectile projectiles[MORPH_BOSS_PROJECTILES];
} BossState;
/* Educational mode (MORPH_EDUCATIONAL). Owned by the core; read-only elsewhere.
 * The challenge clock uses presentation time, never world/slow-motion time. */
#define MORPH_QUIZ_SECONDS 12.0f
#define MORPH_QUIZ_DECK 64
#define MORPH_QUIZ_INPUT_GUARD 0.35f   /* s: an in-flight flap/click cannot answer */
#define MORPH_QUIZ_FEEDBACK_GUARD 0.5f /* s: feedback is readable before continuing */
#define MORPH_REVIVE_GRACE_STEPS 180.0f
typedef enum QuizKind { QUIZ_NONE, QUIZ_PASSAGE, QUIZ_REVIVE } QuizKind;
typedef enum QuizPhase { QUIZ_ASKING, QUIZ_FEEDBACK } QuizPhase;
typedef enum QuizResult { QUIZ_PENDING, QUIZ_CORRECT, QUIZ_WRONG, QUIZ_TIMEOUT } QuizResult;
typedef struct QuizState {
    QuizKind kind;
    QuizPhase phase;
    QuizResult result;
    GameMode return_mode;      /* PLAY (passage/revive) or BOSS (revive) */
    int question, last_question, selected;
    float seconds_left;        /* starts at MORPH_QUIZ_SECONDS; frozen in feedback */
    float age, feedback_age;   /* presentation seconds; guards and animation */
    unsigned serial;           /* increments for each new question or phase */
    unsigned deck_position, deck_count;
    unsigned char deck[MORPH_QUIZ_DECK];
    uint32_t seed;
} QuizState;
typedef struct EduState {
    bool enabled;              /* fixed at GameInit from MORPH_EDUCATIONAL */
    bool revive_used;          /* one second chance per adventure */
    bool portal_unlocked;      /* correct answer for the current world */
    float grace_steps;         /* normal steps of core-side protection after revive */
    unsigned answered, correct;
    QuizState quiz;
} EduState;
typedef struct GameState {
    GameMode mode, resume_mode;
    int world, lives;
    uint64_t score;
    /* distance/minimum_distance tracks collected energy; scroll is actual travel. */
    float distance, minimum_distance, scroll, speed;
    double world_steps;        /* advances with world scale */
    uint64_t simulation_steps;
    BossState boss;
    uint32_t run_id, world_id; /* reject stale module state across resets */
    EduState edu;              /* inert (enabled=false) in the arcade build */
} GameState;

typedef struct GameInput {
    bool focused;
    bool attack_pressed;     /* J edge, queued/consumed by core */
    bool impulse_pressed;     /* queued until the next fixed step; consumed once */
    bool pointer_pressed, pointer_inside;
    GameVec2 pointer;          /* already transformed into 960x540 coordinates */
    GameCommand command;
} GameInput;

typedef struct FrameContext {
    float normal_steps;       /* always 1 at the fixed simulation rate */
    float world_steps;        /* normal_steps * world_scale: do not scale again */
    float normal_seconds, world_seconds, ui_seconds;
    float world_scale, scroll_delta, pace;
    uint64_t tick;
} FrameContext;

typedef struct PlayerView {
    GameVec2 position;
    float vertical_velocity, visual_radius, collision_radius;
    float scale, rotation;
    bool invulnerable, touching_floor;
} PlayerView;
typedef struct BoostView { BoostType type; float seconds_left; } BoostView;
typedef struct ItemsView {
    unsigned collected;
    unsigned boost_count;
    BoostView boosts[MORPH_MAX_BOOSTS];
    bool shield;
    unsigned score_multiplier; /* exactly 1 or 2 */
    float world_scale;        /* exactly 1 or 0.68 */
} ItemsView;

typedef enum GameEventType {
    EVENT_OBSTACLE_PASSED, EVENT_FRAGMENT_COLLECTED, EVENT_BOOST_COLLECTED,
    EVENT_DAMAGE, EVENT_SHIELD_ABSORBED, EVENT_PORTAL_CHARGED, EVENT_PORTAL_ENTERED
} GameEventType;
typedef struct GameEvent {
    GameEventType type;
    GameVec2 position;
    unsigned multiplier;     /* reward multiplier at the time of this event */
    unsigned entity_id;
    BoostType boost;
} GameEvent;
typedef struct GameEventBuffer {
    GameEvent values[MORPH_MAX_EVENTS];
    size_t count;
    bool overflow;
} GameEventBuffer;

typedef struct PassageQuery {
    void *context;
    float (*safe_y)(void *, float x, float fallback_y);
    bool (*is_safe)(void *, GameVec2 position, float radius);
} PassageQuery;

typedef struct PortalPose {
    bool overrides_player;
    GameVec2 position;
    float scale, rotation;
} PortalPose;
typedef enum PortalSignal {
    PORTAL_SIGNAL_NONE, PORTAL_SIGNAL_ENTERED,
    PORTAL_SIGNAL_BLACKOUT, PORTAL_SIGNAL_FADE_DONE
} PortalSignal;
typedef struct UiDecision {
    GameCommand command;
    bool consumes_pointer;
} UiDecision;
typedef struct GameViewport { float x, y, width, height, scale; } GameViewport;

#endif
