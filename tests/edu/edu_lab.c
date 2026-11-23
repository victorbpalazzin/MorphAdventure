/* Modo educativo com TODOS os módulos reais e o coordenador real.
 * Percorre os seis mundos respondendo pelo mesmo caminho de entrada do jogo
 * (UiApi.input + comandos), usa a segunda chance na campanha e no boss e
 * grava capturas em 960x540. Preparações explícitas: energia e posição de
 * letras (como em ui_lab.c); nada altera vidas, portal ou cronômetro. */
#include "raylib.h"
#include "core/game.h"
#include "boss/boss.h"
#include "edu/quiz.h"
#include "items/items_internal.h"
#include "player/player.h"
#include "effects/effects.h"
#include "ui/ui.h"
#include "ui/ui_internal.h"
#include "morph_check.h"
#include <string.h>

GameModules MorphCreateModules(void);
static Game game;
static RenderTexture2D canvas;
static const char *directory;

static void Advance(double seconds, GameInput input)
{
    GameAdvance(&game, seconds, input);
    MORPH_CHECK(game.state.mode != GAME_ERROR);
}
static void Command(GameCommand c) { GameInput i = {.focused = true, .command = c}; Advance(0, i); }
static void Tick(void) { GameInput i = {.focused = true}; Advance(MORPH_STEP_SECONDS, i); }
/* Piloto de teste: entrada comum (impulso), sem editar o estado do jogador. */
static void Fly(void)
{
    PlayerView p = game.modules.player.view(game.modules.player.life.context);
    GameInput i = {.focused = true, .impulse_pressed = p.position.y > 265 && p.vertical_velocity > 0};
    Advance(MORPH_STEP_SECONDS, i);
}
static void Wait(double seconds) { GameInput i = {.focused = true}; for (double t = 0; t < seconds - 1e-9; t += 1.0 / 60) Advance(1.0 / 60, i); }
static void Render(void)
{
    GameState before = game.state;
    BeginTextureMode(canvas); ClearBackground(BLACK); GameDraw(&game); EndTextureMode();
    MORPH_CHECK(memcmp(&before, &game.state, sizeof(before)) == 0);  /* desenho somente leitura */
}
static void Capture(const char *name)
{
    Render();
    Image img = LoadImageFromTexture(canvas.texture); MORPH_CHECK(IsImageValid(img)); ImageFlipVertical(&img);
    char path[2048]; snprintf(path, sizeof(path), "%s/%s.png", directory, name);
    MORPH_CHECK(ExportImage(img, path)); UnloadImage(img);
}
static void SetEnergy(unsigned count)
{
    ItemsState *s = game.modules.items.life.context; s->collected = count;
    for (unsigned i = 0; i < s->required; i++) s->collected_ids[i] = i < count;
}
static const QuizState *Quiz(void) { return &game.state.edu.quiz; }
/* Clique real sobre o botão da alternativa, como o mouse faria. */
static void Click(int option)
{
    UiButton b[4]; MORPH_CHECK(UiQuizButtons(Quiz(), b) > (unsigned)option);
    GameInput in = {.focused = true, .pointer_pressed = true, .pointer_inside = true,
                    .pointer = {b[option].rect.x + 60, b[option].rect.y + b[option].rect.height / 2}};
    Advance(1.0 / 60, in);
}
static void Answer(bool right)
{
    Wait(0.5);
    int correct = QuizCurrent(Quiz())->correct;
    Click(right ? correct : (correct + 1) % 4);
    MORPH_CHECK(Quiz()->phase == QUIZ_FEEDBACK && Quiz()->result == (right ? QUIZ_CORRECT : QUIZ_WRONG));
}
static void Continue(void)
{
    Wait(0.6);
    GameInput in = {.focused = true, .impulse_pressed = true}; /* Espaço ativa "continuar" */
    Advance(1.0 / 60, in);
}
static void ShowLetters(void)
{
    ItemsState *s = game.modules.items.life.context;
    s->fragment_count = 4;
    for (unsigned i = 0; i < 4; i++) s->fragments[i] = (ItemFragmentView){i, {330 + i * 150.0f, 210 + (i % 2) * 90.0f}, 23, .4f * i, 0};
}

static void Campaign(void)
{
    Command(COMMAND_START); MORPH_CHECK(game.state.mode == GAME_PLAY && game.state.edu.enabled);
    ShowLetters(); SetEnergy(1); Wait(0.5);
    Capture("edu-01-letras-hud");
    for (int world = 0; world < MORPH_WORLD_COUNT; world++) {
        MORPH_CHECK(game.state.world == world && game.state.mode == GAME_PLAY && game.state.lives > 0);
        SetEnergy(GameEnergyRequired(world));
        for (int i = 0; i < 900 && game.state.mode == GAME_PLAY; i++) Fly(); /* espera o caminho livre */
        MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->kind == QUIZ_PASSAGE && Quiz()->seconds_left == 12.0f);
        MORPH_CHECK(EffectsInspect().phase == 0); /* portal ainda não começou */
        if (world == 0) {
            Wait(4.2); Capture("edu-02-pergunta");
            Answer(false); Capture("edu-03-erro");
            Continue(); MORPH_CHECK(Quiz()->phase == QUIZ_ASKING && Quiz()->seconds_left == 12.0f);
            Wait(9.5); Capture("edu-04-ultimos-segundos");
            Wait(2.6); MORPH_CHECK(Quiz()->result == QUIZ_TIMEOUT); Capture("edu-05-tempo-esgotado");
            Continue(); MORPH_CHECK(Quiz()->phase == QUIZ_ASKING);
            Command(COMMAND_TOGGLE_PAUSE); Capture("edu-06-pausa-pergunta"); Command(COMMAND_RESUME);
        }
        Answer(true);
        if (world == 0) Capture("edu-07-acerto");
        Continue();
        MORPH_CHECK(game.state.mode == GAME_PORTAL);
        for (int i = 0; i < 400 && game.state.mode != GAME_PLAY && game.state.mode != GAME_BOSS; i++) Tick();
        if (world < 5) MORPH_CHECK(game.state.world == world + 1 && game.state.mode == GAME_PLAY);
        if (world == 1) {
            /* Segunda chance na campanha: derrota real pelo chão. */
            uint64_t score = game.state.score;
            game.state.lives = 1;
            for (int i = 0; i < 400 && game.state.mode == GAME_PLAY; i++) Tick();
            MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->kind == QUIZ_REVIVE && game.state.lives == 0);
            Wait(1.0); Capture("edu-08-ultima-chance");
            Answer(true); Capture("edu-09-revive-acerto"); Continue();
            MORPH_CHECK(game.state.mode == GAME_PLAY && game.state.lives == 1 && game.state.world == 2);
            MORPH_CHECK(game.state.score == score && game.state.edu.revive_used);
            PlayerView p = game.modules.player.view(game.modules.player.life.context);
            MORPH_CHECK(p.invulnerable && p.position.x == 180);
            for (int i = 0; i < 18; i++) Fly();
            Capture("edu-10-protegido");
            game.state.lives = 3; /* restaura o roteiro da campanha */
        }
    }
    MORPH_CHECK(game.state.mode == GAME_BOSS && game.state.boss.segments == 3);
}

static void Boss(void)
{
    for (int i = 0; i < 180; i++) Tick();
    MORPH_CHECK(game.state.boss.stage == BOSS_FIGHT);
    Capture("edu-11-boss-energia");
    /* Nova aventura para usar a segunda chance dentro do combate. */
    game.state.lives = 1;
    game.state.edu.revive_used = false;
    unsigned segments = game.state.boss.segments;
    for (int i = 0; i < 600 && game.state.mode == GAME_BOSS; i++) Tick();
    MORPH_CHECK(game.state.mode == GAME_QUIZ && Quiz()->return_mode == GAME_BOSS);
    Capture("edu-12-boss-ultima-chance");
    Answer(true); Continue();
    MORPH_CHECK(game.state.mode == GAME_BOSS && game.state.lives == 1 && game.state.boss.segments == segments);
    for (unsigned shot = 0; shot < 3; shot++) {
        game.state.boss.energy.collected = game.state.boss.energy.required;
        GameInput fire = {.focused = true, .attack_pressed = true}; Advance(MORPH_STEP_SECONDS, fire);
        MORPH_CHECK(game.state.boss.beam_active);
        for (int i = 0; i < 85; i++) Tick();
    }
    for (int i = 0; i < 240 && game.state.mode == GAME_BOSS; i++) Tick();
    MORPH_CHECK(game.state.mode == GAME_WON);
    Capture("edu-13-vitoria");
}

int main(int argc, char **argv)
{
    MORPH_CHECK(argc == 3 && strcmp(argv[1], "--capture") == 0);
    directory = argv[2];
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(960, 540, "Morph Adventure - laboratorio educativo"); MORPH_CHECK(IsWindowReady());
    canvas = LoadRenderTexture(960, 540); MORPH_CHECK(IsRenderTextureValid(canvas));
    ItemsSetSeed(77); GameSetQuizSeed(2026);
    GameModules modules = MorphCreateModules();
    MORPH_CHECK(GameInitMode(&game, &modules, true) && game.ready);
    Campaign();
    Boss();
    GameShutdown(&game);
    UnloadRenderTexture(canvas); CloseWindow();
    puts("PASS edu-flow-gui: seis mundos reais com desafio antes de cada portal, erro/tempo esgotado/acerto, "
         "segunda chance na campanha e no boss, vitória só após Grootspectro");
    return 0;
}
