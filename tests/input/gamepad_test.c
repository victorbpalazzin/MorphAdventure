/* Mapeamento de ações do controle (src/input/gamepad.c), sem raylib e sem janela. */
#include "input/gamepad.h"
#include "core/game.h"
#include "morph_check.h"
#include <string.h>

#define DT (1.0 / 60)

static PadSnapshot Pad(void) { PadSnapshot p = {0}; p.connected = true; p.id = 0; return p; }
static PadActions Press(PadState *s, PadButton b)
{
    PadSnapshot p = Pad(); p.down[b] = true;
    PadActions a = PadUpdate(s, p, DT);
    PadUpdate(s, Pad(), DT); /* solta */
    return a;
}
static PadState Connected(void) { PadState s; PadReset(&s); PadUpdate(&s, Pad(), DT); return s; }

static void CheckEdges(void)
{
    PadState s = Connected();
    PadSnapshot held = Pad(); held.down[PAD_CROSS] = true;
    MORPH_CHECK(PadUpdate(&s, held, DT).cross);
    for (int i = 0; i < 120; i++) MORPH_CHECK(!PadUpdate(&s, held, DT).cross); /* segurar não repete */
    PadUpdate(&s, Pad(), DT);
    MORPH_CHECK(PadUpdate(&s, held, DT).cross);
    PadUpdate(&s, Pad(), DT);
    MORPH_CHECK(Press(&s, PAD_CIRCLE).circle && Press(&s, PAD_SQUARE).square && Press(&s, PAD_OPTIONS).options);
    PadActions a = Press(&s, PAD_SQUARE);
    MORPH_CHECK(!a.cross && !a.circle && !a.options && a.navigate == 0);
}

/* Conectar com botão apertado, desconectar e trocar de controle: sem disparos fantasmas. */
static void CheckHotplug(void)
{
    PadState s; PadReset(&s);
    PadSnapshot none = {0}; none.id = -1;
    MORPH_CHECK(!PadUpdate(&s, none, DT).cross && !s.connected);
    PadSnapshot held = Pad(); held.down[PAD_CROSS] = held.down[PAD_OPTIONS] = true; held.stick_y = 1;
    PadActions a = PadUpdate(&s, held, DT);
    MORPH_CHECK(s.connected && !a.cross && !a.options && a.navigate == 0);
    for (int i = 0; i < 10; i++) { a = PadUpdate(&s, held, DT); MORPH_CHECK(!a.cross && !a.options); }
    PadUpdate(&s, none, DT);                          /* desconectou */
    MORPH_CHECK(!s.connected);
    MORPH_CHECK(PadUpdate(&s, none, DT).navigate == 0);
    a = PadUpdate(&s, held, DT);                      /* reconectou já apertado */
    MORPH_CHECK(!a.cross && !a.options);
    PadSnapshot other = held; other.id = 1;           /* outro controle assumiu */
    a = PadUpdate(&s, other, DT);
    MORPH_CHECK(s.id == 1 && !a.cross);
    PadSnapshot released = Pad(); released.id = 1;
    PadUpdate(&s, released, DT);
    other.stick_y = 0; other.down[PAD_OPTIONS] = false;
    MORPH_CHECK(PadUpdate(&s, other, DT).cross);      /* agora vale */
}

static void CheckRepeatTimeline(void)
{
    PadState s = Connected();
    PadSnapshot down = Pad(); down.down[PAD_DOWN] = true;
    int frames[16], n = 0;
    for (int f = 0; f < 60 && n < 16; f++) if (PadUpdate(&s, down, DT).navigate == 1) frames[n++] = f;
    MORPH_CHECK(n >= 3 && frames[0] == 0);
    MORPH_CHECK(frames[1] >= 23 && frames[1] <= 25);          /* ~0,40 s */
    MORPH_CHECK(frames[2] - frames[1] >= 10 && frames[2] - frames[1] <= 12); /* ~0,18 s */
    PadUpdate(&s, Pad(), DT);
    PadSnapshot up = Pad(); up.down[PAD_UP] = true;
    MORPH_CHECK(PadUpdate(&s, up, DT).navigate == -1);
    PadSnapshot left = Pad(); left.down[PAD_LEFT] = true;
    PadUpdate(&s, Pad(), DT);
    MORPH_CHECK(PadUpdate(&s, left, DT).navigate == -1);
    PadSnapshot right = Pad(); right.down[PAD_RIGHT] = true;
    MORPH_CHECK(PadUpdate(&s, right, DT).navigate == 1);   /* mudar de direção = passo imediato */
    /* Quadro longo não gera rajada: no máximo um passo por quadro. */
    PadUpdate(&s, Pad(), DT);
    MORPH_CHECK(PadUpdate(&s, down, DT).navigate == 1);
    int total = 0;
    for (int i = 0; i < 10; i++) {                    /* quadros travados de 5 s contam 0,1 s */
        int n = PadUpdate(&s, down, 5.0).navigate;
        MORPH_CHECK(n == 0 || n == 1);
        total += n;
    }
    MORPH_CHECK(total == 4);                          /* 0,40 s e depois a cada 0,18 s em 1,0 s */
}

/* Analógico: zona morta com histerese evita seleções involuntárias. */
static void CheckStick(void)
{
    PadState s = Connected();
    PadSnapshot p = Pad();
    p.stick_y = 0.5f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 0);    /* abaixo de 0,55 */
    p.stick_y = 0.6f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 1);
    p.stick_y = 0.4f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 0);    /* ainda engatado */
    p.stick_y = 0.7f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 0);    /* oscilar não re-dispara */
    p.stick_y = 0.3f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 0);    /* soltou */
    p.stick_y = 0.6f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 1);
    p.stick_y = 0;    PadUpdate(&s, p, DT);
    p.stick_y = -0.9f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == -1);
    p.stick_y = 0; PadUpdate(&s, p, DT);
    p.stick_x = 0.9f; p.stick_y = 0.2f; MORPH_CHECK(PadUpdate(&s, p, DT).navigate == 1);
    p.stick_x = 0; p.stick_y = 0; PadUpdate(&s, p, DT);
    p.stick_y = 0.9f; p.down[PAD_UP] = true;                              /* direcional prevalece */
    MORPH_CHECK(PadUpdate(&s, p, DT).navigate == -1);
    PadSnapshot nan = Pad(); nan.stick_x = NAN; nan.stick_y = NAN;
    PadUpdate(&s, Pad(), DT);
    MORPH_CHECK(PadUpdate(&s, nan, DT).navigate == 0);
}

static GameInput Apply(PadActions a, GameMode mode, int *nav, bool *act)
{
    GameInput in = {0}; in.focused = true; *nav = 0; *act = false;
    PadApply(a, mode, &in, nav, act);
    return in;
}

/* Mapeamento por tela, igual no arcade e no educativo. */
static void CheckRouting(void)
{
    int nav; bool act;
    PadActions cross = {.cross = true}, square = {.square = true}, options = {.options = true};
    PadActions circle = {.circle = true}, down = {.navigate = 1}, up = {.navigate = -1};
    GameInput in = Apply(cross, GAME_PLAY, &nav, &act);
    MORPH_CHECK(in.impulse_pressed && !act && in.command == COMMAND_NONE);          /* X voa */
    in = Apply(cross, GAME_BOSS, &nav, &act); MORPH_CHECK(in.impulse_pressed && !in.attack_pressed);
    in = Apply(square, GAME_BOSS, &nav, &act); MORPH_CHECK(in.attack_pressed && !in.impulse_pressed); /* megarraio */
    in = Apply(square, GAME_PLAY, &nav, &act); MORPH_CHECK(!in.attack_pressed);
    in = Apply(down, GAME_PLAY, &nav, &act); MORPH_CHECK(nav == 0);                 /* analógico não pilota */
    in = Apply(cross, GAME_PORTAL, &nav, &act); MORPH_CHECK(!in.impulse_pressed && !act);
    in = Apply(cross, GAME_TRANSITION, &nav, &act); MORPH_CHECK(!in.impulse_pressed && !act);
    static const GameMode pausable[] = {GAME_PLAY, GAME_BOSS, GAME_PORTAL, GAME_TRANSITION, GAME_QUIZ, GAME_PAUSED};
    for (size_t i = 0; i < sizeof(pausable) / sizeof(*pausable); i++) {
        in = Apply(options, pausable[i], &nav, &act);
        MORPH_CHECK(in.command == COMMAND_TOGGLE_PAUSE);                           /* OPTIONS pausa/retoma */
    }
    static const GameMode menus[] = {GAME_INTRO, GAME_PAUSED, GAME_LOST, GAME_WON, GAME_QUIZ};
    for (size_t i = 0; i < sizeof(menus) / sizeof(*menus); i++) {
        in = Apply(cross, menus[i], &nav, &act);
        MORPH_CHECK(act && !in.impulse_pressed && !in.attack_pressed);             /* X confirma, nunca voa */
        in = Apply(down, menus[i], &nav, &act); MORPH_CHECK(nav == 1);
        in = Apply(up, menus[i], &nav, &act); MORPH_CHECK(nav == -1);
        in = Apply(square, menus[i], &nav, &act); MORPH_CHECK(!in.attack_pressed && !act);
    }
    /* Círculo só onde voltar/cancelar já existe. */
    MORPH_CHECK(Apply(circle, GAME_PAUSED, &nav, &act).command == COMMAND_RESUME);
    MORPH_CHECK(Apply(circle, GAME_LOST, &nav, &act).command == COMMAND_RESTART);
    MORPH_CHECK(Apply(circle, GAME_WON, &nav, &act).command == COMMAND_RESTART);
    static const GameMode no_back[] = {GAME_INTRO, GAME_PLAY, GAME_BOSS, GAME_QUIZ, GAME_PORTAL, GAME_TRANSITION};
    for (size_t i = 0; i < sizeof(no_back) / sizeof(*no_back); i++) {
        in = Apply(circle, no_back[i], &nav, &act);
        MORPH_CHECK(in.command == COMMAND_NONE && !act && !in.impulse_pressed);
    }
    /* Um comando do teclado no mesmo quadro tem prioridade. */
    GameInput keyboard = {0}; keyboard.command = COMMAND_ANSWER_B; nav = 0; act = false;
    PadApply(options, GAME_QUIZ, &keyboard, &nav, &act);
    MORPH_CHECK(keyboard.command == COMMAND_ANSWER_B);
    MORPH_CHECK(PadButtonName(PAD_CROSS)[0] && PadButtonName((PadButton)99)[0] == '?');
    MORPH_CHECK(PadSupported() == (MORPH_EDUCATIONAL != 0)); /* só a versão educativa lê o controle */
}

int main(void)
{
    CheckEdges();
    CheckHotplug();
    CheckRepeatTimeline();
    CheckStick();
    CheckRouting();
    puts("PASS input-gamepad: bordas, conexão/desconexão, repetição controlada, histerese do analógico e mapeamento por tela");
    return 0;
}
