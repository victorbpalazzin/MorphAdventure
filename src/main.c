#include "raylib.h"
#include "core/game.h"
#include "core/viewport.h"
#include "input/gamepad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#if RAYLIB_VERSION_MAJOR != 5 || RAYLIB_VERSION_MINOR != 5
#error This project currently pins raylib 5.5. Coordinate version changes with person 7.
#endif

/* Person 1 adds this composition function after the real modules are delivered.
 * Without it, this executable explicitly shows the integration status only. */
#ifdef MORPH_MODULES_LINKED
#include "ui/ui.h"
#include "items/items.h"
#include "audio/audio.h"
GameModules MorphCreateModules(void);

/* Som: M silencia a qualquer momento; na pausa, 1-3 escolhem o canal e as setas
 * ajustam. Cliques no painel chegam pela UI (UiTakeAudioRequest). */
static int audio_selected = 0;
static void AudioInput(const Game *g)
{
    if (IsKeyPressed(KEY_M)) AudioToggleMute();
    if (g->state.mode == GAME_PAUSED) {
        if (IsKeyPressed(KEY_ONE)) audio_selected = 0;
        if (IsKeyPressed(KEY_TWO)) audio_selected = 1;
        if (IsKeyPressed(KEY_THREE)) audio_selected = 2;
        if (IsKeyPressed(KEY_LEFT)) AudioAdjust((AudioChannel)audio_selected, -1);
        if (IsKeyPressed(KEY_RIGHT)) AudioAdjust((AudioChannel)audio_selected, 1);
    }
    UiAudioRequest r = UiTakeAudioRequest();
    if (r.toggle_mute) AudioToggleMute();
    if (r.channel >= 0 && r.channel < AUDIO_CHANNELS) { audio_selected = r.channel; AudioAdjust((AudioChannel)r.channel, r.delta); }
    AudioSettings s = AudioGetSettings();
    UiAudioView view = {AudioGetStats().enabled, {s.volume[0], s.volume[1], s.volume[2]}, s.muted, audio_selected};
    UiSetAudioView(view);
}
#endif

/* Controle: usa os botões SEMÂNTICOS da raylib 5.5 (mapeados pela base
 * SDL_GameControllerDB embutida no GLFW). No DualShock 4: RIGHT_FACE_DOWN = X,
 * RIGHT_FACE_RIGHT = Círculo, RIGHT_FACE_LEFT = Quadrado, MIDDLE_RIGHT = OPTIONS.
 * O primeiro controle disponível é usado; conectar/desconectar é verificado a
 * cada quadro. Confira com: morph-adventure --gamepad-test */
static const int pad_buttons[PAD_BUTTON_COUNT] = {
    GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    GAMEPAD_BUTTON_RIGHT_FACE_LEFT, GAMEPAD_BUTTON_MIDDLE_RIGHT,
    GAMEPAD_BUTTON_LEFT_FACE_UP, GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    GAMEPAD_BUTTON_LEFT_FACE_LEFT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT};

static PadSnapshot ReadPad(void)
{
    PadSnapshot pad = {0};
    pad.id = -1;
    for (int id = 0; id < 4; id++) {
        if (!IsGamepadAvailable(id)) continue;
        pad.connected = true;
        pad.id = id;
        for (int b = 0; b < PAD_BUTTON_COUNT; b++) pad.down[b] = IsGamepadButtonDown(id, pad_buttons[b]);
        pad.stick_x = GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_X);
        pad.stick_y = GetGamepadAxisMovement(id, GAMEPAD_AXIS_LEFT_Y);
        break;
    }
    return pad;
}

/* Tela de verificação do mapeamento real, sem iniciar o jogo. */
static void GamepadTest(void)
{
    PadState state; PadReset(&state);
    int last = 0, navigated = 0;
    while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
        PadSnapshot pad = ReadPad();
        PadActions a = PadUpdate(&state, pad, GetFrameTime());
        int pressed = GetGamepadButtonPressed();
        if (pressed) last = pressed;
        navigated += a.navigate;
        BeginDrawing();
        ClearBackground((Color){12, 23, 37, 255});
        Color ink = {232, 237, 224, 255}, gold = {238, 207, 143, 255}, on = {116, 222, 150, 255};
        DrawText("MORPH ADVENTURE - teste do controle (ESC sai)", 30, 24, 24, gold);
        if (!pad.connected) DrawText("Nenhum controle detectado. Conecte o cabo USB.", 30, 70, 20, ink);
        else {
            DrawText(TextFormat("Controle %d: %s", pad.id, GetGamepadName(pad.id)), 30, 70, 20, ink);
            for (int b = 0; b < PAD_BUTTON_COUNT; b++)
                DrawText(TextFormat("%-10s %s", PadButtonName((PadButton)b), pad.down[b] ? "PRESSIONADO" : "-"),
                         30, 110 + b * 28, 20, pad.down[b] ? on : ink);
            DrawText(TextFormat("Analogico esquerdo: x=%.2f  y=%.2f  | navegacao acumulada: %d",
                                pad.stick_x, pad.stick_y, navigated), 30, 346, 20, ink);
            DrawText(TextFormat("Ultimo botao raylib (GetGamepadButtonPressed): %d", last), 30, 380, 20, ink);
            DrawText("Esperado no DualShock 4: X=7 Circulo=6 Quadrado=8 OPTIONS=15", 30, 414, 18, gold);
        }
        EndDrawing();
    }
}

static void DrawDiagnostic(const Game *g, GameInput input)
{
    Color ink = {232, 237, 224, 255}, gold = {238, 207, 143, 255};
    ClearBackground((Color){12, 23, 37, 255});
    DrawRectangleGradientV(0, 0, MORPH_WIDTH, MORPH_HEIGHT, (Color){26, 54, 70, 255}, (Color){12, 23, 37, 255});
    DrawText("MORPH ADVENTURE", 48, 40, 34, gold);
    DrawText("C99 + raylib 5.5 | Base de integracao - Pessoa 1", 50, 86, 19, ink);
    DrawText("Esta janela ainda nao e o jogo migrado.", 50, 126, 23, gold);
    DrawText("O jogo sera habilitado quando os modulos reais estiverem conectados.", 50, 160, 18, ink);
    static const char *names[] = {"Pessoa 2: personagem e fisica", "Pessoa 3: cenarios e parallax",
        "Pessoa 4: obstaculos e ambiente", "Pessoa 5: energia e boosts",
        "Pessoa 6: interface", "Pessoa 6: efeitos e portais"};
    for (unsigned i = 0; i < 6; i++) {
        int y = 216 + (int)i * 32;
        bool missing = (g->missing_modules & (1u << i)) != 0;
        DrawCircle(58, y + 9, 5, missing ? gold : GREEN);
        DrawText(names[i], 77, y, 18, ink);
        DrawText(missing ? "PENDENTE" : "CONTRATO PRESENTE", 605, y, 17, missing ? gold : GREEN);
    }
    DrawText("Proximo passo: pessoa 7 configura o build; equipe implementa os modulos.", 50, 440, 17, ink);
    DrawText("Redimensione a janela. As barras externas nao recebem cliques do jogo.", 50, 468, 16, ink);
    if (g->error[0]) DrawText(g->error, 50, 496, 16, RED);
    else DrawText("ESC fecha | Contrato: docs/CONTRACTS.md", 50, 496, 16, gold);
    if (input.pointer_inside) DrawCircleLines((int)input.pointer.x, (int)input.pointer.y, 8, gold);
}

int main(int argc, char **argv)
{
    int smoke_frames = 0, frame = 0;
    const char *capture = NULL;
    bool gamepad_test = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--smoke-frames") == 0 && i + 1 < argc) smoke_frames = atoi(argv[++i]);
        else if (strcmp(argv[i], "--capture") == 0 && i + 1 < argc) capture = argv[++i];
        else if (strcmp(argv[i], "--gamepad-test") == 0) gamepad_test = true;
    }
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1100, 700, "Morph Adventure - base C/raylib (pessoa 1)");
#ifdef MORPH_MODULES_LINKED
    SetWindowTitle(GameEducationalBuild() ? "Morph Adventure - Modo Educativo" : "Morph Adventure");
#endif
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(480, 270);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    if (gamepad_test) {
        if (PadSupported()) GamepadTest();
        else printf("--gamepad-test: o controle existe somente na versao educativa (MORPH_EDUCATIONAL=ON).\n");
        CloseWindow();
        return 0;
    }
    RenderTexture2D target = LoadRenderTexture(MORPH_WIDTH, MORPH_HEIGHT);
    if (!IsRenderTextureValid(target)) { CloseWindow(); return 1; }
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    GameModules modules = {0};
    modules.contract_version = MORPH_CONTRACT_VERSION;
#ifdef MORPH_MODULES_LINKED
    uint32_t session_seed = (uint32_t)time(NULL) ^ (uint32_t)clock() ^
        (uint32_t)(GetTime()*1000000.0) ^ ((uint32_t)GetRandomValue(0,INT_MAX)<<1);
    ItemsSetSeed(session_seed);
    GameSetQuizSeed(session_seed ^ 0x9e3779b9u);
    modules = MorphCreateModules();
    AudioInit(NULL); /* sem dispositivo ou arquivos: segue em silêncio, com aviso no log */
#endif
    Game game;
    bool initialized = GameInit(&game, &modules);
    bool previous_touch = false;
    PadState pad_state; PadReset(&pad_state);
    while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
        GameViewport viewport = ViewportFit(GetScreenWidth(), GetScreenHeight());
        GameInput input = {0};
        input.focused = IsWindowFocused() && !IsWindowMinimized();
        input.impulse_pressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
        input.attack_pressed = IsKeyPressed(KEY_J);
        if (IsKeyPressed(KEY_P)) input.command = COMMAND_TOGGLE_PAUSE;
        if (game.state.mode == GAME_QUIZ) {
            /* Challenge keys: 1-4 or A-D answer directly; arrows move the focus;
             * Space/Enter activate it. Flight keys (Up/W) never answer. */
            static const int keys[4][3] = {{KEY_ONE, KEY_KP_1, KEY_A}, {KEY_TWO, KEY_KP_2, KEY_B},
                                           {KEY_THREE, KEY_KP_3, KEY_C}, {KEY_FOUR, KEY_KP_4, KEY_D}};
            static const GameCommand answers[4] = {COMMAND_ANSWER_A, COMMAND_ANSWER_B, COMMAND_ANSWER_C, COMMAND_ANSWER_D};
            input.impulse_pressed = IsKeyPressed(KEY_SPACE);
            for (int i = 0; i < 4; i++)
                for (int k = 0; k < 3; k++) if (IsKeyPressed(keys[i][k])) input.command = answers[i];
#ifdef MORPH_MODULES_LINKED
            if (IsKeyPressed(KEY_UP)) UiQueueNavigation(-1, false);
            if (IsKeyPressed(KEY_DOWN)) UiQueueNavigation(1, false);
#endif
        }
#ifdef MORPH_MODULES_LINKED
        if (IsKeyPressed(KEY_TAB)) UiQueueNavigation(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? -1 : 1, false);
        if (IsKeyPressed(KEY_ENTER)) UiQueueNavigation(0, true);
#endif
        if (game.state.mode == GAME_INTRO && input.impulse_pressed) input.command = COMMAND_START;
#ifdef MORPH_MODULES_LINKED
        /* Versão educativa: o controle entra no mesmo fluxo do teclado/mouse.
         * No arcade nada aqui é executado (nem leitura, nem textos de ajuda). */
        if (PadSupported()) {
            int pad_navigation = 0; bool pad_activate = false;
            PadActions pad = PadUpdate(&pad_state, ReadPad(), (double)GetFrameTime());
            PadApply(pad, game.state.mode, &input, &pad_navigation, &pad_activate);
            if (pad_navigation || pad_activate) UiQueueNavigation(pad_navigation, pad_activate);
            UiSetGamepadHints(pad_state.connected);
        }
#endif
        bool touch = GetTouchPointCount() > 0;
        Vector2 point = touch ? GetTouchPosition(0) : GetMousePosition();
        input.pointer_inside = ViewportToLogical(viewport, (GameVec2){point.x, point.y}, &input.pointer);
        input.pointer_pressed = touch ? !previous_touch : IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        previous_touch = touch;
        GameAdvance(&game, (double)GetFrameTime(), input);
#ifdef MORPH_MODULES_LINKED
        AudioInput(&game);
        AudioFrame(&game.state, UiInspect().focus, (double)GetFrameTime());
#endif
        BeginTextureMode(target);
        ClearBackground((Color){12, 23, 37, 255});
        if (!game.ready || game.state.mode == GAME_ERROR) DrawDiagnostic(&game, input);
        else GameDraw(&game);
        EndTextureMode();
        BeginDrawing();
        ClearBackground(BLACK);
        if (viewport.scale > 0) DrawTexturePro(target.texture,
            (Rectangle){0, 0, MORPH_WIDTH, -MORPH_HEIGHT},
            (Rectangle){viewport.x, viewport.y, viewport.width, viewport.height},
            (Vector2){0}, 0, WHITE);
        EndDrawing();
        frame++;
        if (smoke_frames > 0 && frame >= smoke_frames) {
            if (capture) TakeScreenshot(capture);
            break;
        }
    }
    printf("core-shell: initialized=%d ready=%d missing=0x%02x frames=%d\n",
           initialized, game.ready, game.missing_modules, frame);
    GameShutdown(&game);
#ifdef MORPH_MODULES_LINKED
    AudioShutdown();
#endif
    UnloadRenderTexture(target);
    CloseWindow();
    return initialized ? 0 : 1;
}
