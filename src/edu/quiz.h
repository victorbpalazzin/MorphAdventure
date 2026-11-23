#ifndef MORPH_EDU_QUIZ_H
#define MORPH_EDU_QUIZ_H
/* Regras puras do desafio de lógica: sorteio, cronômetro de 12 s e respostas.
 * Sem raylib e sem Game: o coordenador decide o que cada resultado provoca. */
#include "core/game_types.h"
#include "edu/questions.h"

/* Embaralha o baralho de perguntas para uma nova aventura. */
void QuizSeed(QuizState *quiz, uint32_t seed);
/* Abre um desafio: próxima pergunta do baralho, exatamente 12 s. */
void QuizBegin(QuizState *quiz, QuizKind kind, GameMode return_mode);
/* Nova pergunta do mesmo tipo (nunca a anterior), reiniciando os 12 s. */
void QuizNextQuestion(QuizState *quiz);
/* Tempo de apresentação limitado a 0,1 s por quadro. Só corre em QUIZ_ASKING.
 * Retorna true quando o prazo termina nesta chamada (vira TEMPO ESGOTADO). */
bool QuizAdvanceClock(QuizState *quiz, double seconds);
/* Registra a alternativa 0..3. Rejeitada fora do prazo, durante o feedback ou
 * antes da proteção inicial contra cliques/impulsos que já estavam em curso. */
bool QuizAnswer(QuizState *quiz, int option);
bool QuizCanContinue(const QuizState *quiz);
const LogicQuestion *QuizCurrent(const QuizState *quiz);
/* Converte COMMAND_ANSWER_A..D em 0..3; -1 para outros comandos. */
int QuizOptionFromCommand(GameCommand command);

#endif
