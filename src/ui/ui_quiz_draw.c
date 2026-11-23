/* Painel "DESAFIO DE LÓGICA" / "ÚLTIMA CHANCE!" do modo educativo.
 * Somente leitura: não altera GameState nem avança relógios. 960x540 lógicos. */
#include "ui_internal.h"
#include "ui/ui.h"
#include "edu/quiz.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const Color ink = {237, 246, 236, 255}, gold = {244, 218, 158, 255}, mint = {156, 239, 213, 255};
static const Color amber = {255, 206, 92, 255}, coral = {255, 124, 118, 255}, green = {116, 222, 150, 255};

static void Panel(Rectangle r, Color color, float roundness)
{
    DrawRectangleRounded(r, roundness, 12, color);
    DrawRectangleRoundedLinesEx(r, roundness, 12, 1, GetColor(0xb5d9ca55));
}

/* Quebra por palavras sem alocação; devolve a altura usada. draw=false só mede. */
static int Layout(const char *text, int x, int y, int width, int size, int line_height, Color color, bool center, bool draw)
{
    char line[256] = {0}, candidate[256];
    int lines = 0;
    const char *p = text;
    while (*p) {
        while (*p == ' ') p++;
        const char *end = p;
        while (*end && *end != ' ') end++;
        if (end == p) break;
        int word = (int)(end - p);
        snprintf(candidate, sizeof(candidate), "%s%s%.*s", line, line[0] ? " " : "", word, p);
        if (line[0] && MeasureText(candidate, size) > width) {
            int lx = center ? x + (width - MeasureText(line, size)) / 2 : x;
            if (draw) DrawText(line, lx, y + lines * line_height, size, color);
            lines++;
            snprintf(line, sizeof(line), "%.*s", word, p);
        } else {
            memcpy(line, candidate, sizeof(line));
        }
        p = end;
    }
    if (line[0]) {
        int lx = center ? x + (width - MeasureText(line, size)) / 2 : x;
        if (draw) DrawText(line, lx, y + lines * line_height, size, color);
        lines++;
    }
    return lines * line_height;
}

static int Wrap(const char *text, int x, int y, int width, int size, int line_height, Color color, bool center)
{
    return Layout(text, x, y, width, size, line_height, color, center, true);
}

/* Foco só vale depois que a entrada da UI viu esta pergunta/fase. */
static int Focus(const UiState *s, const QuizState *q)
{
    if (s->quiz_serial == q->serial) return s->quiz_focus;
    return q->phase == QUIZ_FEEDBACK ? 0 : -1;
}

static void Centered(const char *text, int y, int size, Color color)
{
    DrawText(text, 480 - MeasureText(text, size) / 2, y, size, color);
}

static Color TimerColor(float seconds)
{
    if (seconds <= 3) return coral;
    if (seconds <= 5) return amber;
    return mint;
}

static void Timer(const QuizState *q)
{
    float seconds = fmaxf(0, q->seconds_left);
    Color color = TimerColor(seconds);
    bool asking = q->phase == QUIZ_ASKING;
    /* Pulsação discreta nos últimos 3 s, apenas enquanto o relógio corre. */
    float pulse = asking && seconds <= 3 && seconds > 0 ? .5f + .5f * sinf(q->age * 12) : 0;
    char text[16];
    snprintf(text, sizeof(text), "%d", (int)ceilf(seconds));
    int size = 44 + (int)(pulse * 6);
    Rectangle badge = {776, 30, 88, 48};
    Panel(badge, Fade(GetColor(0x0b1d2dff), .95f), .3f);
    if (pulse > 0) DrawRectangleRoundedLinesEx((Rectangle){772, 26, 96, 56}, .3f, 12, 2, Fade(coral, .35f + .5f * pulse));
    DrawText(text, (int)(badge.x + (badge.width - MeasureText(text, size)) / 2), (int)(badge.y + (badge.height - size) / 2 + 2), size, color);
    DrawText("s", (int)(badge.x + badge.width - 14), 60, 14, Fade(ink, .7f));
    Rectangle bar = {96, 86, 664, 12};
    DrawRectangleRounded(bar, .5f, 8, GetColor(0x0b1d2dff));
    float fraction = fminf(1, seconds / MORPH_QUIZ_SECONDS);
    if (fraction > 0) DrawRectangleRounded((Rectangle){bar.x, bar.y, bar.width * fraction, bar.height}, .5f, 8, color);
}

static void Option(const LogicQuestion *question, const QuizState *q, UiButton b, int index, bool focus)
{
    Rectangle r = {b.rect.x, b.rect.y, b.rect.width, b.rect.height};
    Color fill = GetColor(0x1c3b52ff), letter = gold, text = ink;
    if (q->phase == QUIZ_FEEDBACK) {
        if (index == question->correct) { fill = GetColor(0x1f6b4cff); letter = green; }
        else if (index == q->selected) { fill = GetColor(0x6b2a33ff); letter = coral; }
        else { text = Fade(ink, .55f); letter = Fade(gold, .5f); }
    }
    Panel(r, fill, .3f);
    if (focus) DrawRectangleRoundedLinesEx((Rectangle){r.x - 3, r.y - 3, r.width + 6, r.height + 6}, .3f, 12, 3, gold);
    Vector2 center = {r.x + 26, r.y + r.height / 2};
    DrawCircleV(center, 16, GetColor(0x0d2233ff));
    DrawCircleLinesV(center, 16, letter);
    char label[2] = {(char)('A' + index), '\0'};
    DrawText(label, (int)(center.x - MeasureText(label, 20) / 2.0f), (int)(center.y - 10), 20, letter);
    DrawText(question->options[index], (int)r.x + 56, (int)(r.y + (r.height - 21) / 2), 21, text);
}

static void Asking(const UiState *s, const GameState *g, const LogicQuestion *question)
{
    const QuizState *q = &g->edu.quiz;
    int y = 108;
    if (q->kind == QUIZ_REVIVE) {
        y += Wrap("Você perdeu todas as suas vidas, mas ainda pode continuar! Responda corretamente "
                  "a uma pergunta de lógica em até 12 segundos para ganhar mais uma chance.",
                  96, y, 768, 18, 23, gold, true);
        y += 8;
    }
    char topic[96];
    snprintf(topic, sizeof(topic), "Tema: %s", LogicTopicName(question->topic));
    DrawText(topic, 96, y, 16, Fade(mint, .85f));
    UiButton b[4];
    unsigned n = UiQuizButtons(q, b);
    /* Enunciado centralizado verticalmente entre o tema e as alternativas. */
    int area_top = y + 24, area = (int)b[0].rect.y - 12 - area_top;
    int height = Layout(question->prompt, 96, 0, 768, 25, 31, ink, true, false);
    Wrap(question->prompt, 96, area_top + (area > height ? (area - height) / 2 : 0), 768, 25, 31, ink, true);
    int focus = Focus(s, q);
    for (unsigned i = 0; i < n; i++) Option(question, q, b[i], (int)i, focus == (int)i);
    Centered(UiGamepadHints() ? "Direcional escolhe a alternativa  |  X confirma  |  OPTIONS pausa"
                              : "Clique, use 1-4 ou A-D  |  Setas e Enter  |  P pausa", 498, 15, Fade(ink, .75f));
}

static void Feedback(const UiState *s, const GameState *g, const LogicQuestion *question)
{
    const QuizState *q = &g->edu.quiz;
    const char *heading = q->result == QUIZ_CORRECT ? "ACERTOU!" : q->result == QUIZ_TIMEOUT ? "TEMPO ESGOTADO!" : "RESPOSTA INCORRETA";
    Color color = q->result == QUIZ_CORRECT ? green : q->result == QUIZ_TIMEOUT ? amber : coral;
    Centered(heading, 112, 36, color);
    char line[256];
    int y = 160;
    snprintf(line, sizeof(line), "Resposta correta: %c) %s", 'A' + question->correct, question->options[question->correct]);
    y += Wrap(line, 96, y, 768, 22, 28, mint, true);
    if (q->result == QUIZ_WRONG && q->selected >= 0 && q->selected < LOGIC_OPTION_COUNT) {
        snprintf(line, sizeof(line), "Sua resposta: %c) %s", 'A' + q->selected, question->options[q->selected]);
        y += Wrap(line, 96, y + 4, 768, 18, 24, Fade(coral, .9f), true) + 4;
    }
    Rectangle box = {110, (float)y + 14, 740, 108};
    Panel(box, GetColor(0x0d2536ee), .15f);
    DrawText("Explicação", (int)box.x + 16, (int)box.y + 10, 16, gold);
    Wrap(question->explanation, (int)box.x + 16, (int)box.y + 34, (int)box.width - 32, 20, 25, ink, false);
    const char *next;
    if (q->kind == QUIZ_PASSAGE)
        next = q->result == QUIZ_CORRECT ? (g->world == MORPH_WORLD_COUNT - 1 ? "Portal liberado! Grootspectro aguarda do outro lado."
                                                                              : "Portal liberado! Siga para o próximo mundo.")
                                         : "O portal continua fechado. Outra pergunta chega com 12 segundos.";
    else
        next = q->result == QUIZ_CORRECT ? "Você ganhou mais uma vida! Proteção temporária ativada."
                                         : "A segunda chance terminou.";
    Centered(next, 386, 18, q->result == QUIZ_CORRECT ? green : ink);
    UiButton b[4];
    unsigned n = UiQuizButtons(q, b);
    bool ready = QuizCanContinue(q);
    for (unsigned i = 0; i < n; i++) {
        Rectangle r = {b[i].rect.x, b[i].rect.y, b[i].rect.width, b[i].rect.height};
        Panel(r, ready ? GetColor(0xa9e5ceff) : GetColor(0x5d7f78ff), .3f);
        if (Focus(s, q) == (int)i) DrawRectangleRoundedLinesEx((Rectangle){r.x - 4, r.y - 4, r.width + 8, r.height + 8}, .3f, 12, 2, gold);
        DrawText(b[i].label, (int)(r.x + (r.width - MeasureText(b[i].label, 22)) / 2), (int)(r.y + 14), 22, GetColor(0x173443ff));
    }
    Centered(UiGamepadHints() ? "X para continuar" : "Enter, Espaço ou clique para continuar", 486, 15, Fade(ink, .75f));
}

void UiQuizDraw(const UiState *s, const GameState *g)
{
    const QuizState *q = &g->edu.quiz;
    const LogicQuestion *question = QuizCurrent(q);
    if (q->kind == QUIZ_NONE || !question) return;
    DrawRectangle(0, 0, MORPH_WIDTH, MORPH_HEIGHT, GetColor(0x071322bb));
    Panel((Rectangle){70, 20, 820, 500}, GetColor(0x132c3ff8), .06f);
    bool revive = q->kind == QUIZ_REVIVE;
    DrawText(revive ? "ÚLTIMA CHANCE!" : "DESAFIO DE LÓGICA", 96, 36, 34, revive ? coral : gold);
    DrawText(revive ? "Uma única pergunta para voltar à aventura" : "Acerte para liberar o portal",
             98, 68, 15, Fade(ink, .8f));
    Timer(q);
    if (q->phase == QUIZ_ASKING) Asking(s, g, question);
    else Feedback(s, g, question);
}
