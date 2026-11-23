/* Coordenador real (src/core/game.c) + módulos simulados SOMENTE neste teste.
 * Cobre o modo educativo e a regressão do arcade com o mesmo fixture. */
#include "core/game.h"
#include "boss/boss.h"
#include "edu/quiz.h"
#include "morph_check.h"
#include <string.h>

static const WorldConfig configs[6] = {
    {"Ar",4.2f,210,78.2f,12,.019f,1500,440,.065f}, {"Agua",4.6f,202,73.1f,16,.021f,2100,410,.080f},
    {"Floresta",5,194,68,20,.023f,2700,380,0},     {"Gelo",5.4f,186,62.9f,24,.025f,3300,350,.090f},
    {"Terra",5.8f,178,57.8f,28,.027f,3900,320,.105f}, {"Fogo",6.2f,170,52.7f,32,.029f,4500,290,0}};

typedef struct Fixture {
    PlayerView player;
    ItemsView items;
    PortalSignal signal;
    GameEventBuffer queued;
    bool collision, clear;
    unsigned passed;
    int steps, impulses, hits, timer_steps, collect_steps, obstacle_steps, portal_begins, portal_steps;
    int player_resets, item_resets, obstacle_resets;
} Fixture;

static bool Init(void *p) { (void)p; return true; }
static void Shut(void *p) { (void)p; }
static void Reset(void *p, const GameState *s, const WorldConfig *c) { (void)p; (void)s; (void)c; }
static void ResetPlayer(void *p, const GameState *s, const WorldConfig *c)
{
    (void)s; (void)c; Fixture *f = p; f->player_resets++;
    f->player = (PlayerView){0}; f->player.position = (GameVec2){180, 245};
    f->player.collision_radius = 18; f->player.visual_radius = 23; f->player.scale = 1;
}
static void ResetItems(void *p, const GameState *s, const WorldConfig *c)
{
    (void)s; (void)c; Fixture *f = p; f->item_resets++;
    f->items = (ItemsView){0}; f->items.score_multiplier = 1; f->items.world_scale = 1; f->queued = (GameEventBuffer){0};
}
static void ResetObstacles(void *p, const GameState *s, const WorldConfig *c) { (void)s; (void)c; ((Fixture *)p)->obstacle_resets++; }
static void PlayerStep(void *p, const GameState *s, const FrameContext *c, const GameInput *i, float force)
{ (void)s; (void)c; (void)force; Fixture *f = p; f->steps++; f->impulses += i->impulse_pressed ? 1 : 0; }
static PlayerView Player(void *p) { return ((Fixture *)p)->player; }
static void Hit(void *p) { Fixture *f = p; f->hits++; f->player.invulnerable = true; }
static void Pose(void *p, PortalPose pose) { (void)p; (void)pose; }
static void DrawPlayer(void *p, const GameState *s, const ItemsView *v) { (void)p; (void)s; (void)v; }
static void StepWorld(void *p, const GameState *s, const FrameContext *c) { (void)p; (void)s; (void)c; }
static void Draw(void *p, const GameState *s) { (void)p; (void)s; }
static const WorldConfig *Config(void *p, int i) { (void)p; return i >= 0 && i < 6 ? &configs[i] : NULL; }
static void ObstaclesStep(void *p, const GameState *s, const FrameContext *c, PlayerView v, bool spawn)
{ (void)s; (void)c; (void)v; (void)spawn; ((Fixture *)p)->obstacle_steps++; }
static float Force(void *p) { (void)p; return 0; }
static bool Touch(void *p, PlayerView v) { (void)v; return ((Fixture *)p)->collision; }
static bool Clear(void *p, PlayerView v) { (void)v; return ((Fixture *)p)->clear; }
static unsigned Passed(void *p) { Fixture *f = p; unsigned n = f->passed; f->passed = 0; return n; }
static float SafeY(void *p, float x, float fallback) { (void)p; (void)x; return fallback; }
static bool Safe(void *p, GameVec2 pos, float r) { (void)p; (void)pos; (void)r; return true; }
static PassageQuery Passages(void *p) { return (PassageQuery){p, SafeY, Safe}; }
static void Timers(void *p, const FrameContext *c) { (void)c; ((Fixture *)p)->timer_steps++; }
static ItemsView Items(void *p) { return ((Fixture *)p)->items; }
static bool Shield(void *p) { Fixture *f = p; bool had = f->items.shield; f->items.shield = false; return had; }
static void Collect(void *p, const GameState *s, const FrameContext *c, PlayerView v, PassageQuery q, GameEventBuffer *b)
{
    (void)s; (void)c; (void)v; (void)q; Fixture *f = p; f->collect_steps++; *b = f->queued;
    for (size_t j = 0; j < b->count; j++) if (b->values[j].type == EVENT_FRAGMENT_COLLECTED) f->items.collected++;
    f->queued = (GameEventBuffer){0};
}
static UiDecision Input(void *p, const GameState *s, const GameInput *i) { (void)p; (void)s; (void)i; return (UiDecision){0}; }
static void StepUi(void *p, const GameState *s, const FrameContext *c, const ItemsView *v) { (void)p; (void)s; (void)c; (void)v; }
static void Event(void *p, const GameEvent *e) { (void)p; (void)e; }
static void PortalBegin(void *p, const GameState *s, PlayerView v) { (void)v; MORPH_CHECK(s->mode == GAME_PORTAL); ((Fixture *)p)->portal_begins++; }
static PortalSignal PortalStep(void *p, const GameState *s, const FrameContext *c) { (void)s; (void)c; Fixture *f = p; f->portal_steps++; return f->signal; }
static PortalPose PortalPosition(void *p) { (void)p; return (PortalPose){0}; }
static void Switch(void *p, const GameState *s, const WorldConfig *c) { (void)p; (void)s; (void)c; }

static GameModules Modules(Fixture *f)
{
    GameModules m = {0}; m.contract_version = MORPH_CONTRACT_VERSION;
    ModuleLife life = {f, Init, Reset, Shut};
    m.player = (PlayerApi){life, PlayerStep, Player, Hit, Pose, DrawPlayer}; m.player.life.reset_world = ResetPlayer;
    m.worlds = (WorldsApi){life, StepWorld, Draw};
    m.obstacles = (ObstaclesApi){life, Config, ObstaclesStep, Force, Touch, Clear, Passed, Passages, Draw};
    m.obstacles.life.reset_world = ResetObstacles;
    m.items = (ItemsApi){life, Timers, Items, Shield, Collect, Draw}; m.items.life.reset_world = ResetItems;
    m.ui = (UiApi){life, Input, StepUi, DrawPlayer};
    m.effects = (EffectsApi){life, Event, StepWorld, PortalBegin, PortalStep, PortalPosition, Switch, Draw, Draw, Draw};
    return m;
}
static GameInput Focused(void) { GameInput i = {0}; i.focused = true; return i; }
static void Tick(Game *g) { GameAdvance(g, MORPH_STEP_SECONDS, Focused()); MORPH_CHECK(g->state.mode != GAME_ERROR); }
static void Frame(Game *g, double seconds, GameCommand command)
{ GameInput i = Focused(); i.command = command; GameAdvance(g, seconds, i); MORPH_CHECK(g->state.mode != GAME_ERROR); }
static void Start(Game *g, Fixture *f, bool educational)
{
    memset(f, 0, sizeof(*f)); GameModules m = Modules(f);
    MORPH_CHECK(GameInitMode(g, &m, educational) && g->ready);
    MORPH_CHECK(g->state.edu.enabled == educational);
    Frame(g, 1.0, COMMAND_START); MORPH_CHECK(g->state.mode == GAME_PLAY);
}
static void Fragment(Fixture *f) { GameEvent e = {0}; e.type = EVENT_FRAGMENT_COLLECTED; e.multiplier = 1; MORPH_CHECK(GameEnqueueEvent(&f->queued, e)); }
static void FillWorld(Game *g, Fixture *f) { unsigned n = GameEnergyRequired(g->state.world); for (unsigned k = 0; k < n; k++) { Fragment(f); Tick(g); } }
static const QuizState *Quiz(const Game *g) { return &g->state.edu.quiz; }
static GameCommand Right(const Game *g) { return (GameCommand)(COMMAND_ANSWER_A + QuizCurrent(Quiz(g))->correct); }
static GameCommand Wrong(const Game *g) { return (GameCommand)(COMMAND_ANSWER_A + (QuizCurrent(Quiz(g))->correct + 1) % 4); }
static void PassGuard(Game *g) { for (int i = 0; i < 4; i++) Frame(g, 0.1, COMMAND_NONE); }   /* 0,4 s */
static void Answer(Game *g, GameCommand c) { PassGuard(g); Frame(g, 1.0 / 60, c); MORPH_CHECK(Quiz(g)->phase == QUIZ_FEEDBACK); }
static void Continue(Game *g) { for (int i = 0; i < 6; i++) Frame(g, 0.1, COMMAND_NONE); Frame(g, 1.0 / 60, COMMAND_CONTINUE); }

typedef struct Frozen { int steps, timers, collects, obstacles, portal_steps; uint64_t score, sim; float scroll; double world_steps; int lives; BossState boss; } Frozen;
static Frozen Snapshot(const Game *g, const Fixture *f)
{
    Frozen s = {f->steps, f->timer_steps, f->collect_steps, f->obstacle_steps, f->portal_steps, g->state.score,
                g->state.simulation_steps, g->state.scroll, g->state.world_steps, g->state.lives, g->state.boss};
    return s;
}
static void CheckFrozen(const Game *g, const Fixture *f, Frozen s)
{
    Frozen n = Snapshot(g, f);
    MORPH_CHECK(n.steps == s.steps && n.timers == s.timers && n.collects == s.collects && n.obstacles == s.obstacles);
    MORPH_CHECK(n.portal_steps == s.portal_steps && n.score == s.score && n.sim == s.sim && n.lives == s.lives);
    MORPH_CHECK(n.scroll == s.scroll && n.world_steps == s.world_steps);
    MORPH_CHECK(memcmp(&n.boss, &s.boss, sizeof(n.boss)) == 0);
}

/* 6, 11, 12, 13, 14, 15, 22 */
static void CheckPassage(void)
{
    Game g; Fixture f; Start(&g, &f, true);
    MORPH_CHECK(GameEnergyRequired(0) == 4);
    f.passed = 1; Tick(&g); uint64_t score = g.state.score; MORPH_CHECK(score == 10);
    FillWorld(&g, &f);
    MORPH_CHECK(f.items.collected == 4 && g.state.mode == GAME_PLAY && g.state.score == score + 4 * 35);
    f.clear = false; Tick(&g); MORPH_CHECK(g.state.mode == GAME_PLAY && Quiz(&g)->kind == QUIZ_NONE);
    f.clear = true; Tick(&g);                               /* mesmo instante seguro do portal original */
    MORPH_CHECK(g.state.mode == GAME_QUIZ && Quiz(&g)->kind == QUIZ_PASSAGE && Quiz(&g)->return_mode == GAME_PLAY);
    MORPH_CHECK(f.portal_begins == 0 && Quiz(&g)->seconds_left == 12.0f);
    unsigned serial = Quiz(&g)->serial;
    Frozen frozen = Snapshot(&g, &f);
    GameInput flap = Focused(); flap.impulse_pressed = true; flap.attack_pressed = true; flap.pointer_pressed = flap.pointer_inside = true;
    for (int i = 0; i < 30; i++) GameAdvance(&g, 1.0 / 60, flap);
    f.collision = true; f.passed = 3;                       /* nada disso pode ser processado */
    for (int i = 0; i < 30; i++) Tick(&g);
    CheckFrozen(&g, &f, frozen);
    MORPH_CHECK(f.impulses == 0 && f.hits == 0 && Quiz(&g)->serial == serial);  /* não reabre a cada quadro */
    MORPH_CHECK_NEAR(Quiz(&g)->seconds_left, 11.0, 1e-3);   /* 8: 60 quadros de 1/60 s */
    f.collision = false; f.passed = 0;

    /* Erro: mostra a correta, nova pergunta com 12 s, sem perder vida/letras. */
    int asked = Quiz(&g)->question;
    Answer(&g, Wrong(&g));
    MORPH_CHECK(Quiz(&g)->result == QUIZ_WRONG && g.state.mode == GAME_QUIZ);
    float stopped = Quiz(&g)->seconds_left;
    Frame(&g, 1.0 / 60, COMMAND_CONTINUE);                 /* feedback precisa ser lido */
    MORPH_CHECK(Quiz(&g)->phase == QUIZ_FEEDBACK && Quiz(&g)->seconds_left == stopped);
    Continue(&g);
    MORPH_CHECK(Quiz(&g)->phase == QUIZ_ASKING && Quiz(&g)->question != asked && Quiz(&g)->seconds_left == 12.0f);
    MORPH_CHECK(g.state.mode == GAME_QUIZ && f.portal_begins == 0 && g.state.lives == 3 && f.items.collected == 4);

    /* Tempo esgotado: 12 s inteiros; resposta no último quadro já é tardia. */
    asked = Quiz(&g)->question;
    for (int i = 0; i < 119; i++) Frame(&g, 0.1, COMMAND_NONE);
    MORPH_CHECK(Quiz(&g)->phase == QUIZ_ASKING && Quiz(&g)->seconds_left > 0);
    Frame(&g, 0.1, Right(&g));                              /* 10: mesmo quadro do fim do prazo */
    MORPH_CHECK(Quiz(&g)->result == QUIZ_TIMEOUT && Quiz(&g)->seconds_left == 0);
    Continue(&g);
    MORPH_CHECK(Quiz(&g)->phase == QUIZ_ASKING && Quiz(&g)->question != asked && Quiz(&g)->seconds_left == 12.0f);
    MORPH_CHECK(f.portal_begins == 0 && g.state.lives == 3 && f.items.collected == 4);
    CheckFrozen(&g, &f, frozen);                            /* 22: nada correu durante 3 perguntas */

    /* Acerto dentro do prazo libera o portal original. */
    Answer(&g, Right(&g));
    MORPH_CHECK(Quiz(&g)->result == QUIZ_CORRECT && g.state.mode == GAME_QUIZ && f.portal_begins == 0);
    Continue(&g);
    MORPH_CHECK(g.state.mode == GAME_PORTAL && f.portal_begins == 1 && g.state.edu.portal_unlocked);
    MORPH_CHECK(g.state.score == frozen.score && g.state.lives == 3);
    f.signal = PORTAL_SIGNAL_ENTERED; Tick(&g); MORPH_CHECK(g.state.mode == GAME_TRANSITION && g.state.score == frozen.score + 100);
    f.signal = PORTAL_SIGNAL_BLACKOUT; Tick(&g); MORPH_CHECK(g.state.world == 1 && !g.state.edu.portal_unlocked);
    f.signal = PORTAL_SIGNAL_FADE_DONE; Tick(&g); MORPH_CHECK(g.state.mode == GAME_PLAY);
    f.signal = PORTAL_SIGNAL_NONE;
    GameShutdown(&g);
}

/* 23: pausa e perda de foco congelam o cronômetro; sem salto ao voltar. */
static void CheckPauseAndFocus(void)
{
    Game g; Fixture f; Start(&g, &f, true);
    FillWorld(&g, &f); f.clear = true; Tick(&g); MORPH_CHECK(g.state.mode == GAME_QUIZ);
    Frame(&g, 0.1, COMMAND_NONE); float left = Quiz(&g)->seconds_left;
    MORPH_CHECK_NEAR(left, 11.9, 1e-4);
    Frame(&g, 0.1, COMMAND_TOGGLE_PAUSE);
    MORPH_CHECK(g.state.mode == GAME_PAUSED && g.state.resume_mode == GAME_QUIZ && Quiz(&g)->seconds_left == left);
    for (int i = 0; i < 50; i++) Frame(&g, 0.1, COMMAND_NONE);
    MORPH_CHECK(Quiz(&g)->seconds_left == left);
    Frame(&g, 3.0, Right(&g));                              /* resposta durante a pausa: ignorada */
    MORPH_CHECK(Quiz(&g)->phase == QUIZ_ASKING);
    Frame(&g, 2.0, COMMAND_RESUME);
    MORPH_CHECK(g.state.mode == GAME_QUIZ && Quiz(&g)->seconds_left == left);
    GameInput away = Focused(); away.focused = false;
    GameAdvance(&g, 4.0, away);
    MORPH_CHECK(g.state.mode == GAME_PAUSED && Quiz(&g)->seconds_left == left);
    Frame(&g, 9.0, COMMAND_TOGGLE_PAUSE);
    MORPH_CHECK(g.state.mode == GAME_QUIZ && Quiz(&g)->seconds_left == left);
    Frame(&g, 30.0, COMMAND_NONE);                          /* quadro travado: no máximo 0,1 s */
    MORPH_CHECK_NEAR(Quiz(&g)->seconds_left, left - 0.1, 1e-4);
    Frame(&g, 0.0, COMMAND_RESTART);
    MORPH_CHECK(g.state.mode == GAME_INTRO && Quiz(&g)->kind == QUIZ_NONE && !g.state.edu.revive_used);
    GameShutdown(&g);
}

static void Die(Game *g, Fixture *f)
{
    f->collision = true;
    while (g->state.lives > 0 && g->state.mode == GAME_PLAY) { f->player.invulnerable = false; Tick(g); }
    f->collision = false;
}

/* 16, 17, 18, 19, 20, 21 */
static void CheckRevive(void)
{
    Game g; Fixture f; Start(&g, &f, true);
    FillWorld(&g, &f); f.clear = true; Tick(&g); Answer(&g, Right(&g)); Continue(&g);
    f.signal = PORTAL_SIGNAL_ENTERED; Tick(&g); f.signal = PORTAL_SIGNAL_BLACKOUT; Tick(&g);
    f.signal = PORTAL_SIGNAL_FADE_DONE; Tick(&g); f.signal = PORTAL_SIGNAL_NONE; f.clear = false;
    MORPH_CHECK(g.state.world == 1 && g.state.mode == GAME_PLAY);
    Fragment(&f); Fragment(&f); Tick(&g); MORPH_CHECK(f.items.collected == 2);
    uint64_t score = g.state.score; int item_resets = f.item_resets, obstacle_resets = f.obstacle_resets;
    int hits = f.hits;
    Die(&g, &f);
    MORPH_CHECK(f.hits == hits + 3);
    MORPH_CHECK(g.state.mode == GAME_QUIZ && Quiz(&g)->kind == QUIZ_REVIVE && Quiz(&g)->return_mode == GAME_PLAY);
    MORPH_CHECK(g.state.lives == 0 && Quiz(&g)->seconds_left == 12.0f && !g.state.edu.revive_used);
    Frozen frozen = Snapshot(&g, &f);
    for (int i = 0; i < 20; i++) Tick(&g);
    CheckFrozen(&g, &f, frozen);
    Answer(&g, Right(&g)); Continue(&g);
    MORPH_CHECK(g.state.mode == GAME_PLAY && g.state.lives == 1 && g.state.world == 1);   /* uma vida só */
    MORPH_CHECK(g.state.score == score && f.items.collected == 2 && g.state.edu.revive_used);
    MORPH_CHECK(f.item_resets == item_resets && f.obstacle_resets == obstacle_resets);   /* progresso intacto */
    MORPH_CHECK(f.player.invulnerable && f.player.position.y == 245 && g.state.edu.grace_steps > 0);
    /* Proteção temporária: nenhum dano instantâneo ao retornar. */
    f.collision = true; f.player.touching_floor = true; hits = f.hits;
    for (int i = 0; i < (int)MORPH_REVIVE_GRACE_STEPS - 1; i++) { f.player.invulnerable = false; Tick(&g); }
    MORPH_CHECK(g.state.lives == 1 && f.hits == hits && g.state.mode == GAME_PLAY);
    f.player.invulnerable = false; Tick(&g); Tick(&g);
    /* 20: segunda chance usada -> derrota direta, sem nova pergunta. */
    MORPH_CHECK(g.state.mode == GAME_LOST && g.state.lives == 0 && Quiz(&g)->kind == QUIZ_NONE);
    f.collision = false; f.player.touching_floor = false;
    Frame(&g, 0.1, COMMAND_RETRY);
    MORPH_CHECK(g.state.mode == GAME_PLAY && g.state.lives == 3 && !g.state.edu.revive_used && g.state.world == 0);

    /* 18: erro na ressurreição -> derrota após o feedback; apenas uma pergunta. */
    Die(&g, &f); MORPH_CHECK(g.state.mode == GAME_QUIZ);
    unsigned serial = Quiz(&g)->serial;
    Answer(&g, Wrong(&g));
    MORPH_CHECK(g.state.mode == GAME_QUIZ && Quiz(&g)->result == QUIZ_WRONG && g.state.lives == 0);
    Continue(&g);
    MORPH_CHECK(g.state.mode == GAME_LOST && g.state.lives == 0 && g.state.edu.revive_used);
    MORPH_CHECK(Quiz(&g)->serial == serial + 1);            /* nenhuma segunda pergunta foi aberta */
    Frame(&g, 0.1, COMMAND_RETRY);
    MORPH_CHECK(g.state.lives == 3 && !g.state.edu.revive_used);

    /* 19: tempo esgotado na ressurreição -> derrota. */
    Die(&g, &f); MORPH_CHECK(g.state.mode == GAME_QUIZ);
    for (int i = 0; i < 120; i++) Frame(&g, 0.1, COMMAND_NONE);
    MORPH_CHECK(Quiz(&g)->result == QUIZ_TIMEOUT && g.state.mode == GAME_QUIZ);
    Frame(&g, 0.1, Right(&g));
    MORPH_CHECK(Quiz(&g)->result == QUIZ_TIMEOUT);
    Continue(&g);
    MORPH_CHECK(g.state.mode == GAME_LOST && g.state.lives == 0);
    GameShutdown(&g);
}

/* 25, 26: seis mundos com perguntas, Fogo libera o boss; 17/21/24 no combate. */
static void CheckCampaignAndBoss(void)
{
    Game g; Fixture f; Start(&g, &f, true);
    static const unsigned targets[6] = {4, 6, 8, 10, 12, 16};
    uint64_t expected = 0;
    for (int world = 0; world < 6; world++) {
        MORPH_CHECK(g.state.world == world && GameEnergyRequired(world) == targets[world]);   /* 3 */
        FillWorld(&g, &f); expected += 35u * targets[world];
        MORPH_CHECK(g.state.score == expected && g.state.mode == GAME_PLAY);
        f.clear = true; Tick(&g); MORPH_CHECK(g.state.mode == GAME_QUIZ);
        if (world == 2) { Answer(&g, Wrong(&g)); Continue(&g); MORPH_CHECK(g.state.mode == GAME_QUIZ); }
        Answer(&g, Right(&g)); Continue(&g);
        MORPH_CHECK(g.state.mode == GAME_PORTAL);
        f.signal = PORTAL_SIGNAL_ENTERED; Tick(&g); expected += 100;
        f.signal = PORTAL_SIGNAL_BLACKOUT; Tick(&g);
        if (world == 5) break;
        f.signal = PORTAL_SIGNAL_FADE_DONE; Tick(&g); f.signal = PORTAL_SIGNAL_NONE; f.clear = false;
    }
    MORPH_CHECK(g.state.mode == GAME_BOSS && g.state.mode != GAME_WON);   /* acerto no Fogo != vitória */
    MORPH_CHECK(g.state.boss.segments == 3 && g.state.score == expected && g.state.lives == 3);
    for (int i = 0; i < 180; i++) Tick(&g);
    MORPH_CHECK(g.state.boss.stage == BOSS_FIGHT);
    /* Primeiro raio: o boss fica com dois segmentos. */
    g.state.boss.energy.collected = g.state.boss.energy.required;
    GameInput attack = Focused(); attack.attack_pressed = true; GameAdvance(&g, MORPH_STEP_SECONDS, attack);
    MORPH_CHECK(g.state.boss.beam_active);
    for (int i = 0; i < 85; i++) Tick(&g);
    MORPH_CHECK(g.state.boss.segments == 2 && !g.state.boss.beam_active);
    /* Ressurreição no combate. */
    g.state.lives = 1; f.player.invulnerable = false; f.player.touching_floor = true;
    Tick(&g);
    MORPH_CHECK(g.state.mode == GAME_QUIZ && Quiz(&g)->return_mode == GAME_BOSS && g.state.lives == 0);
    f.player.touching_floor = false;
    Frozen frozen = Snapshot(&g, &f);
    for (int i = 0; i < 40; i++) Tick(&g);
    CheckFrozen(&g, &f, frozen);                            /* boss congelado */
    Answer(&g, Right(&g)); Continue(&g);
    MORPH_CHECK(g.state.mode == GAME_BOSS && g.state.lives == 1 && g.state.boss.segments == 2);
    MORPH_CHECK(g.state.boss.stage == BOSS_FIGHT && g.state.edu.grace_steps > 0 && f.player.invulnerable);
    g.state.boss.projectile_count = 1;                      /* tiro em cima do jogador ao voltar */
    g.state.boss.projectiles[0] = (BossProjectile){.position = f.player.position, .previous = f.player.position, .radius = 12};
    f.player.invulnerable = false; Tick(&g);
    MORPH_CHECK(g.state.lives == 1 && g.state.mode == GAME_BOSS);
    g.state.boss.projectile_count = 0;
    /* 24: energia de combate continua (fragmento exclusivo, 35 pontos). */
    uint64_t score = g.state.score; unsigned charge = g.state.boss.energy.collected;
    g.state.boss.energy.present = true; g.state.boss.energy.position = f.player.position;
    Tick(&g);
    MORPH_CHECK(g.state.boss.energy.collected == charge + 1 && g.state.score == score + 35 && f.items.collected == 16);
    for (unsigned shot = 0; shot < 2; shot++) {
        MORPH_CHECK(g.state.mode == GAME_BOSS);
        g.state.boss.energy.collected = g.state.boss.energy.required;
        GameAdvance(&g, MORPH_STEP_SECONDS, attack);
        for (int i = 0; i < 85; i++) Tick(&g);
    }
    MORPH_CHECK(g.state.boss.segments == 0 && g.state.mode == GAME_BOSS);  /* desintegração ainda em curso */
    for (int i = 0; i < 220 && g.state.mode == GAME_BOSS; i++) Tick(&g);
    MORPH_CHECK(g.state.mode == GAME_WON);                  /* vitória só após derrotar o boss */
    GameShutdown(&g);
}

/* 27: o mesmo coordenador com edu desligado se comporta como o arcade. */
static void CheckArcade(void)
{
    Game g; Fixture f; Start(&g, &f, false);
    MORPH_CHECK(!g.state.edu.enabled);
    FillWorld(&g, &f); f.clear = true; Tick(&g);
    MORPH_CHECK(g.state.mode == GAME_PORTAL && f.portal_begins == 1 && Quiz(&g)->kind == QUIZ_NONE);
    Frame(&g, 0.1, COMMAND_ANSWER_A); Frame(&g, 0.1, COMMAND_CONTINUE);
    MORPH_CHECK(g.state.mode == GAME_PORTAL);
    Frame(&g, 0.0, COMMAND_RESTART); Frame(&g, 0.0, COMMAND_START);
    Die(&g, &f);
    MORPH_CHECK(g.state.mode == GAME_LOST && g.state.lives == 0 && Quiz(&g)->kind == QUIZ_NONE);
    MORPH_CHECK(g.state.edu.grace_steps == 0 && !g.state.edu.revive_used);
    GameShutdown(&g);
    MORPH_CHECK(GameEducationalBuild() == (MORPH_EDUCATIONAL != 0));
    MORPH_CHECK(strcmp(GameModeName(GAME_QUIZ), "QUIZ") == 0);
}

int main(void)
{
    CheckPassage();
    CheckPauseAndFocus();
    CheckRevive();
    CheckCampaignAndBoss();
    CheckArcade();
    printf("PASS edu-flow (build educativo=%d): portal exige acerto, 12 s por pergunta, simulação congelada, "
           "pausa/foco, segunda chance única, boss preservado e arcade sem perguntas\n", MORPH_EDUCATIONAL);
    return 0;
}
