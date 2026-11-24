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

#ifdef MORPH_MODULES_LINKED
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

#endif
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
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--smoke-frames") == 0 && i + 1 < argc) smoke_frames = atoi(argv[++i]);
        else if (strcmp(argv[i], "--capture") == 0 && i + 1 < argc) capture = argv[++i];
    }
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1100, 700, "Morph Adventure - base C/raylib (pessoa 1)");
#ifdef MORPH_MODULES_LINKED
    SetWindowTitle("Morph Adventure");
#endif
    if (!IsWindowReady()) return 1;
    SetWindowMinSize(480, 270);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    RenderTexture2D target = LoadRenderTexture(MORPH_WIDTH, MORPH_HEIGHT);
    if (!IsRenderTextureValid(target)) { CloseWindow(); return 1; }
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    GameModules modules = {0};
    modules.contract_version = MORPH_CONTRACT_VERSION;
#ifdef MORPH_MODULES_LINKED
    ItemsSetSeed((uint32_t)time(NULL) ^ (uint32_t)clock() ^
        (uint32_t)(GetTime()*1000000.0) ^ ((uint32_t)GetRandomValue(0,INT_MAX)<<1));
    modules = MorphCreateModules();
    AudioInit(NULL); /* sem dispositivo ou arquivos: segue em silêncio, com aviso no log */
#endif
    Game game;
    bool initialized = GameInit(&game, &modules);
    bool previous_touch = false;
#ifdef MORPH_MODULES_LINKED
    PadState pad_state; PadReset(&pad_state);
#endif
    while (!WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
        GameViewport viewport = ViewportFit(GetScreenWidth(), GetScreenHeight());
        GameInput input = {0};
        input.focused = IsWindowFocused() && !IsWindowMinimized();
        input.impulse_pressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
        input.attack_pressed = IsKeyPressed(KEY_J);
        if (IsKeyPressed(KEY_P)) input.command = COMMAND_TOGGLE_PAUSE;
#ifdef MORPH_MODULES_LINKED
        if (IsKeyPressed(KEY_TAB)) UiQueueNavigation(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? -1 : 1, false);
        if (IsKeyPressed(KEY_ENTER)) UiQueueNavigation(0, true);
#endif
        if (game.state.mode == GAME_INTRO && input.impulse_pressed) input.command = COMMAND_START;
#ifdef MORPH_MODULES_LINKED
        if (PadSupported()) {
            int pad_navigation = 0;
            bool pad_activate = false;
            PadActions pad = PadUpdate(&pad_state, ReadPad(), (double)GetFrameTime());
            PadApply(pad, game.state.mode, &input, &pad_navigation, &pad_activate);
            if (pad_navigation || pad_activate) UiQueueNavigation(pad_navigation, pad_activate);
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
