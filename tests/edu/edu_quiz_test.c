/* Modo educativo: banco de perguntas e regras puras do cronômetro (src/edu/).
 * Sem janela e sem módulos simulados. */
#include "edu/quiz.h"
#include "morph_check.h"
#include <string.h>

static void CheckBank(void)
{
    size_t count = LogicQuestionCount();
    MORPH_CHECK(count >= 36);
    MORPH_CHECK(count <= MORPH_QUIZ_DECK);
    MORPH_CHECK(LogicQuestionAt(count) == NULL);
    unsigned per_topic[TOPIC_COUNT] = {0}, per_answer[LOGIC_OPTION_COUNT] = {0};
    for (size_t i = 0; i < count; i++) {
        const LogicQuestion *q = LogicQuestionAt(i);
        MORPH_CHECK(q && q->prompt && q->explanation);
        MORPH_CHECK(strlen(q->prompt) > 10 && strlen(q->prompt) < 130);    /* rápido de ler */
        MORPH_CHECK(strlen(q->explanation) > 10 && strlen(q->explanation) < 150);
        MORPH_CHECK((unsigned)q->topic < TOPIC_COUNT && LogicTopicName(q->topic)[0]);
        MORPH_CHECK(q->correct < LOGIC_OPTION_COUNT);
        for (int a = 0; a < LOGIC_OPTION_COUNT; a++) {
            MORPH_CHECK(q->options[a] && q->options[a][0]);
            MORPH_CHECK(strlen(q->options[a]) < 48);                       /* cabe no botão */
            for (int b = a + 1; b < LOGIC_OPTION_COUNT; b++) MORPH_CHECK(strcmp(q->options[a], q->options[b]) != 0);
        }
        for (size_t j = i + 1; j < count; j++) MORPH_CHECK(strcmp(q->prompt, LogicQuestionAt(j)->prompt) != 0);
        per_topic[q->topic]++;
        per_answer[q->correct]++;
    }
    for (int t = 0; t < TOPIC_COUNT; t++) MORPH_CHECK(per_topic[t] >= 4);   /* todos os assuntos */
    for (int a = 0; a < LOGIC_OPTION_COUNT; a++) MORPH_CHECK(per_answer[a] >= 5); /* sem padrão de letra */
}

/* Verificação semântica: as perguntas de valor lógico concordam com as regras. */
static int Truth(const char *option)
{
    if (strcmp(option, "Verdadeiro.") == 0 || strcmp(option, "Verdadeira.") == 0) return 1;
    if (strcmp(option, "Falso.") == 0 || strcmp(option, "Falsa.") == 0) return 0;
    return -1;
}
static void CheckConnectiveAnswers(void)
{
    struct { const char *prompt; int value; } expected[] = {
        {"Se P é verdadeiro e Q é falso, qual é o resultado de \"P E Q\"?", 0},
        {"Se P é verdadeiro e Q é verdadeiro, \"P E Q\" é:", 1},
        {"Se P é falso e Q é falso, \"P E Q\" é:", 0},
        {"Se P é verdadeiro e Q é falso, \"P OU Q\" é:", 1},
        {"Se P é falso e Q é falso, \"P OU Q\" é:", 0},
        {"Se P e Q são verdadeiras, \"P OU Q\" é:", 1},
        {"Se P é verdadeiro e Q é verdadeiro, \"SE P, ENTÃO Q\" é:", 1},
        {"Se P é verdadeiro e Q é falso, \"SE P, ENTÃO Q\" é:", 0},
        {"Se P é verdadeiro, então NÃO P é:", 0},
        {"Se P é falso, então NÃO P é:", 1},
        {"Se P é verdadeiro, qual é o valor de NÃO (NÃO P)?", 1},
        {"A proposição \"2 + 2 = 5\" é:", 0},
        {"A frase \"2 é par E 3 é par\" é:", 0},
        {"A frase \"O Sol é uma estrela OU 2 + 2 = 5\" é:", 1},
    };
    for (size_t e = 0; e < sizeof(expected) / sizeof(*expected); e++) {
        bool found = false;
        for (size_t i = 0; i < LogicQuestionCount(); i++) {
            const LogicQuestion *q = LogicQuestionAt(i);
            if (strcmp(q->prompt, expected[e].prompt) != 0) continue;
            found = true;
            MORPH_CHECK(Truth(q->options[q->correct]) == expected[e].value);
            int candidates = 0; /* exatamente uma alternativa com o valor certo */
            for (int a = 0; a < LOGIC_OPTION_COUNT; a++) if (Truth(q->options[a]) == expected[e].value) candidates++;
            MORPH_CHECK(candidates == 1);
        }
        MORPH_CHECK(found);
    }
}

static void Wait(QuizState *q, double seconds)
{
    for (double t = 0; t < seconds - 1e-9; t += 0.05) MORPH_CHECK(!QuizAdvanceClock(q, 0.05));
}

static void CheckTimer(void)
{
    QuizState q;
    QuizSeed(&q, 77);
    QuizBegin(&q, QUIZ_PASSAGE, GAME_PLAY);
    MORPH_CHECK(q.phase == QUIZ_ASKING && q.result == QUIZ_PENDING && QuizCurrent(&q));
    MORPH_CHECK(q.seconds_left == MORPH_QUIZ_SECONDS && MORPH_QUIZ_SECONDS == 12.0f);   /* 7 */
    MORPH_CHECK(!QuizAnswer(&q, 0));                       /* proteção: impulso já em curso */
    MORPH_CHECK(!QuizAdvanceClock(&q, 1.0 / 60));
    MORPH_CHECK_NEAR(q.seconds_left, 12 - 1.0 / 60, 1e-5);   /* 8 */
    MORPH_CHECK(!QuizAdvanceClock(&q, 5.0));                /* salto limitado a 0,1 s */
    MORPH_CHECK_NEAR(q.seconds_left, 12 - 1.0 / 60 - 0.1, 1e-5);
    MORPH_CHECK(!QuizAdvanceClock(&q, NAN) && !QuizAdvanceClock(&q, -3));
    MORPH_CHECK_NEAR(q.seconds_left, 12 - 1.0 / 60 - 0.1, 1e-5);
    Wait(&q, 0.4);
    const LogicQuestion *current = QuizCurrent(&q);
    MORPH_CHECK(!QuizAnswer(&q, -1) && !QuizAnswer(&q, 4));
    MORPH_CHECK(QuizAnswer(&q, current->correct));          /* 9 */
    MORPH_CHECK(q.result == QUIZ_CORRECT && q.phase == QUIZ_FEEDBACK && q.selected == current->correct);
    float frozen = q.seconds_left;
    MORPH_CHECK(!QuizCanContinue(&q));
    for (int i = 0; i < 100; i++) MORPH_CHECK(!QuizAdvanceClock(&q, 0.1));
    MORPH_CHECK(q.seconds_left == frozen && QuizCanContinue(&q)); /* feedback congela o relógio */
    MORPH_CHECK(!QuizAnswer(&q, current->correct));         /* uma resposta por pergunta */

    /* Exatamente 12 s a 60 Hz: 719 quadros ainda permitem resposta, o 720º encerra. */
    int previous = q.question;
    QuizNextQuestion(&q);
    MORPH_CHECK(q.question != previous && q.seconds_left == 12.0f && q.phase == QUIZ_ASKING); /* 13 */
    for (int i = 0; i < 719; i++) MORPH_CHECK(!QuizAdvanceClock(&q, 1.0 / 60));
    MORPH_CHECK(q.phase == QUIZ_ASKING && q.seconds_left > 0);
    MORPH_CHECK(QuizAdvanceClock(&q, 1.0 / 60));
    MORPH_CHECK(q.result == QUIZ_TIMEOUT && q.seconds_left == 0 && q.phase == QUIZ_FEEDBACK);
    MORPH_CHECK(!QuizAnswer(&q, QuizCurrent(&q)->correct)); /* 10: depois do prazo */
    MORPH_CHECK(q.result == QUIZ_TIMEOUT);

    /* Errar mostra a correta e mantém a escolha registrada. */
    QuizNextQuestion(&q);
    Wait(&q, 0.4);
    int wrong = (QuizCurrent(&q)->correct + 1) % LOGIC_OPTION_COUNT;
    MORPH_CHECK(QuizAnswer(&q, wrong) && q.result == QUIZ_WRONG && q.selected == wrong);
    MORPH_CHECK(QuizOptionFromCommand(COMMAND_ANSWER_A) == 0 && QuizOptionFromCommand(COMMAND_ANSWER_D) == 3);
    MORPH_CHECK(QuizOptionFromCommand(COMMAND_CONTINUE) == -1 && QuizOptionFromCommand(COMMAND_START) == -1);
}

static void CheckNoImmediateRepetition(void)
{
    for (uint32_t seed = 1; seed < 40; seed++) {
        QuizState q;
        QuizSeed(&q, seed * 7919u);
        QuizBegin(&q, QUIZ_PASSAGE, GAME_PLAY);
        unsigned seen[MORPH_QUIZ_DECK] = {0};
        size_t total = LogicQuestionCount();
        int previous = q.question;
        seen[previous]++;
        for (size_t i = 1; i < total * 5; i++) {
            QuizNextQuestion(&q);
            MORPH_CHECK(q.question >= 0 && (size_t)q.question < total);
            MORPH_CHECK(q.question != previous);           /* nunca a mesma em seguida */
            previous = q.question;
            seen[q.question]++;
        }
        for (size_t i = 0; i < total; i++) MORPH_CHECK(seen[i] == 5); /* baralho: todas aparecem */
    }
    QuizState a, b;
    QuizSeed(&a, 1234); QuizSeed(&b, 1234);
    MORPH_CHECK(memcmp(a.deck, b.deck, sizeof(a.deck)) == 0);  /* reprodutível em testes */
    QuizSeed(&b, 4321);
    MORPH_CHECK(memcmp(a.deck, b.deck, sizeof(a.deck)) != 0);
}

int main(void)
{
    CheckBank();
    CheckConnectiveAnswers();
    CheckTimer();
    CheckNoImmediateRepetition();
    printf("PASS edu-quiz: %zu perguntas, cronômetro de 12 s, prazo determinístico, sem repetição imediata\n",
           LogicQuestionCount());
    return 0;
}
