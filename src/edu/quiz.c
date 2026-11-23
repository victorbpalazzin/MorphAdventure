#include "quiz.h"
#include <math.h>
#include <string.h>

/* Abaixo disto o prazo é considerado esgotado: evita que o arredondamento de
 * 720 quadros de 1/60 s deixe um resto microscópico e conceda um quadro extra. */
#define QUIZ_EPSILON 1e-4f

static uint32_t Next(QuizState *q)
{
    q->seed = q->seed * 1664525u + 1013904223u;
    return q->seed >> 8;
}

static void Shuffle(QuizState *q)
{
    size_t total = LogicQuestionCount();
    unsigned count = (unsigned)(total < MORPH_QUIZ_DECK ? total : MORPH_QUIZ_DECK);
    for (unsigned i = 0; i < count; i++) q->deck[i] = (unsigned char)i;
    for (unsigned i = count; i > 1; i--) {
        unsigned j = Next(q) % i;
        unsigned char t = q->deck[i - 1]; q->deck[i - 1] = q->deck[j]; q->deck[j] = t;
    }
    /* A virada do baralho nunca repete imediatamente a última pergunta. */
    if (count > 1 && (int)q->deck[0] == q->question) {
        unsigned char t = q->deck[0]; q->deck[0] = q->deck[1]; q->deck[1] = t;
    }
    q->deck_count = count;
    q->deck_position = 0;
}

void QuizSeed(QuizState *q, uint32_t seed)
{
    memset(q, 0, sizeof(*q));
    q->seed = seed ? seed : 0x6c6f6769u;
    q->question = q->last_question = -1;
    q->selected = -1;
    Shuffle(q);
}

static void Ask(QuizState *q)
{
    if (!q->deck_count || q->deck_position >= q->deck_count) Shuffle(q);
    q->last_question = q->question;
    q->question = q->deck_count ? q->deck[q->deck_position++] : -1;
    q->phase = QUIZ_ASKING;
    q->result = QUIZ_PENDING;
    q->selected = -1;
    q->seconds_left = MORPH_QUIZ_SECONDS;
    q->age = q->feedback_age = 0;
    q->serial++;
}

void QuizBegin(QuizState *q, QuizKind kind, GameMode return_mode)
{
    q->kind = kind;
    q->return_mode = return_mode;
    Ask(q);
}

void QuizNextQuestion(QuizState *q) { Ask(q); }

static void Feedback(QuizState *q, QuizResult result)
{
    q->result = result;
    q->phase = QUIZ_FEEDBACK;
    q->feedback_age = 0;
    q->serial++;
}

bool QuizAdvanceClock(QuizState *q, double seconds)
{
    if (!isfinite(seconds) || seconds <= 0) return false;
    float dt = (float)fmin(seconds, 0.1);
    if (q->phase == QUIZ_FEEDBACK) { q->feedback_age += dt; return false; }
    q->age += dt;
    q->seconds_left -= dt;
    if (q->seconds_left > QUIZ_EPSILON) return false;
    q->seconds_left = 0;
    Feedback(q, QUIZ_TIMEOUT);
    return true;
}

bool QuizAnswer(QuizState *q, int option)
{
    const LogicQuestion *question = QuizCurrent(q);
    if (!question || q->phase != QUIZ_ASKING || option < 0 || option >= LOGIC_OPTION_COUNT) return false;
    if (q->seconds_left <= QUIZ_EPSILON || q->age < MORPH_QUIZ_INPUT_GUARD) return false;
    q->selected = option;
    Feedback(q, option == question->correct ? QUIZ_CORRECT : QUIZ_WRONG);
    return true;
}

bool QuizCanContinue(const QuizState *q)
{
    return q->phase == QUIZ_FEEDBACK && q->feedback_age >= MORPH_QUIZ_FEEDBACK_GUARD;
}

const LogicQuestion *QuizCurrent(const QuizState *q)
{
    return q->question >= 0 ? LogicQuestionAt((size_t)q->question) : NULL;
}

int QuizOptionFromCommand(GameCommand command)
{
    switch (command) {
    case COMMAND_ANSWER_A: return 0;
    case COMMAND_ANSWER_B: return 1;
    case COMMAND_ANSWER_C: return 2;
    case COMMAND_ANSWER_D: return 3;
    default: return -1;
    }
}
