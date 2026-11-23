#include "gamepad.h"
#include "core/game.h"
#include <math.h>
#include <string.h>

bool PadSupported(void) { return GameEducationalBuild(); }

void PadReset(PadState *s)
{
    memset(s, 0, sizeof(*s));
    s->id = -1;
}

static int Direction(PadState *s, const PadSnapshot *p)
{
    /* Direcional digital tem prioridade sobre o analógico. */
    int dpad = (p->down[PAD_DOWN] || p->down[PAD_RIGHT] ? 1 : 0) - (p->down[PAD_UP] || p->down[PAD_LEFT] ? 1 : 0);
    float x = isfinite(p->stick_x) ? p->stick_x : 0, y = isfinite(p->stick_y) ? p->stick_y : 0;
    float magnitude = fmaxf(fabsf(x), fabsf(y));
    s->stick_engaged = s->stick_engaged ? magnitude > PAD_STICK_RELEASE : magnitude > PAD_STICK_ENGAGE;
    if (dpad) return dpad;
    if (!s->stick_engaged) return 0;
    float axis = fabsf(y) >= fabsf(x) ? y : x;
    return axis > 0 ? 1 : -1;
}

PadActions PadUpdate(PadState *s, PadSnapshot p, double seconds)
{
    PadActions a = {0};
    bool same = p.connected && s->connected && p.id == s->id;
    if (!same) {
        /* Conectou, desconectou ou trocou: nenhum disparo fantasma. Botões que
         * já estavam apertados viram "anteriores" e precisam ser soltos. */
        PadReset(s);
        if (!p.connected) return a;
        s->connected = true;
        s->id = p.id;
        memcpy(s->previous, p.down, sizeof(s->previous));
        s->held_direction = Direction(s, &p);
        s->repeat_timer = PAD_REPEAT_DELAY;
        return a;
    }
    a.cross = p.down[PAD_CROSS] && !s->previous[PAD_CROSS];
    a.circle = p.down[PAD_CIRCLE] && !s->previous[PAD_CIRCLE];
    a.square = p.down[PAD_SQUARE] && !s->previous[PAD_SQUARE];
    a.options = p.down[PAD_OPTIONS] && !s->previous[PAD_OPTIONS];
    memcpy(s->previous, p.down, sizeof(s->previous));

    float dt = isfinite(seconds) && seconds > 0 ? (float)fmin(seconds, 0.1) : 0;
    int direction = Direction(s, &p);
    if (direction != s->held_direction) {
        s->held_direction = direction;
        s->repeat_timer = PAD_REPEAT_DELAY;
        a.navigate = direction;            /* um passo imediato por inclinação */
    } else if (direction) {
        s->repeat_timer -= dt;
        if (s->repeat_timer <= 1e-4f) {    /* repetição controlada, no máximo 1 por quadro */
            a.navigate = direction;
            s->repeat_timer = PAD_REPEAT_INTERVAL;
        }
    }
    return a;
}

static bool Menu(GameMode mode)
{
    return mode == GAME_INTRO || mode == GAME_PAUSED || mode == GAME_LOST || mode == GAME_WON || mode == GAME_QUIZ;
}

void PadApply(PadActions a, GameMode mode, GameInput *input, int *navigation, bool *activate)
{
    if (a.options && input->command == COMMAND_NONE) input->command = COMMAND_TOGGLE_PAUSE;
    if (mode == GAME_PLAY || mode == GAME_BOSS) {
        /* X só voa no gameplay; em PORTAL/TRANSITION o core ignora impulso. */
        if (a.cross) input->impulse_pressed = true;
        if (a.square && mode == GAME_BOSS) input->attack_pressed = true;
        return;
    }
    if (!Menu(mode)) return;
    /* Em menus e perguntas o X confirma pelo foco da UI e nunca vira impulso:
     * a borda é consumida neste quadro e o gameplay retomado exige novo toque. */
    if (a.navigate && navigation) *navigation = a.navigate;
    if (a.cross && activate) *activate = true;
    if (a.circle && input->command == COMMAND_NONE) {
        if (mode == GAME_PAUSED) input->command = COMMAND_RESUME;                       /* fecha a pausa */
        else if (mode == GAME_LOST || mode == GAME_WON) input->command = COMMAND_RESTART; /* "Voltar ao início"/"Menu Principal" */
    }
}

const char *PadButtonName(PadButton b)
{
    static const char *names[PAD_BUTTON_COUNT] = {"X (cruz)", "Circulo", "Quadrado", "OPTIONS",
                                                  "Cima", "Baixo", "Esquerda", "Direita"};
    return (unsigned)b < PAD_BUTTON_COUNT ? names[b] : "?";
}
