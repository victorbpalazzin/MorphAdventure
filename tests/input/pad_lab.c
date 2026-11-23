/* Controle simulado sobre TODOS os módulos reais e o coordenador real, na versão
 * educativa (a única que lê o controle). Cada quadro passa pelo mesmo caminho do main.c:
 * PadUpdate -> PadApply -> UiQueueNavigation -> GameAdvance. Nenhuma tecla ou
 * clique é usado. Preparações explícitas: energia completa e arena do boss
 * (como em ui_lab.c). */
#include "raylib.h"
#include "core/game.h"
#include "boss/boss.h"
#include "edu/quiz.h"
#include "input/gamepad.h"
#include "items/items_internal.h"
#include "ui/ui_internal.h"
#include "morph_check.h"
#include <string.h>

#define DT (1.0 / 60)
GameModules MorphCreateModules(void);
static Game game;
static PadState pad;
static RenderTexture2D canvas;
static const char *directory;
static bool pilot_pressed;
static float min_velocity_seen;

static PadSnapshot Pad(void) { PadSnapshot p = {0}; p.connected = true; p.id = 0; return p; }
static PlayerView Player(void) { return game.modules.player.view(game.modules.player.life.context); }
static UiState *Ui(void) { return game.modules.ui.life.context; }
static const QuizState *Quiz(void) { return &game.state.edu.quiz; }

static void Frame(PadSnapshot p)
{
    GameInput input = {.focused = true};
    int navigation = 0; bool activate = false;
    PadActions a = PadUpdate(&pad, p, DT);
    PadApply(a, game.state.mode, &input, &navigation, &activate);
    if (navigation || activate) UiQueueNavigation(navigation, activate);
    UiSetGamepadHints(pad.connected);
    GameAdvance(&game, DT, input);
    MORPH_CHECK(game.state.mode != GAME_ERROR);
    float v = Player().vertical_velocity;
    if (v < min_velocity_seen) min_velocity_seen = v;
}
static void Idle(int frames) { for (int i = 0; i < frames; i++) Frame(Pad()); }
static void Tap(PadButton b) { PadSnapshot p = Pad(); p.down[b] = true; Frame(p); Frame(Pad()); }
/* Piloto: apenas X, como um jogador; solta entre toques. */
static void Pilot(int frames)
{
    for (int i = 0; i < frames; i++) {
        PlayerView v = Player();
        bool press = !pilot_pressed && v.position.y > 280 && v.vertical_velocity > 0;
        PadSnapshot p = Pad(); p.down[PAD_CROSS] = press; pilot_pressed = press;
        Frame(p);
    }
}
static void Capture(const char *name)
{
    GameState before = game.state;
    BeginTextureMode(canvas); ClearBackground(BLACK); GameDraw(&game); EndTextureMode();
    MORPH_CHECK(memcmp(&before, &game.state, sizeof(before)) == 0);
    Image img = LoadImageFromTexture(canvas.texture); MORPH_CHECK(IsImageValid(img)); ImageFlipVertical(&img);
    char path[2048]; snprintf(path, sizeof(path), "%s/%s.png", directory, name);
    MORPH_CHECK(ExportImage(img, path)); UnloadImage(img);
}
static void SetEnergy(unsigned count)
{
    ItemsState *s = game.modules.items.life.context; s->collected = count;
    for (unsigned i = 0; i < s->required; i++) s->collected_ids[i] = i < count;
}
static void Init(bool educational)
{
    GameModules modules = MorphCreateModules();
    MORPH_CHECK(GameInitMode(&game, &modules, educational) && game.ready);
    PadReset(&pad);
    Frame(Pad()); /* controle conectado */
    MORPH_CHECK(pad.connected && UiGamepadHints());
}
/* X segurado ao voltar ao jogo não pode virar impulso. */
static void HoldCrossWithoutFlap(int frames)
{
    PadSnapshot held = Pad(); held.down[PAD_CROSS] = true;
    min_velocity_seen = 99;
    for (int i = 0; i < frames; i++) Frame(held);
    MORPH_CHECK(min_velocity_seen > -6.0f);
    Frame(Pad());
}

static void FocusOption(int option)
{
    for (int i = 0; i < 8 && Ui()->quiz_focus != option; i++) Tap(PAD_DOWN);
    MORPH_CHECK(Ui()->quiz_focus == option);
}
static void FocusCorrect(void) { FocusOption(QuizCurrent(Quiz())->correct); }
/* Segunda chance recusada pelo controle: resposta errada e X para ver a derrota. */
static void LoseRevive(void)
{
    MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->kind == QUIZ_REVIVE);
    Idle(25); FocusOption((QuizCurrent(Quiz())->correct + 1) % 4); Tap(PAD_CROSS);
    MORPH_CHECK(Quiz()->result == QUIZ_WRONG);
    Idle(31); Tap(PAD_CROSS);
}

/* Telas comuns: menu, voo, pausa, derrota, portal, batalha final e vitória. */
static void Screens(void)
{
    Init(true);
    MORPH_CHECK(game.state.mode == GAME_INTRO);
    Capture("pad-01-inicio");
    Tap(PAD_CROSS);                                                     /* menu inicial */
    MORPH_CHECK(game.state.mode == GAME_PLAY);
    Idle(20);
    PadSnapshot x = Pad(); x.down[PAD_CROSS] = true; Frame(x);          /* voo */
    MORPH_CHECK(Player().vertical_velocity < -5.5f);
    Frame(Pad());
    /* Pausa, navegação e Círculo. */
    Tap(PAD_OPTIONS); MORPH_CHECK(game.state.mode == GAME_PAUSED);
    Capture("pad-02-pausa");
    Tap(PAD_DOWN); MORPH_CHECK(UiInspect().focus == 1);
    Tap(PAD_UP); MORPH_CHECK(UiInspect().focus == 0);
    Tap(PAD_CIRCLE); MORPH_CHECK(game.state.mode == GAME_PLAY);
    Tap(PAD_OPTIONS); MORPH_CHECK(game.state.mode == GAME_PAUSED);
    Tap(PAD_OPTIONS); MORPH_CHECK(game.state.mode == GAME_PLAY);        /* OPTIONS retoma */
    /* X confirma "Continuar" e continua segurado: nenhum impulso acidental. */
    Tap(PAD_OPTIONS);
    PadSnapshot held = Pad(); held.down[PAD_CROSS] = true; Frame(held);
    MORPH_CHECK(game.state.mode == GAME_PLAY);
    HoldCrossWithoutFlap(40);
    Frame(x); MORPH_CHECK(Player().vertical_velocity < -5.5f);         /* novo toque voa */
    Frame(Pad());
    /* Derrota: Círculo volta ao início; X tenta novamente. */
    game.state.lives = 1;
    for (int i = 0; i < 600 && game.state.mode == GAME_PLAY; i++) Frame(Pad());
    LoseRevive();
    MORPH_CHECK(game.state.mode == GAME_LOST);
    Tap(PAD_CIRCLE); MORPH_CHECK(game.state.mode == GAME_INTRO);
    Tap(PAD_CROSS); MORPH_CHECK(game.state.mode == GAME_PLAY);
    game.state.lives = 1;
    for (int i = 0; i < 600 && game.state.mode == GAME_PLAY; i++) Frame(Pad());
    LoseRevive();
    MORPH_CHECK(game.state.mode == GAME_LOST);
    Tap(PAD_CROSS); MORPH_CHECK(game.state.mode == GAME_PLAY && game.state.lives == 3);
    /* Portal com energia completa, guiado pelo piloto. */
    SetEnergy(GameEnergyRequired(0));
    for (int i = 0; i < 900 && game.state.mode == GAME_PLAY; i++) Pilot(1);
    MORPH_CHECK(game.state.mode == GAME_QUIZ);
    Idle(25); FocusCorrect(); Tap(PAD_CROSS); Idle(31); Tap(PAD_CROSS);
    MORPH_CHECK(game.state.mode == GAME_PORTAL);
    for (int i = 0; i < 400 && game.state.mode != GAME_PLAY; i++) Frame(Pad());
    MORPH_CHECK(game.state.world == 1 && game.state.mode == GAME_PLAY);
    /* Batalha final: Quadrado dispara o megarraio. */
    game.state.world = 5; game.state.mode = GAME_BOSS;
    game.config = game.modules.obstacles.config(game.modules.obstacles.life.context, 5);
    game.modules.player.life.reset_world(game.modules.player.life.context, &game.state, game.config);
    game.modules.obstacles.life.reset_world(game.modules.obstacles.life.context, &game.state, game.config);
    game.modules.effects.life.reset_world(game.modules.effects.life.context, &game.state, game.config);
    BossBegin(&game.state.boss, 4);
    Pilot(180); MORPH_CHECK(game.state.boss.stage == BOSS_FIGHT);
    Tap(PAD_SQUARE); MORPH_CHECK(!game.state.boss.beam_active);         /* sem energia, nada */
    Capture("pad-03-boss");
    for (unsigned shot = 0; shot < 3; shot++) {
        game.state.boss.energy.collected = game.state.boss.energy.required;
        Tap(PAD_SQUARE); MORPH_CHECK(game.state.boss.beam_active);
        Pilot(90);
        MORPH_CHECK(game.state.boss.segments == 2 - shot);
    }
    for (int i = 0; i < 260 && game.state.mode == GAME_BOSS; i++) Frame(Pad());
    MORPH_CHECK(game.state.mode == GAME_WON);
    Tap(PAD_DOWN); MORPH_CHECK(UiInspect().focus == 1);
    Tap(PAD_CROSS); MORPH_CHECK(game.state.mode == GAME_INTRO);         /* "Menu Principal" */
    GameShutdown(&game);
}

static void Educational(void)
{
    Init(true);
    Tap(PAD_CROSS); MORPH_CHECK(game.state.mode == GAME_PLAY);
    SetEnergy(GameEnergyRequired(0));
    for (int i = 0; i < 900 && game.state.mode == GAME_PLAY; i++) Pilot(1);
    MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->kind == QUIZ_PASSAGE && Quiz()->phase == QUIZ_ASKING);
    /* X do piloto (ou um X perdido) sem alternativa escolhida não responde. */
    Tap(PAD_CROSS); Idle(30); Tap(PAD_CROSS);
    MORPH_CHECK(Quiz()->phase == QUIZ_ASKING && Ui()->quiz_focus == -1);
    MORPH_CHECK_NEAR(Quiz()->seconds_left, 12 - 34 * DT, 1e-3);      /* cronômetro corre normalmente */
    /* Analógico segurado 1 s: repetição controlada, sem rajada. */
    PadSnapshot stick = Pad(); stick.stick_y = 1;
    int steps = 0;
    for (int i = 0; i < 60; i++) {
        int before = Ui()->quiz_focus; Frame(stick);
        if (Ui()->quiz_focus != before) steps++;
    }
    Frame(Pad());
    MORPH_CHECK(steps == 5);                            /* 0 s, 0,40 s, 0,58 s, 0,76 s e 0,94 s */
    MORPH_CHECK(Quiz()->phase == QUIZ_ASKING);
    Capture("pad-04-pergunta");
    /* Pausa por OPTIONS congela o cronômetro; Círculo volta à pergunta. */
    float left = Quiz()->seconds_left;
    Tap(PAD_OPTIONS); MORPH_CHECK(game.state.mode == GAME_PAUSED);
    Idle(120); MORPH_CHECK(Quiz()->seconds_left == left);
    PadSnapshot circle = Pad(); circle.down[PAD_CIRCLE] = true; Frame(circle);
    MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->seconds_left == left); /* sem salto ao voltar */
    Frame(Pad()); MORPH_CHECK_NEAR(Quiz()->seconds_left, left - DT, 1e-5);
    /* Resposta errada pelo controle, X continua, nova pergunta com 12 s. */
    int wrong = (QuizCurrent(Quiz())->correct + 1) % 4;
    for (int i = 0; i < 8 && Ui()->quiz_focus != wrong; i++) Tap(PAD_DOWN);
    Tap(PAD_CROSS);
    MORPH_CHECK(Quiz()->phase == QUIZ_FEEDBACK && Quiz()->result == QUIZ_WRONG && Quiz()->selected == wrong);
    Tap(PAD_CROSS); MORPH_CHECK(Quiz()->phase == QUIZ_FEEDBACK);        /* proteção de 0,5 s */
    Idle(30); Tap(PAD_CROSS);
    MORPH_CHECK(Quiz()->phase == QUIZ_ASKING && Quiz()->seconds_left > 11.9f && Ui()->quiz_focus == -1);
    /* Acerto e continuar com X segurado: o portal abre e o voo não dispara. */
    Idle(25); FocusCorrect(); Tap(PAD_CROSS);
    MORPH_CHECK(Quiz()->result == QUIZ_CORRECT);
    Idle(31);
    PadSnapshot held = Pad(); held.down[PAD_CROSS] = true; Frame(held);
    MORPH_CHECK(game.state.mode == GAME_PORTAL);
    min_velocity_seen = 99;
    for (int i = 0; i < 400 && game.state.mode != GAME_PLAY; i++) Frame(held);
    MORPH_CHECK(game.state.mode == GAME_PLAY && game.state.world == 1);
    HoldCrossWithoutFlap(30);
    /* Segunda chance pelo controle. */
    uint64_t score = game.state.score;
    game.state.lives = 1;
    for (int i = 0; i < 600 && game.state.mode == GAME_PLAY; i++) Frame(Pad());
    MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->kind == QUIZ_REVIVE);
    Idle(25); FocusCorrect(); Tap(PAD_CROSS);
    MORPH_CHECK(Quiz()->result == QUIZ_CORRECT);
    Idle(31); Tap(PAD_CROSS);
    MORPH_CHECK(game.state.mode == GAME_PLAY && game.state.lives == 1 && game.state.score == score);
    /* Controle desconectado no meio do jogo: teclado/mouse seguem; ao voltar, sem disparo. */
    PadSnapshot gone = {0}; gone.id = -1;
    Frame(gone); MORPH_CHECK(!pad.connected && !UiGamepadHints());
    GameInput key = {.focused = true, .command = COMMAND_TOGGLE_PAUSE};
    GameAdvance(&game, DT, key); MORPH_CHECK(game.state.mode == GAME_PAUSED);
    PadSnapshot back = Pad(); back.down[PAD_OPTIONS] = true;
    Frame(back); MORPH_CHECK(game.state.mode == GAME_PAUSED);           /* já apertado ao conectar */
    Frame(Pad()); Tap(PAD_OPTIONS); MORPH_CHECK(game.state.mode == GAME_PLAY);
    GameShutdown(&game);
}

int main(int argc, char **argv)
{
    MORPH_CHECK(argc == 3 && strcmp(argv[1], "--capture") == 0);
    directory = argv[2];
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(960, 540, "Morph Adventure - controle"); MORPH_CHECK(IsWindowReady());
    canvas = LoadRenderTexture(960, 540); MORPH_CHECK(IsRenderTextureValid(canvas));
    ItemsSetSeed(11); GameSetQuizSeed(42);
    MORPH_CHECK(PadSupported() == GameEducationalBuild());
    Screens();
    Educational();
    UnloadRenderTexture(canvas); CloseWindow();
    puts("PASS input-pad-flow (versão educativa): controle no menu, voo, pausa, Círculo, derrota, portal, megarraio, vitória "
         "e perguntas, repetição do analógico, segunda chance, X sem impulso acidental e reconexão (educativo)");
    return 0;
}
