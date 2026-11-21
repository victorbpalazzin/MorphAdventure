/* Testes das regras puras do núcleo (pessoa 1), sem janela e sem módulos
 * simulados: chama as funções reais de src/core/. Complementa core_check.c. */
#include "core/game.h"
#include "core/viewport.h"
#include "morph_check.h"
#include <math.h>
#include <string.h>

static void CheckContractConstants(void)
{
    MORPH_CHECK(MORPH_CONTRACT_VERSION == 5u);
    MORPH_CHECK(MORPH_WIDTH == 960 && MORPH_HEIGHT == 540);
    MORPH_CHECK(MORPH_WORLD_COUNT == 6);
    MORPH_CHECK(MORPH_MAX_BOOSTS == 2);
    MORPH_CHECK_NEAR(MORPH_STEP_SECONDS * 60.0, 1.0, 1e-12);
    /* A maior meta (Fogo) cabe no limite de fragmentos do contrato. */
    MORPH_CHECK(GameEnergyRequired(MORPH_WORLD_COUNT - 1) <= MORPH_MAX_ENERGY);
}

/* Metas 4, 6, 8, 10, 12, 16: nunca seis fragmentos fixos. */
static void CheckEnergyTargets(void)
{
    static const unsigned expected[MORPH_WORLD_COUNT] = {4, 6, 8, 10, 12, 16};
    for (int w = 0; w < MORPH_WORLD_COUNT; w++)
        MORPH_CHECK(GameEnergyRequired(w) == expected[w]);
    MORPH_CHECK(GameEnergyRequired(-1) == 0);
    MORPH_CHECK(GameEnergyRequired(MORPH_WORLD_COUNT) == 0);
    MORPH_CHECK(GameEnergyRequired(1000) == 0);
}

/* Bateria proporcional após cada coleta; 100% só ao completar a meta. */
static void CheckBatteryAfterEachCollection(void)
{
    for (int w = 0; w < MORPH_WORLD_COUNT; w++) {
        unsigned target = GameEnergyRequired(w);
        float previous = -1;
        for (unsigned k = 0; k <= target; k++) {
            float fraction = GameEnergyFraction(k, w);
            MORPH_CHECK(fraction > previous);
            MORPH_CHECK_NEAR(fraction, (double)k / target, 1e-6);
            if (k < target) MORPH_CHECK(fraction < 1.0f);
            else MORPH_CHECK(fraction == 1.0f);
            previous = fraction;
        }
        MORPH_CHECK(GameEnergyFraction(target + 5, w) == 1.0f); /* nunca acima de 100% */
    }
    MORPH_CHECK(GameEnergyFraction(3, -1) == 0.0f);
    MORPH_CHECK(GameEnergyFraction(3, MORPH_WORLD_COUNT) == 0.0f);
}

static void CheckEventQueue(void)
{
    GameEvent e = {0};
    MORPH_CHECK(!GameEnqueueEvent(NULL, e));
    GameEventBuffer buffer = {0};
    for (unsigned i = 0; i < MORPH_MAX_EVENTS; i++) {
        e.entity_id = i;
        MORPH_CHECK(GameEnqueueEvent(&buffer, e));
    }
    MORPH_CHECK(buffer.count == MORPH_MAX_EVENTS && !buffer.overflow);
    for (unsigned i = 0; i < MORPH_MAX_EVENTS; i++) MORPH_CHECK(buffer.values[i].entity_id == i); /* ordem preservada */
    e.entity_id = 999;
    MORPH_CHECK(!GameEnqueueEvent(&buffer, e));
    MORPH_CHECK(buffer.overflow && buffer.count == MORPH_MAX_EVENTS); /* falha explícita, sem sobrescrever */
}

static void CheckModeNames(void)
{
    static const char *names[] = {"INTRO", "PLAY", "PAUSED", "PORTAL", "TRANSITION", "LOST", "WON", "ERROR"};
    for (int m = GAME_INTRO; m <= GAME_ERROR; m++) MORPH_CHECK(strcmp(GameModeName((GameMode)m), names[m]) == 0);
    MORPH_CHECK(strcmp(GameModeName(GAME_BOSS), "BOSS") == 0);
    MORPH_CHECK(strcmp(GameModeName((GameMode)99), "INVALID") == 0);
}

/* Redimensionamento: escala proporcional, barras e conversão inversa de clique. */
static void CheckViewportSizes(void)
{
    static const int sizes[][2] = {
        {960, 540}, {1280, 720}, {1920, 1080}, {1100, 700}, {800, 600},
        {480, 270}, {2560, 1080}, {1080, 1920}, {1366, 768}, {3840, 2160}};
    for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; i++) {
        int w = sizes[i][0], h = sizes[i][1];
        GameViewport v = ViewportFit(w, h);
        MORPH_CHECK(v.scale > 0);
        MORPH_CHECK_NEAR(v.width / v.height, 960.0 / 540.0, 1e-4);         /* proporção mantida */
        MORPH_CHECK(v.width <= w + 0.01f && v.height <= h + 0.01f);         /* cabe na janela */
        MORPH_CHECK(fabsf(v.width - w) < 0.01f || fabsf(v.height - h) < 0.01f); /* encosta em um eixo */
        MORPH_CHECK_NEAR(v.x * 2 + v.width, w, 0.01);                        /* centralizado */
        MORPH_CHECK_NEAR(v.y * 2 + v.height, h, 0.01);

        GameVec2 p;
        /* Ida e volta lógica -> tela -> lógica numa grade. */
        for (float lx = 0; lx < MORPH_WIDTH; lx += 95.5f) {
            for (float ly = 0; ly < MORPH_HEIGHT; ly += 53.5f) {
                GameVec2 screen = {v.x + lx * v.scale, v.y + ly * v.scale};
                MORPH_CHECK(ViewportToLogical(v, screen, &p));
                MORPH_CHECK_NEAR(p.x, lx, 1e-2);
                MORPH_CHECK_NEAR(p.y, ly, 1e-2);
            }
        }
        /* Fora da área (barras ou bordas externas) nunca é clique de jogo.
         * Usa 1 px de folga: exatamente em x = v.x + v.width há arredondamento
         * de float (ex.: janela 1100x700 dá 959.99997 e conta como dentro).
         * Registrado para a pessoa 1 em docs/handoffs/pessoa-7.md. */
        MORPH_CHECK(!ViewportToLogical(v, (GameVec2){v.x - 1, v.y + v.height / 2}, &p));
        MORPH_CHECK(!ViewportToLogical(v, (GameVec2){v.x + v.width + 1, v.y + v.height / 2}, &p));
        MORPH_CHECK(!ViewportToLogical(v, (GameVec2){v.x + v.width / 2, v.y - 1}, &p));
        MORPH_CHECK(!ViewportToLogical(v, (GameVec2){v.x + v.width / 2, v.y + v.height + 1}, &p));
    }
    GameVec2 p = {5, 5};
    MORPH_CHECK(!ViewportToLogical(ViewportFit(-1, 100), (GameVec2){1, 1}, &p));
    MORPH_CHECK(p.x == 0 && p.y == 0);
    MORPH_CHECK(!ViewportToLogical(ViewportFit(960, 540), (GameVec2){INFINITY, 1}, &p));
    MORPH_CHECK(!ViewportToLogical(ViewportFit(960, 540), (GameVec2){1, 1}, NULL));
}

/* Sem módulos, o executável é só diagnóstico: não inicia partida nem simula. */
static void CheckDiagnosticShell(void)
{
    MORPH_CHECK(!GameInit(NULL, NULL));
    Game g;
    GameModules none = {0};
    none.contract_version = MORPH_CONTRACT_VERSION;
    MORPH_CHECK(GameInit(&g, &none));
    MORPH_CHECK(!g.ready && g.missing_modules == MODULE_ALL && g.state.mode == GAME_INTRO);
    MORPH_CHECK(g.state.lives == 3 && g.state.score == 0);
    GameInput input = {0};
    input.focused = true;
    input.impulse_pressed = true;
    input.command = COMMAND_START;
    for (int i = 0; i < 120; i++) GameAdvance(&g, MORPH_STEP_SECONDS, input);
    MORPH_CHECK(g.state.mode == GAME_INTRO && g.state.simulation_steps == 0);
    GameDraw(&g); /* não chama módulos ausentes */
    GameShutdown(&g);
    GameShutdown(&g);
    MORPH_CHECK(!g.ready);
}

int main(void)
{
    CheckContractConstants();
    CheckEnergyTargets();
    CheckBatteryAfterEachCollection();
    CheckEventQueue();
    CheckModeNames();
    CheckViewportSizes();
    CheckDiagnosticShell();
    puts("PASS core-rules: metas 4..19, bateria por coleta, fila de eventos, nomes de modo, "
         "viewport em 10 tamanhos e janela de diagnóstico sem módulos.");
    return 0;
}
