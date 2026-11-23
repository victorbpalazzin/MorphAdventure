#ifndef MORPH_EDU_QUESTIONS_H
#define MORPH_EDU_QUESTIONS_H
/* Banco de perguntas do modo educativo (Lógica Proposicional, nível feira).
 * Dados puros, sem raylib. Ver docs/handoffs/modo-educativo.md. */
#include <stddef.h>

#define LOGIC_OPTION_COUNT 4

typedef enum LogicTopic {
    TOPIC_PROPOSITION, TOPIC_TRUTH_VALUE, TOPIC_NEGATION, TOPIC_CONJUNCTION,
    TOPIC_DISJUNCTION, TOPIC_CONDITIONAL, TOPIC_CLASSIFICATION, TOPIC_COUNT
} LogicTopic;

typedef struct LogicQuestion {
    LogicTopic topic;
    const char *prompt;
    const char *options[LOGIC_OPTION_COUNT];
    unsigned char correct;     /* 0..3 = A..D, exactly one right alternative */
    const char *explanation;
} LogicQuestion;

size_t LogicQuestionCount(void);
const LogicQuestion *LogicQuestionAt(size_t index); /* NULL outside the bank */
const char *LogicTopicName(LogicTopic topic);

#endif
