#ifndef MORPH_TEST_CHECK_H
#define MORPH_TEST_CHECK_H
/* Verificação de testes que continua ativa com NDEBUG (Release).
 * Não use a macro assert da <assert.h> nos testes: em Release ela some, e a chamada dentro
 * dele também. MORPH_CHECK sempre avalia a expressão e encerra com falha. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MORPH_CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "CHECK falhou: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

#define MORPH_CHECK_NEAR(a, b, eps) MORPH_CHECK(fabs((double)(a) - (double)(b)) <= (double)(eps))

#endif
