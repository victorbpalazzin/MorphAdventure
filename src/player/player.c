#include "player_internal.h"
#include <math.h>
#include <string.h>

static PlayerState instance;
static float Tilt(float velocity) { return fmaxf(-0.35f, fminf(0.45f, velocity * 0.045f)); }

static void Reset(void *context, const GameState *game, const WorldConfig *config)
{
    (void)game; (void)config;
    PlayerState *p = context;
    p->view = (PlayerView){0};
    p->view.position = (GameVec2){180, 245};
    p->view.visual_radius = 23;
    p->view.collision_radius = 18;
    p->view.scale = 1;
    p->invulnerability_steps = 0;
    p->portal_override = false;
}
static bool Init(void *context)
{
    memset(context, 0, sizeof(PlayerState));
    Reset(context, NULL, NULL);
    return true; /* All art is procedural: no graphics resources to allocate. */
}
static void Shutdown(void *context) { memset(context, 0, sizeof(PlayerState)); }
static PlayerView View(void *context) { return ((PlayerState *)context)->view; }

static void Step(void *context, const GameState *game, const FrameContext *frame,
                 const GameInput *input, float force)
{
    PlayerState *p = context;
    if (game->mode != GAME_PLAY || p->portal_override) return;
    const float dt = frame->normal_steps;
    if (!isfinite(dt) || dt <= 0) return;
    if (!isfinite(force)) force = 0;
    p->invulnerability_steps = fmaxf(0, p->invulnerability_steps - dt);
    p->view.invulnerable = p->invulnerability_steps > 0;
    p->view.touching_floor = false;
    /* Input is already filtered and queued by core. Do not inspect pointer/keys. */
    if (input->impulse_pressed) p->view.vertical_velocity = -6.4f;
    p->view.vertical_velocity = fminf(7.2f, p->view.vertical_velocity + (0.34f + force) * dt);
    p->view.position.y += p->view.vertical_velocity * dt;
    if (p->view.position.y < 42) {
        p->view.position.y = 42;
        p->view.vertical_velocity = 1;
    }
    if (p->view.position.y > MORPH_HEIGHT - 53) {
        p->view.position.y = MORPH_HEIGHT - 53;
        p->view.touching_floor = true; /* Core decides shield, lives and defeat. */
    }
    p->view.rotation = Tilt(p->view.vertical_velocity);
}
static void Hit(void *context)
{
    PlayerState *p = context;
    p->view.vertical_velocity = -4.5f;
    p->view.rotation = Tilt(-4.5f);
    p->invulnerability_steps = 75;
    p->view.invulnerable = true;
}
static void Pose(void *context, PortalPose pose)
{
    PlayerState *p = context;
    p->portal_override = pose.overrides_player;
    if (pose.overrides_player) {
        p->view.position = pose.position;
        p->view.scale = pose.scale;
        p->view.rotation = pose.rotation;
        p->view.touching_floor = false;
    } else {
        p->view.scale = 1;
        p->view.rotation = Tilt(p->view.vertical_velocity);
    }
}
static void Draw(void *context, const GameState *game, const ItemsView *items)
{ PlayerDrawArt(context, game, items); }

PlayerApi PlayerModuleCreate(void)
{
    return (PlayerApi){{&instance, Init, Reset, Shutdown}, Step, View, Hit, Pose, Draw};
}
void PlayerSetDebugHitbox(bool enabled) { instance.debug_hitbox = enabled; }
