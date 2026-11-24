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

static int Direction(PadState *s, const PadSnapshot *p, bool *horizontal)
{
    /* Direcional digital tem prioridade sobre o analógico. */
    int vertical = (p->down[PAD_DOWN] ? 1 : 0) - (p->down[PAD_UP] ? 1 : 0);
    int side = (p->down[PAD_RIGHT] ? 1 : 0) - (p->down[PAD_LEFT] ? 1 : 0);
    float x = isfinite(p->stick_x) ? p->stick_x : 0, y = isfinite(p->stick_y) ? p->stick_y : 0;
    float magnitude = fmaxf(fabsf(x), fabsf(y));
    s->stick_engaged = s->stick_engaged ? magnitude > PAD_STICK_RELEASE : magnitude > PAD_STICK_ENGAGE;
    *horizontal = false;
    if (vertical) return vertical;
    if (side) { *horizontal = true; return side; }
    if (!s->stick_engaged) return 0;
    *horizontal = fabsf(x) > fabsf(y);
    float axis = *horizontal ? x : y;
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
        s->held_direction = Direction(s, &p, &s->held_horizontal);
        s->repeat_timer = PAD_REPEAT_DELAY;
        return a;
    }
    a.cross = p.down[PAD_CROSS] && !s->previous[PAD_CROSS];
    a.circle = p.down[PAD_CIRCLE] && !s->previous[PAD_CIRCLE];
    a.square = p.down[PAD_SQUARE] && !s->previous[PAD_SQUARE];
    a.options = p.down[PAD_OPTIONS] && !s->previous[PAD_OPTIONS];
    a.triangle = p.down[PAD_TRIANGLE] && !s->previous[PAD_TRIANGLE];
    a.l1 = p.down[PAD_L1] && !s->previous[PAD_L1];
    a.r1 = p.down[PAD_R1] && !s->previous[PAD_R1];
    memcpy(s->previous, p.down, sizeof(s->previous));

    float dt = isfinite(seconds) && seconds > 0 ? (float)fmin(seconds, 0.1) : 0;
    bool horizontal = false;
    int direction = Direction(s, &p, &horizontal);
    a.horizontal = horizontal;
    if (direction != s->held_direction || horizontal != s->held_horizontal) {
        s->held_direction = direction;
        s->held_horizontal = horizontal;
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
    /* Na pausa, ←/→ ajustam o volume (PadAudioActions); só ↑/↓ trocam de botão. */
    if (a.navigate && navigation && !(mode == GAME_PAUSED && a.horizontal)) *navigation = a.navigate;
    if (a.cross && activate) *activate = true;
    if (a.circle && input->command == COMMAND_NONE) {
        if (mode == GAME_PAUSED) input->command = COMMAND_RESUME;                       /* fecha a pausa */
        else if (mode == GAME_LOST || mode == GAME_WON) input->command = COMMAND_RESTART; /* "Voltar ao início"/"Menu Principal" */
    }
}

PadAudio PadAudioActions(PadActions a, GameMode mode)
{
    PadAudio audio = {0, 0, a.triangle};
    if (mode == GAME_PAUSED) {
        audio.select = (a.r1 ? 1 : 0) - (a.l1 ? 1 : 0);
        if (a.navigate && a.horizontal) audio.adjust = a.navigate;
    }
    return audio;
}

const char *PadButtonName(PadButton b)
{
    static const char *names[PAD_BUTTON_COUNT] = {"X (cruz)", "Circulo", "Quadrado", "OPTIONS",
                                                  "Cima", "Baixo", "Esquerda", "Direita",
                                                  "Triangulo", "L1", "R1"};
    return (unsigned)b < PAD_BUTTON_COUNT ? names[b] : "?";
}
