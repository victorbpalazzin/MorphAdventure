/* Prova que MORPH_CHECK continua ativo com NDEBUG. Este alvo é compilado
 * sempre com NDEBUG e PRECISA terminar com falha (CTest: WILL_FAIL).
 * Se um dia MORPH_CHECK passar a usar a macro assert, este teste passa a "passar" e o
 * CTest acusa o erro. */
#ifndef NDEBUG
#error "check_macro_selftest deve ser compilado com NDEBUG"
#endif
#include "morph_check.h"

int main(void)
{
    MORPH_CHECK(1 + 1 == 3); /* falso: deve encerrar aqui com código de falha */
    puts("ERRO: MORPH_CHECK foi ignorado com NDEBUG.");
    return 0;
}
