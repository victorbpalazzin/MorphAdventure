/* Módulos reais (items, ui, obstacles) no modo educativo, sem janela. */
#include "items/items_internal.h"
#include "ui/ui_internal.h"
#include "obstacles/obstacles.h"
#include "core/game.h"
#include "edu/quiz.h"
#include "morph_check.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

static float SafeY(void *ctx, float x, float y) { (void)ctx; (void)x; return y; }
static bool Safe(void *ctx, GameVec2 p, float r) { (void)ctx; (void)p; (void)r; return true; }

typedef struct Trace { unsigned collected, spawned, missed; uint64_t points; GameEvent log[256]; unsigned count; } Trace;

/* Mesmo roteiro de voo nos dois modos; compara tudo o que a coleta produz. */
static Trace Run(int world, bool educational, ItemsSnapshot *final)
{
    ItemsApi api = ItemsModuleCreate();
    MORPH_CHECK(api.life.init(api.life.context));
    ItemsSetSeed(2026);
    GameState game = {.mode = GAME_PLAY, .world = world, .lives = 3, .run_id = 9};
    game.edu.enabled = educational;
    WorldConfig cfg = {0}; cfg.duration_steps = 1500 + world * 600;
    api.life.reset_world(api.life.context, &game, &cfg);
    PassageQuery query = {NULL, SafeY, Safe};
    FrameContext frame = {.normal_steps = 1, .world_steps = 1, .normal_seconds = 1.0f / 60, .world_seconds = 1.0f / 60,
                          .world_scale = 1, .pace = 1, .scroll_delta = 4.6f};
    Trace t = {0};
    for (int step = 0; step < 9000 && t.collected < GameEnergyRequired(world); step++) {
        /* Piloto determinístico: segue a letra mais próxima, mas deixa passar
         * de propósito em janelas periódicas para exercitar o reaparecimento. */
        ItemsSnapshot view = ItemsInspect();
        float y = 60;
        bool skip = (step / 400) % 3 == 1;
        for (unsigned i = 0; i < view.fragment_count && !skip; i++)
            if (view.fragments[i].fade == 0) { y = view.fragments[i].position.y; break; }
        PlayerView player = {.position = {180, y}, .visual_radius = 23, .collision_radius = 18};
        GameEventBuffer events = {0};
        ItemsSnapshot before = ItemsInspect();
        api.step_timers(api.life.context, &frame);
        api.collect_step(api.life.context, &game, &frame, player, query, &events);
        MORPH_CHECK(!events.overflow);
        ItemsSnapshot after = ItemsInspect();
        if (after.fragment_count > before.fragment_count) t.spawned++;
        for (size_t i = 0; i < events.count; i++) {
            if (events.values[i].type == EVENT_FRAGMENT_COLLECTED) { t.collected++; t.points += 35u * events.values[i].multiplier; }
            if (t.count < 256) t.log[t.count++] = events.values[i];
        }
    }
    t.missed = t.spawned - t.collected;
    *final = ItemsInspect();
    api.life.shutdown(api.life.context);
    return t;
}

/* 1, 2, 3, 4, 5 */
static void CheckLettersAndCollection(void)
{
    static const unsigned targets[6] = {4, 6, 8, 10, 12, 16};
    unsigned missed_total = 0;
    for (int w = 0; w < MORPH_WORLD_COUNT; w++) {
        MORPH_CHECK(GameEnergyRequired(w) == targets[w]);
        const char *set = ItemsWorldLetters(w);
        MORPH_CHECK(strlen(set) == targets[w]);
        GameState arcade = {.world = w}, edu = {.world = w};
        edu.edu.enabled = true;
        for (unsigned id = 0; id < targets[w]; id++) {
            MORPH_CHECK(ItemsFragmentLetter(&arcade, id) == '\0');      /* raio original */
            char c = ItemsFragmentLetter(&edu, id);
            MORPH_CHECK(c == set[id] && isupper((unsigned char)c));
        }
        ItemsSnapshot a, e;
        Trace ta = Run(w, false, &a), te = Run(w, true, &e);
        MORPH_CHECK(ta.collected == targets[w] && te.collected == targets[w]);
        MORPH_CHECK(ta.points == te.points && ta.points == 35u * targets[w]);
        MORPH_CHECK(ta.spawned == te.spawned && ta.count == te.count);
        MORPH_CHECK(memcmp(ta.log, te.log, sizeof(ta.log)) == 0);    /* mesma ordem, IDs e posições */
        MORPH_CHECK(memcmp(&a, &e, sizeof(a)) == 0);
        missed_total += ta.missed;
    }
    MORPH_CHECK(missed_total > 0);  /* o roteiro também perde letras, que reaparecem */
    GameState none = {.world = 9}; none.edu.enabled = true;
    MORPH_CHECK(ItemsFragmentLetter(&none, 0) == '\0' && ItemsFragmentLetter(NULL, 0) == '\0');
}

static void CheckHud(void)
{
    char text[64];
    GameState g = {.world = 1};
    UiProgressLabel(&g, 3, 6, 50, text, sizeof(text));
    MORPH_CHECK(strcmp(text, "ENERGIA  3/6  50%") == 0);                /* arcade intacto */
    g.edu.enabled = true;
    UiProgressLabel(&g, 3, 6, 50, text, sizeof(text));
    MORPH_CHECK(strcmp(text, "LETRAS: 3/6") == 0);
    g.boss.stage = BOSS_FIGHT;                                         /* combate mantém a energia */
    UiProgressLabel(&g, 2, 4, 50, text, sizeof(text));
    MORPH_CHECK(strcmp(text, "ENERGIA  2/4  50%") == 0);
}

static bool Fits(GameRect r) { return r.x >= 0 && r.y >= 0 && r.x + r.width <= MORPH_WIDTH && r.y + r.height <= MORPH_HEIGHT; }

static void CheckQuizInput(void)
{
    UiApi api = UiModuleCreate();
    MORPH_CHECK(api.life.init(api.life.context));
    GameState g = {.mode = GAME_PLAY};
    g.edu.enabled = true;
    api.life.reset_world(api.life.context, &g, NULL);
    QuizSeed(&g.edu.quiz, 5);
    QuizBegin(&g.edu.quiz, QUIZ_PASSAGE, GAME_PLAY);
    g.mode = GAME_QUIZ;
    UiButton b[4];
    for (int kind = QUIZ_PASSAGE; kind <= QUIZ_REVIVE; kind++) {
        g.edu.quiz.kind = (QuizKind)kind;
        MORPH_CHECK(UiQuizButtons(&g.edu.quiz, b) == 4);
        for (int i = 0; i < 4; i++) {
            MORPH_CHECK(Fits(b[i].rect) && (int)b[i].command == (int)COMMAND_ANSWER_A + i);
            if (i) MORPH_CHECK(b[i].rect.y >= b[i - 1].rect.y + b[i - 1].rect.height);
        }
    }
    g.edu.quiz.kind = QUIZ_PASSAGE;
    GameInput in = {.focused = true, .impulse_pressed = true};
    /* Espaço sem foco (o jogador vinha voando) não responde. */
    MORPH_CHECK(api.input(api.life.context, &g, &in).command == COMMAND_NONE);
    in.impulse_pressed = false;
    in.pointer_pressed = in.pointer_inside = true;
    in.pointer = (GameVec2){b[2].rect.x + 40, b[2].rect.y + 10};
    UiDecision d = api.input(api.life.context, &g, &in);
    MORPH_CHECK(d.command == COMMAND_ANSWER_C && d.consumes_pointer);
    in.pointer = (GameVec2){20, 20};
    d = api.input(api.life.context, &g, &in);
    MORPH_CHECK(d.command == COMMAND_NONE && d.consumes_pointer);   /* clique no painel não vira impulso */
    in.pointer_pressed = false;
    g.edu.quiz.serial++;                                            /* nova pergunta: foco limpo */
    UiQueueNavigation(1, false);
    MORPH_CHECK(api.input(api.life.context, &g, &in).command == COMMAND_NONE);
    UiQueueNavigation(1, true);
    MORPH_CHECK(api.input(api.life.context, &g, &in).command == COMMAND_ANSWER_B);
    UiQueueNavigation(-1, false); UiQueueNavigation(0, true);
    MORPH_CHECK(api.input(api.life.context, &g, &in).command == COMMAND_ANSWER_A);
    /* Feedback: um botão de continuar, Espaço ativa. */
    g.edu.quiz.phase = QUIZ_FEEDBACK; g.edu.quiz.result = QUIZ_WRONG; g.edu.quiz.serial++;
    MORPH_CHECK(UiQuizButtons(&g.edu.quiz, b) == 1 && b[0].command == COMMAND_CONTINUE && Fits(b[0].rect));
    in.impulse_pressed = true;
    MORPH_CHECK(api.input(api.life.context, &g, &in).command == COMMAND_CONTINUE);
    in.focused = false;
    MORPH_CHECK(api.input(api.life.context, &g, &in).command == COMMAND_NONE);
    /* Sem desafio aberto, os botões do arcade continuam os mesmos. */
    UiButton arcade[2];
    MORPH_CHECK(UiButtons(GAME_PLAY, arcade) == 1 && arcade[0].command == COMMAND_TOGGLE_PAUSE);
    MORPH_CHECK(UiButtons(GAME_QUIZ, arcade) == 0);
    api.life.shutdown(api.life.context);
}

static void CheckHazardInterval(void)
{
    for (int w = 0; w < MORPH_WORLD_COUNT; w++) {
        const ObstacleHazardConfig *h = ObstaclesHazardSettings(w);
        MORPH_CHECK(h && h->interval == 510);                       /* 8,5 s a 60 passos/s */
        MORPH_CHECK_NEAR(h->interval * MORPH_STEP_SECONDS, 8.5, 1e-9);
    }
}

int main(void)
{
    CheckLettersAndCollection();
    CheckHud();
    CheckQuizInput();
    CheckHazardInterval();
    puts("PASS edu-modules: letras só no educativo, coleta/IDs/pontos idênticos ao arcade, HUD LETRAS, "
         "entrada do desafio sem conflito com o voo, perigos a cada 8,5 s");
    return 0;
}
