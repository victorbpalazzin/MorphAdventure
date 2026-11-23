#include "game.h"
#include "boss/boss.h"
#include "items/combat_energy.h"
#include "edu/quiz.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static uint32_t quiz_session_seed = 0x51a7e5edu;

static uint32_t MixSeed(uint32_t x)
{
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu;
    return x ^ (x >> 16);
}

static bool LifeComplete(ModuleLife life)
{
    return life.init && life.reset_world && life.shutdown;
}

static unsigned Missing(const GameModules *m)
{
    unsigned missing = 0;
    if (!LifeComplete(m->player.life) || !m->player.step || !m->player.view ||
        !m->player.apply_hit || !m->player.set_portal_pose || !m->player.draw) missing |= MODULE_PLAYER;
    if (!LifeComplete(m->worlds.life) || !m->worlds.step || !m->worlds.draw) missing |= MODULE_WORLDS;
    if (!LifeComplete(m->obstacles.life) || !m->obstacles.config || !m->obstacles.step ||
        !m->obstacles.vertical_force || !m->obstacles.touches_player || !m->obstacles.path_clear ||
        !m->obstacles.take_passed || !m->obstacles.passages || !m->obstacles.draw) missing |= MODULE_OBSTACLES;
    if (!LifeComplete(m->items.life) || !m->items.step_timers || !m->items.view ||
        !m->items.consume_shield || !m->items.collect_step || !m->items.draw) missing |= MODULE_ITEMS;
    if (!LifeComplete(m->ui.life) || !m->ui.input || !m->ui.step || !m->ui.draw) missing |= MODULE_UI;
    if (!LifeComplete(m->effects.life) || !m->effects.event || !m->effects.step_particles ||
        !m->effects.begin_portal || !m->effects.step_portal || !m->effects.portal_pose ||
        !m->effects.switch_world_under_fade || !m->effects.draw_portal ||
        !m->effects.draw_particles || !m->effects.draw_transition) missing |= MODULE_EFFECTS;
    return missing;
}

static ModuleLife LifeAt(Game *g, unsigned i)
{
    ModuleLife lives[] = {g->modules.player.life, g->modules.worlds.life,
        g->modules.obstacles.life, g->modules.items.life, g->modules.ui.life, g->modules.effects.life};
    return lives[i];
}

static void Fail(Game *g, const char *message)
{
    snprintf(g->error, sizeof(g->error), "%s", message);
    g->state.mode = GAME_ERROR;
    g->accumulator = 0;
    g->pending_impulse = false; g->pending_attack = false;
}

unsigned GameEnergyRequired(int world)
{
    static const unsigned targets[MORPH_WORLD_COUNT] = {4, 6, 8, 10, 12, 16};
    return world >= 0 && world < MORPH_WORLD_COUNT ? targets[world] : 0u;
}

float GameEnergyFraction(unsigned collected, int world)
{
    unsigned required = GameEnergyRequired(world);
    if (!required) return 0;
    return collected >= required ? 1.0f : (float)collected / required;
}

bool GameEnqueueEvent(GameEventBuffer *buffer, GameEvent event)
{
    if (!buffer) return false;
    if (buffer->count >= MORPH_MAX_EVENTS) { buffer->overflow = true; return false; }
    buffer->values[buffer->count++] = event;
    return true;
}

static bool ConfigValid(const WorldConfig *c)
{
    return c && c->name && isfinite(c->base_speed) && c->base_speed > 0 &&
        isfinite(c->duration_steps) && c->duration_steps > 0 &&
        isfinite(c->base_speed * c->duration_steps) &&
        isfinite(c->gap) && c->gap > 36 && c->gap < MORPH_HEIGHT &&
        isfinite(c->motion) && c->motion >= 0 &&
        isfinite(c->obstacle_interval) && c->obstacle_interval > 0 &&
        isfinite(c->angular_frequency) && c->angular_frequency >= 0 &&
        isfinite(c->event_interval) && c->event_interval > 0 && isfinite(c->vertical_force);
}

static bool BeginWorld(Game *g, int world, bool preserve_fade)
{
    if (world < 0 || world >= MORPH_WORLD_COUNT) { Fail(g, "Invalid world index."); return false; }
    const WorldConfig *cfg = g->modules.obstacles.config(g->modules.obstacles.life.context, world);
    if (!ConfigValid(cfg)) { Fail(g, "Missing/invalid world configuration from person 4."); return false; }
    g->config = cfg;
    g->state.world = world;
    g->state.world_id++;
    g->life_awarded = false;
    g->state.edu.portal_unlocked = false; /* each world needs its own correct answer */
    g->state.distance = g->state.scroll = 0;
    g->state.minimum_distance = cfg->base_speed * cfg->duration_steps;
    g->state.world_steps = 0;
    g->state.speed = cfg->base_speed;
    g->pending_impulse = false; g->pending_attack = false;
    /* Lives/score intentionally untouched. Effects retain fade at the midpoint. */
    for (unsigned i = 0; i < 5; i++) {
        ModuleLife life = LifeAt(g, i);
        life.reset_world(life.context, &g->state, cfg);
    }
    EffectsApi *fx = &g->modules.effects;
    if (preserve_fade) fx->switch_world_under_fade(fx->life.context, &g->state, cfg);
    else fx->life.reset_world(fx->life.context, &g->state, cfg);
    return true;
}

static void NewRun(Game *g, GameMode mode)
{
    g->state.boss = (BossState){0};
    g->state.lives = 3;
    g->state.score = 0;
    g->state.run_id++;
    /* Second chance and challenge deck restart only with a new adventure. */
    bool educational = g->state.edu.enabled;
    g->state.edu = (EduState){0};
    g->state.edu.enabled = educational;
    QuizSeed(&g->state.edu.quiz, MixSeed(quiz_session_seed ^ MixSeed(g->state.run_id)));
    g->state.simulation_steps = 0;
    g->state.mode = mode;
    g->state.resume_mode = GAME_PLAY;
    g->accumulator = 0;
    g->pending_impulse = false; g->pending_attack = false;
    g->transition_switched = false;
    (void)BeginWorld(g, 0, false);
}

bool GameEducationalBuild(void) { return MORPH_EDUCATIONAL != 0; }

void GameSetQuizSeed(uint32_t seed) { quiz_session_seed = seed; }

bool GameInit(Game *g, const GameModules *modules)
{
    return GameInitMode(g, modules, GameEducationalBuild());
}

bool GameInitMode(Game *g, const GameModules *modules, bool educational)
{
    if (!g || !modules) return false;
    memset(g, 0, sizeof(*g));
    g->state.edu.enabled = educational;
    g->modules = *modules;
    g->state.mode = GAME_INTRO;
    g->state.resume_mode = GAME_PLAY;
    g->state.lives = 3;
    if (modules->contract_version != MORPH_CONTRACT_VERSION) {
        Fail(g, "Incompatible module contract version."); return false;
    }
    g->missing_modules = Missing(modules);
    if (g->missing_modules) return true; /* Explicit diagnostic shell, no fake modules. */
    for (unsigned i = 0; i < 6; i++) {
        ModuleLife life = LifeAt(g, i);
        if (!life.init(life.context)) {
            Fail(g, "Module initialization failed. See module log.");
            GameShutdown(g); return false;
        }
        g->initialized_modules |= 1u << i;
    }
    for (int i = 0; i < MORPH_WORLD_COUNT; i++) {
        if (!ConfigValid(modules->obstacles.config(modules->obstacles.life.context, i))) {
            Fail(g, "World configurations 0..5 must all be available.");
            GameShutdown(g); return false;
        }
    }
    g->ready = true;
    NewRun(g, GAME_INTRO);
    return g->state.mode != GAME_ERROR;
}

void GameShutdown(Game *g)
{
    if (!g) return;
    for (int i = 5; i >= 0; i--) {
        if (g->initialized_modules & (1u << i)) {
            ModuleLife life = LifeAt(g, (unsigned)i);
            life.shutdown(life.context);
        }
    }
    g->initialized_modules = 0;
    g->ready = false;
}

static bool Active(GameMode mode)
{
    return mode == GAME_PLAY || mode == GAME_PORTAL || mode == GAME_TRANSITION || mode == GAME_BOSS;
}

/* The logic challenge is not simulation, but pause/focus loss must freeze it. */
static bool Pausable(GameMode mode) { return Active(mode) || mode == GAME_QUIZ; }

static void Pause(Game *g)
{
    if (Pausable(g->state.mode)) { g->state.resume_mode = g->state.mode; g->state.mode = GAME_PAUSED; }
    g->accumulator = 0;
    g->pending_impulse = false; g->pending_attack = false;
}

static bool Command(Game *g, GameCommand command)
{
    GameMode before = g->state.mode;
    if (command == COMMAND_RESTART) NewRun(g, GAME_INTRO);
    else if (command == COMMAND_START && before == GAME_INTRO) NewRun(g, GAME_PLAY);
    else if (command == COMMAND_RETRY && (before == GAME_LOST || before == GAME_WON)) NewRun(g, GAME_PLAY);
    else if (command == COMMAND_TOGGLE_PAUSE && Pausable(before)) Pause(g);
    else if ((command == COMMAND_RESUME || command == COMMAND_TOGGLE_PAUSE) && before == GAME_PAUSED) {
        g->state.mode = g->state.resume_mode;
        g->accumulator = 0;
        g->pending_impulse = false; g->pending_attack = false;
    } else return false;
    return true;
}

static ItemsView ReadItems(Game *g)
{
    ItemsView v = g->modules.items.view(g->modules.items.life.context);
    if (v.collected > GameEnergyRequired(g->state.world) || v.boost_count > MORPH_MAX_BOOSTS ||
        (v.score_multiplier != 1 && v.score_multiplier != 2) ||
        (!isfinite(v.world_scale)) || (fabsf(v.world_scale - 1) > 0.0001f && fabsf(v.world_scale - 0.68f) > 0.0001f)) {
        Fail(g, "ItemsView violates the shared contract.");
    }
    return v;
}

static void Notify(Game *g, GameEventType type, GameVec2 position, unsigned multiplier)
{
    GameEvent event = {0};
    event.type = type; event.position = position; event.multiplier = multiplier;
    g->modules.effects.event(g->modules.effects.life.context, &event);
}

static void Reward(Game *g, unsigned points, unsigned multiplier)
{
    if (multiplier != 1 && multiplier != 2) { Fail(g, "Invalid event reward multiplier."); return; }
    uint64_t addition = (uint64_t)points * multiplier;
    if (UINT64_MAX - g->state.score < addition) { Fail(g, "Score overflow."); return; }
    g->state.score += addition;
}

/* Opens a challenge and freezes everything except the presentation clock. */
static void OpenQuiz(Game *g, QuizKind kind)
{
    QuizBegin(&g->state.edu.quiz, kind, g->state.mode);
    g->state.mode = GAME_QUIZ;
    g->accumulator = 0;
    g->pending_impulse = false; g->pending_attack = false;
}

static bool Damage(Game *g, PlayerView player)
{
    if (player.invulnerable || (g->state.mode != GAME_PLAY && g->state.mode != GAME_BOSS)) return false;
    if (g->state.edu.grace_steps > 0) return false; /* protection after the second chance */
    ItemsView protection = ReadItems(g);
    if (g->state.mode == GAME_ERROR) return false;
    for (unsigned i=0;i<protection.boost_count;i++)
        if (protection.boosts[i].type==BOOST_PHASE) return false;
    bool shield = g->modules.items.consume_shield(g->modules.items.life.context);
    g->modules.player.apply_hit(g->modules.player.life.context);
    if (!shield) g->state.lives--;
    Notify(g, shield ? EVENT_SHIELD_ABSORBED : EVENT_DAMAGE, player.position, 1);
    if (g->state.lives <= 0) {
        g->state.lives = 0;
        if (g->state.edu.enabled && !g->state.edu.revive_used) { OpenQuiz(g, QUIZ_REVIVE); return true; }
        g->state.mode = GAME_LOST;
        g->pending_impulse = false; g->pending_attack = false;
        g->accumulator = 0;
    }
    return true;
}

static float Pace(const GameState *s)
{
    float progress = s->distance / s->minimum_distance;
    if (progress < 0.12f) return 0.98f;
    if (progress > 0.86f) return 1.1f + 0.025f * sinf((float)s->world_steps * 0.012f);
    return 1.025f + 0.085f * sinf((float)fmod(s->world_steps, 840.0) / 840.0f * 6.28318530718f);
}

static FrameContext Frame(Game *g, float scale)
{
    FrameContext f = {0};
    f.normal_steps = 1; f.world_steps = scale; f.world_scale = scale; f.pace = 1;
    f.normal_seconds = f.ui_seconds = (float)MORPH_STEP_SECONDS;
    f.world_seconds = (float)MORPH_STEP_SECONDS * scale;
    f.tick = g->state.simulation_steps;
    return f;
}

static void Step(Game *g, GameInput input)
{
    GameModules *m = &g->modules;
    g->state.simulation_steps++;
    if (g->state.edu.grace_steps > 0) g->state.edu.grace_steps = fmaxf(0, g->state.edu.grace_steps - 1);
    ItemsView items = ReadItems(g);
    if (g->state.mode == GAME_ERROR) return;
    FrameContext f = Frame(g, 1);
    if (g->state.mode == GAME_PLAY) {
        m->items.step_timers(m->items.life.context, &f);
        items = ReadItems(g);
        if (g->state.mode == GAME_ERROR) return;
        f = Frame(g, items.world_scale);
        g->state.distance = g->state.minimum_distance * GameEnergyFraction(items.collected, g->state.world);
        f.pace = Pace(&g->state);
        g->state.speed += (g->config->base_speed * f.pace - g->state.speed) * 0.025f * f.world_steps;
        f.scroll_delta = g->state.speed * f.world_steps;
        g->state.scroll += f.scroll_delta;
        g->state.world_steps += f.world_steps;
        PlayerView player = m->player.view(m->player.life.context);
        bool spawn = items.collected < GameEnergyRequired(g->state.world);
        m->obstacles.step(m->obstacles.life.context, &g->state, &f, player, spawn);
        m->player.step(m->player.life.context, &g->state, &f, &input, m->obstacles.vertical_force(m->obstacles.life.context));
        player = m->player.view(m->player.life.context);
        unsigned passed = m->obstacles.take_passed(m->obstacles.life.context);
        if (passed > 64) { Fail(g, "Too many obstacle rewards in one simulation step."); return; }
        for (unsigned i = 0; i < passed; i++) {
            Reward(g, 10, items.score_multiplier);
            Notify(g, EVENT_OBSTACLE_PASSED, player.position, items.score_multiplier);
        }
        if (g->state.mode == GAME_ERROR) return;
        if (player.touching_floor || m->obstacles.touches_player(m->obstacles.life.context, player)) Damage(g, player);
        if (g->state.mode != GAME_PLAY) return;
        GameEventBuffer events = {0};
        PassageQuery query = m->obstacles.passages(m->obstacles.life.context);
        if (!query.safe_y || !query.is_safe) { Fail(g, "Missing passage query functions."); return; }
        m->items.collect_step(m->items.life.context, &g->state, &f, player, query, &events);
        if (events.overflow || events.count > MORPH_MAX_EVENTS) { Fail(g, "Item event queue overflow."); return; }
        for (size_t i = 0; i < events.count; i++) {
            GameEvent *event = &events.values[i];
            if (event->type == EVENT_FRAGMENT_COLLECTED) Reward(g, 35, event->multiplier);
            else if (event->type == EVENT_BOOST_COLLECTED && event->boost == BOOST_LIFE) {
                if (g->state.world%2==1 && !g->life_awarded) {
                    g->state.lives++;g->life_awarded=true;
                }
            }
            else if (event->type != EVENT_BOOST_COLLECTED) { Fail(g, "Unexpected item event."); return; }
            if (g->state.mode == GAME_ERROR) return;
            m->effects.event(m->effects.life.context, event);
        }
        unsigned before = items.collected;
        items = ReadItems(g);
        if (g->state.mode == GAME_ERROR) return;
        g->state.distance = g->state.minimum_distance * GameEnergyFraction(items.collected, g->state.world);
        if (before < GameEnergyRequired(g->state.world) && items.collected == GameEnergyRequired(g->state.world))
            Notify(g, EVENT_PORTAL_CHARGED, player.position, 1);
        if (items.collected == GameEnergyRequired(g->state.world) &&
            m->obstacles.path_clear(m->obstacles.life.context, player)) {
            /* Educational: the same safe moment opens the challenge; the portal
             * itself only begins after a correct answer (QuizResolve). */
            if (g->state.edu.enabled && !g->state.edu.portal_unlocked) { OpenQuiz(g, QUIZ_PASSAGE); return; }
            g->state.mode = GAME_PORTAL;
            g->pending_impulse = false; g->pending_attack = false;
            m->effects.begin_portal(m->effects.life.context, &g->state, player);
        }
    } else if (g->state.mode == GAME_BOSS) {
        BossState *boss = &g->state.boss;
        if (boss->stage == BOSS_FIGHT) m->items.step_timers(m->items.life.context, &f);
        items = ReadItems(g);
        if (g->state.mode == GAME_ERROR) return;
        f = Frame(g, items.world_scale);
        g->state.speed += (0.8f-g->state.speed)*0.04f;
        f.scroll_delta=g->state.speed*f.world_steps;
        g->state.scroll+=f.scroll_delta;
        if (boss->stage == BOSS_FIGHT && !boss->beam_active)
            m->player.step(m->player.life.context, &g->state, &f, &input, 0);
        PlayerView player=m->player.view(m->player.life.context);
        BossStep(boss, &f, player);
        if (boss->stage == BOSS_FIGHT && !boss->beam_active) {
            if (player.touching_floor) (void)Damage(g,player);
            else if (BossTouches(boss,player) && Damage(g,player)) BossRegisterContact(boss,player);
            if (g->state.mode != GAME_BOSS) return;
            bool magnet=false;
            for (unsigned i=0;i<items.boost_count;i++) if(items.boosts[i].type==BOOST_MAGNET) magnet=true;
            if (CombatEnergyStep(&boss->energy, &f, player, magnet)) {
                Reward(g,35,items.score_multiplier);
                Notify(g,EVENT_FRAGMENT_COLLECTED,player.position,items.score_multiplier);
            }
            if(input.attack_pressed) (void)BossFire(boss,player);
        }
        if(boss->stage==BOSS_DONE) {g->state.mode=GAME_WON;return;}
    } else if (g->state.mode == GAME_PORTAL || g->state.mode == GAME_TRANSITION) {
        if (g->state.mode == GAME_PORTAL) {
            f.scroll_delta = g->state.speed * 0.35f;
            g->state.scroll += f.scroll_delta;
            m->obstacles.step(m->obstacles.life.context, &g->state, &f, m->player.view(m->player.life.context), false);
            (void)m->obstacles.take_passed(m->obstacles.life.context); /* no portal bonus for stale obstacles */
        }
        PortalSignal signal = m->effects.step_portal(m->effects.life.context, &g->state, &f);
        if (signal == PORTAL_SIGNAL_ENTERED && g->state.mode == GAME_PORTAL) {
            Reward(g, 100, items.score_multiplier);
            if (g->state.mode == GAME_ERROR) return;
            Notify(g, EVENT_PORTAL_ENTERED, m->player.view(m->player.life.context).position, items.score_multiplier);
            g->state.mode = GAME_TRANSITION;
            g->transition_switched = false;
        } else if (signal == PORTAL_SIGNAL_BLACKOUT && g->state.mode == GAME_TRANSITION && !g->transition_switched) {
            g->transition_switched = true;
            if (g->state.world == MORPH_WORLD_COUNT - 1) {
                g->state.mode = GAME_BOSS;
                BossBegin(&g->state.boss, MORPH_COMBAT_ENERGY_DEFAULT);
                m->player.life.reset_world(m->player.life.context, &g->state, g->config);
                m->obstacles.life.reset_world(m->obstacles.life.context, &g->state, g->config);
                m->effects.life.reset_world(m->effects.life.context, &g->state, g->config);
                g->pending_attack = g->pending_impulse = false;
                return;
            }
            if (!BeginWorld(g, g->state.world + 1, true)) return;
            items = ReadItems(g);
        } else if (signal == PORTAL_SIGNAL_FADE_DONE && g->state.mode == GAME_TRANSITION && g->transition_switched) {
            g->state.mode = GAME_PLAY;
            g->pending_impulse = false; g->pending_attack = false;
        }
        m->player.set_portal_pose(m->player.life.context, m->effects.portal_pose(m->effects.life.context));
    }
    if (g->state.mode == GAME_ERROR) return;
    m->worlds.step(m->worlds.life.context, &g->state, &f);
    m->effects.step_particles(m->effects.life.context, &g->state, &f);
    m->ui.step(m->ui.life.context, &g->state, &f, &items);
}

/* Second chance accepted: exactly one life, same world/score/letters/boss. */
static void Revive(Game *g, GameMode back)
{
    GameModules *m = &g->modules;
    g->state.lives = 1;
    g->state.mode = back;
    g->accumulator = 0;
    g->pending_impulse = false; g->pending_attack = false;
    m->player.life.reset_world(m->player.life.context, &g->state, g->config); /* (180,245), speed 0 */
    m->player.apply_hit(m->player.life.context); /* small hop + visible invulnerability */
    g->state.edu.grace_steps = MORPH_REVIVE_GRACE_STEPS;
}

static void QuizResolve(Game *g)
{
    EduState *e = &g->state.edu;
    QuizState *q = &e->quiz;
    if (q->kind == QUIZ_PASSAGE) {
        if (q->result != QUIZ_CORRECT) { QuizNextQuestion(q); return; } /* portal stays locked */
        e->portal_unlocked = true;
        q->kind = QUIZ_NONE;
        g->state.mode = GAME_PORTAL;
        g->accumulator = 0;
        g->pending_impulse = false; g->pending_attack = false;
        GameModules *m = &g->modules;
        m->effects.begin_portal(m->effects.life.context, &g->state, m->player.view(m->player.life.context));
    } else if (q->kind == QUIZ_REVIVE) {
        GameMode back = q->return_mode;
        e->revive_used = true; /* single attempt, whatever the result */
        q->kind = QUIZ_NONE;
        if (q->result == QUIZ_CORRECT) Revive(g, back);
        else { g->state.lives = 0; g->state.mode = GAME_LOST; g->accumulator = 0; }
    }
}

/* Presentation clock: real frame seconds limited to 0.1 s, independent of the
 * fixed simulation, world speed and slow motion. Elapsed time is applied before
 * the input of the same frame, so an input arriving at the deadline is late. */
static void QuizFrame(Game *g, double seconds, GameCommand command)
{
    QuizState *q = &g->state.edu.quiz;
    g->accumulator = 0;
    g->pending_impulse = false; g->pending_attack = false;
    if (q->phase == QUIZ_ASKING) {
        if (QuizAdvanceClock(q, seconds)) { g->state.edu.answered++; return; }
        int option = QuizOptionFromCommand(command);
        if (option >= 0 && QuizAnswer(q, option)) {
            g->state.edu.answered++;
            if (q->result == QUIZ_CORRECT) g->state.edu.correct++;
        }
        return;
    }
    (void)QuizAdvanceClock(q, seconds); /* feedback: question clock frozen */
    if (command == COMMAND_CONTINUE && QuizCanContinue(q)) QuizResolve(g);
}

void GameAdvance(Game *g, double seconds, GameInput input)
{
    if (!g || !g->ready || g->state.mode == GAME_ERROR) return;
    if (!input.focused) { Pause(g); return; }
    UiDecision ui = g->modules.ui.input(g->modules.ui.life.context, &g->state, &input);
    GameCommand command = ui.command != COMMAND_NONE ? ui.command : input.command;
    if (Command(g, command)) return; /* Never simulate the elapsed time before a reset/resume. */
    if (g->state.mode == GAME_QUIZ) { QuizFrame(g, seconds, command); return; }
    if (!Active(g->state.mode)) {
        g->accumulator = 0; g->pending_impulse = false; g->pending_attack = false;
        if ((g->state.mode==GAME_INTRO || g->state.mode==GAME_WON) && isfinite(seconds) && seconds>0) {
            FrameContext presentation={.ui_seconds=(float)fmin(seconds,.1)};
            ItemsView items=ReadItems(g);
            if(g->state.mode!=GAME_ERROR)g->modules.ui.step(g->modules.ui.life.context,&g->state,&presentation,&items);
        }
        return;
    }
    if ((g->state.mode == GAME_PLAY || g->state.mode == GAME_BOSS) && (input.impulse_pressed ||
        (input.pointer_pressed && input.pointer_inside && !ui.consumes_pointer))) g->pending_impulse = true;
    if (g->state.mode == GAME_BOSS && input.attack_pressed) g->pending_attack = true;
    if (!isfinite(seconds) || seconds <= 0) return;
    g->accumulator += fmin(seconds, 0.1);
    unsigned count = 0;
    while (g->accumulator + 1e-10 >= MORPH_STEP_SECONDS && count < MORPH_MAX_CATCHUP_STEPS && Active(g->state.mode)) {
        g->accumulator = fmax(0, g->accumulator - MORPH_STEP_SECONDS);
        GameInput step_input = input;
        step_input.command = COMMAND_NONE;
        step_input.pointer_pressed = false;
        step_input.impulse_pressed = g->pending_impulse;
        step_input.attack_pressed = g->pending_attack;
        g->pending_impulse = false; g->pending_attack = false;
        Step(g, step_input);
        count++;
    }
    if (!Active(g->state.mode)) g->accumulator = 0;
    else if (count == MORPH_MAX_CATCHUP_STEPS) g->accumulator = fmod(g->accumulator, MORPH_STEP_SECONDS);
}

void GameDraw(Game *g)
{
    if (!g || !g->ready || g->state.mode == GAME_ERROR) return;
    GameModules *m = &g->modules;
    ItemsView items = ReadItems(g);
    if (g->state.mode == GAME_ERROR) return;
    m->worlds.draw(m->worlds.life.context, &g->state);
    if(g->state.boss.stage==BOSS_DORMANT) {
        m->obstacles.draw(m->obstacles.life.context, &g->state);
        m->items.draw(m->items.life.context, &g->state);
    }
    m->effects.draw_portal(m->effects.life.context, &g->state);
    m->player.draw(m->player.life.context, &g->state, &items);
    m->effects.draw_particles(m->effects.life.context, &g->state);
    m->ui.draw(m->ui.life.context, &g->state, &items);
    /* The final blackout stops advancing at WON; never cover the victory UI. */
    if (g->state.mode != GAME_WON && g->state.boss.stage==BOSS_DORMANT)
        m->effects.draw_transition(m->effects.life.context, &g->state);
}

const char *GameModeName(GameMode mode)
{
    static const char *names[] = {"INTRO", "PLAY", "PAUSED", "PORTAL", "TRANSITION", "LOST", "WON", "ERROR", "BOSS", "QUIZ"};
    return mode >= GAME_INTRO && mode <= GAME_QUIZ ? names[mode] : "INVALID";
}
