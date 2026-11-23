#include "questions.h"

/* Perguntas curtas e diretas para visitantes sem estudo prévio de Lógica.
 * Conectivos escritos por extenso (E, OU, NÃO, SE... ENTÃO). A disjunção é
 * sempre a inclusiva. Cada item tem uma única alternativa correta. */
static const LogicQuestion bank[] = {
    /* ---------- Proposições ---------- */
    {TOPIC_PROPOSITION, "Qual frase pode ser considerada uma proposição lógica?",
     {"Feche a porta!", "Que horas são?", "O número 2 é par.", "Bom dia!"}, 2,
     "Uma proposição é uma afirmação que pode ser verdadeira ou falsa."},
    {TOPIC_PROPOSITION, "Qual destas frases NÃO é uma proposição?",
     {"O Sol é uma estrela.", "3 + 3 = 7.", "Recife fica no Brasil.", "Abra a janela!"}, 3,
     "Uma ordem não é verdadeira nem falsa. Já \"3 + 3 = 7\" é uma proposição (falsa)."},
    {TOPIC_PROPOSITION, "Uma proposição é uma frase que:",
     {"Faz uma pergunta.", "Dá uma ordem.", "Pode ser verdadeira ou falsa.", "Expressa um desejo."}, 2,
     "Perguntas, ordens e desejos não podem ser julgados como verdadeiros ou falsos."},
    {TOPIC_PROPOSITION, "A frase \"5 é maior que 2.\" é:",
     {"Uma proposição.", "Uma pergunta.", "Uma ordem.", "Uma exclamação."}, 0,
     "É uma afirmação que podemos julgar: neste caso, ela é verdadeira."},
    {TOPIC_PROPOSITION, "Quantos valores lógicos uma proposição tem ao mesmo tempo?",
     {"Exatamente um.", "Nenhum.", "Dois.", "Três."}, 0,
     "Uma proposição é verdadeira ou falsa, nunca as duas coisas ao mesmo tempo."},
    {TOPIC_PROPOSITION, "Qual frase é uma proposição?",
     {"Que dia lindo!", "Você gosta de pizza?", "Estude para a prova.", "A Lua gira em torno da Terra."}, 3,
     "\"A Lua gira em torno da Terra\" é uma afirmação; as outras são exclamação, pergunta e ordem."},

    /* ---------- Verdadeiro e falso ---------- */
    {TOPIC_TRUTH_VALUE, "A proposição \"2 + 2 = 5\" é:",
     {"Verdadeira.", "Falsa.", "Uma pergunta.", "Uma ordem."}, 1,
     "2 + 2 = 4, então a afirmação é falsa. Mesmo falsa, ela continua sendo uma proposição."},
    {TOPIC_TRUTH_VALUE, "Na Lógica Proposicional, os dois valores lógicos são:",
     {"Sim e talvez.", "Certo e quase.", "Verdadeiro e falso.", "Zero e dez."}, 2,
     "Toda proposição recebe um destes dois valores: verdadeiro (V) ou falso (F)."},
    {TOPIC_TRUTH_VALUE, "\"Um triângulo tem três lados.\" Qual é o valor lógico?",
     {"Verdadeiro.", "Falso.", "Depende do dia.", "Não tem valor lógico."}, 0,
     "Todo triângulo tem exatamente três lados, então a proposição é verdadeira."},
    {TOPIC_TRUTH_VALUE, "Nas tabelas de valores lógicos, a letra V representa:",
     {"Vazio.", "Verdadeiro.", "Volume.", "Vento."}, 1,
     "Usamos V para verdadeiro e F para falso."},
    {TOPIC_TRUTH_VALUE, "\"10 é um número ímpar.\" Essa proposição é:",
     {"Verdadeira.", "Uma pergunta.", "Verdadeira e falsa.", "Falsa."}, 3,
     "10 é divisível por 2, ou seja, é par. Por isso a afirmação é falsa."},
    {TOPIC_TRUTH_VALUE, "Nas tabelas de valores lógicos, a letra F representa:",
     {"Falso.", "Fácil.", "Final.", "Forte."}, 0,
     "F indica que a proposição é falsa; V indica que é verdadeira."},

    /* ---------- Negação ---------- */
    {TOPIC_NEGATION, "Qual é a negação de \"Está chovendo\"?",
     {"Está ventando.", "Não está chovendo.", "Está chovendo muito.", "Está nublado."}, 1,
     "A negação afirma que a proposição original não é verdadeira."},
    {TOPIC_NEGATION, "Se P é verdadeiro, então NÃO P é:",
     {"Verdadeiro.", "Verdadeiro e falso.", "Impossível saber.", "Falso."}, 3,
     "A negação inverte o valor lógico: verdadeiro vira falso."},
    {TOPIC_NEGATION, "Se P é falso, então NÃO P é:",
     {"Falso.", "Impossível saber.", "Verdadeiro.", "Falso e verdadeiro."}, 2,
     "A negação inverte o valor lógico: falso vira verdadeiro."},
    {TOPIC_NEGATION, "Qual é a negação de \"O gato é preto\"?",
     {"O gato é branco.", "O cachorro é preto.", "O gato não é preto.", "O gato é grande."}, 2,
     "Negar não é escolher outra cor: basta dizer que o gato não é preto."},
    {TOPIC_NEGATION, "Se P é verdadeiro, qual é o valor de NÃO (NÃO P)?",
     {"Verdadeiro.", "Falso.", "Uma pergunta.", "Impossível saber."}, 0,
     "Negar duas vezes volta ao valor original: V vira F, e F volta a ser V."},
    {TOPIC_NEGATION, "Qual palavra usamos para negar uma proposição?",
     {"E", "OU", "NÃO", "SE"}, 2,
     "O conectivo NÃO forma a negação. E, OU e SE formam outros conectivos."},

    /* ---------- Conjunção (E) ---------- */
    {TOPIC_CONJUNCTION, "Se P é verdadeiro e Q é falso, qual é o resultado de \"P E Q\"?",
     {"Verdadeiro.", "Falso.", "Sempre verdadeiro.", "Não é possível determinar."}, 1,
     "A conjunção E só é verdadeira quando as duas proposições são verdadeiras."},
    {TOPIC_CONJUNCTION, "\"P E Q\" é verdadeira quando:",
     {"P e Q são verdadeiras.", "Só P é verdadeira.", "Só Q é verdadeira.", "P e Q são falsas."}, 0,
     "Com o E, as duas partes precisam ser verdadeiras ao mesmo tempo."},
    {TOPIC_CONJUNCTION, "Se P é verdadeiro e Q é verdadeiro, \"P E Q\" é:",
     {"Verdadeiro.", "Falso.", "Impossível saber.", "Uma pergunta."}, 0,
     "As duas partes são verdadeiras, então a conjunção é verdadeira."},
    {TOPIC_CONJUNCTION, "Qual operação lógica é representada pela palavra \"E\"?",
     {"Negação.", "Disjunção.", "Conjunção.", "Condicional."}, 2,
     "O conectivo E representa a conjunção."},
    {TOPIC_CONJUNCTION, "A frase \"2 é par E 3 é par\" é:",
     {"Verdadeira.", "Uma pergunta.", "Uma ordem.", "Falsa."}, 3,
     "3 é ímpar. Basta uma parte falsa para a conjunção E ser falsa."},
    {TOPIC_CONJUNCTION, "Se P é falso e Q é falso, \"P E Q\" é:",
     {"Verdadeiro.", "Impossível saber.", "Verdadeiro e falso.", "Falso."}, 3,
     "Nenhuma das partes é verdadeira, então a conjunção é falsa."},

    /* ---------- Disjunção inclusiva (OU) ---------- */
    {TOPIC_DISJUNCTION, "Qual operação lógica é representada pela palavra \"OU\"?",
     {"Conjunção.", "Negação.", "Disjunção.", "Bicondicional."}, 2,
     "O conectivo OU representa uma disjunção."},
    {TOPIC_DISJUNCTION, "Se P é verdadeiro e Q é falso, \"P OU Q\" é:",
     {"Verdadeiro.", "Falso.", "Impossível saber.", "Uma ordem."}, 0,
     "Na disjunção OU, basta uma das partes ser verdadeira."},
    {TOPIC_DISJUNCTION, "A disjunção \"P OU Q\" só é falsa quando:",
     {"P e Q são verdadeiras.", "Só P é falsa.", "P e Q são falsas.", "Só Q é falsa."}, 2,
     "O OU só falha quando nenhuma das partes é verdadeira."},
    {TOPIC_DISJUNCTION, "A frase \"O Sol é uma estrela OU 2 + 2 = 5\" é:",
     {"Verdadeira.", "Falsa.", "Uma pergunta.", "Uma ordem."}, 0,
     "\"O Sol é uma estrela\" é verdadeira, e isso basta para o OU ser verdadeiro."},
    {TOPIC_DISJUNCTION, "Se P é falso e Q é falso, \"P OU Q\" é:",
     {"Verdadeiro.", "Impossível saber.", "Verdadeiro e falso.", "Falso."}, 3,
     "Nenhuma parte é verdadeira, então a disjunção é falsa."},
    {TOPIC_DISJUNCTION, "Se P e Q são verdadeiras, \"P OU Q\" é:",
     {"Falso.", "Impossível saber.", "Uma pergunta.", "Verdadeiro."}, 3,
     "No OU inclusivo, as duas partes verdadeiras também tornam a frase verdadeira."},

    /* ---------- Condicional (SE... ENTÃO) ---------- */
    {TOPIC_CONDITIONAL, "Em \"SE chover, ENTÃO levo guarda-chuva\", qual é o conectivo?",
     {"Conjunção (E).", "Condicional (SE... ENTÃO).", "Negação (NÃO).", "Disjunção (OU)."}, 1,
     "A estrutura SE... ENTÃO forma uma proposição condicional."},
    {TOPIC_CONDITIONAL, "\"SE P, ENTÃO Q\" é falsa somente quando:",
     {"P é verdadeiro e Q é falso.", "P é falso e Q é verdadeiro.", "P e Q são verdadeiros.", "P e Q são falsos."}, 0,
     "A condicional só falha quando a condição acontece e o resultado prometido não."},
    {TOPIC_CONDITIONAL, "Se P é verdadeiro e Q é verdadeiro, \"SE P, ENTÃO Q\" é:",
     {"Falso.", "Impossível saber.", "Uma ordem.", "Verdadeiro."}, 3,
     "A condição aconteceu e o resultado também: a condicional é verdadeira."},
    {TOPIC_CONDITIONAL, "Promessa: \"SE fizer sol, ENTÃO vou à praia\". Fez sol e a pessoa não foi. A promessa foi:",
     {"Cumprida.", "Quebrada (falsa).", "Cumprida pela metade.", "Impossível saber."}, 1,
     "A condição (sol) aconteceu, mas o resultado (praia) não: a condicional é falsa."},
    {TOPIC_CONDITIONAL, "Se P é verdadeiro e Q é falso, \"SE P, ENTÃO Q\" é:",
     {"Verdadeiro.", "Impossível saber.", "Falso.", "Verdadeiro e falso."}, 2,
     "É o único caso em que a condicional é falsa: V seguido de F."},

    /* ---------- Tautologia, contradição e contingência ---------- */
    {TOPIC_CLASSIFICATION, "Uma expressão lógica que é verdadeira em todas as possibilidades é chamada de:",
     {"Contradição.", "Contingência.", "Condicional.", "Tautologia."}, 3,
     "Uma tautologia é sempre verdadeira, independentemente dos valores das proposições."},
    {TOPIC_CLASSIFICATION, "Uma expressão lógica que é falsa em todas as possibilidades é chamada de:",
     {"Tautologia.", "Contradição.", "Contingência.", "Conjunção."}, 1,
     "Uma contradição nunca pode ser verdadeira."},
    {TOPIC_CLASSIFICATION, "Uma expressão que pode ser verdadeira ou falsa, dependendo dos valores, é uma:",
     {"Contingência.", "Tautologia.", "Contradição.", "Negação."}, 0,
     "Na contingência, o resultado depende dos valores das proposições."},
    {TOPIC_CLASSIFICATION, "\"P OU NÃO P\" (exemplo: \"Chove ou não chove\") é uma:",
     {"Contradição.", "Contingência.", "Tautologia.", "Negação."}, 2,
     "Sempre uma das partes é verdadeira, então a expressão é sempre verdadeira."},
    {TOPIC_CLASSIFICATION, "\"P E NÃO P\" (exemplo: \"Chove e não chove\") é uma:",
     {"Tautologia.", "Contradição.", "Contingência.", "Disjunção."}, 1,
     "As duas partes nunca são verdadeiras juntas, então a expressão é sempre falsa."},
    {TOPIC_CLASSIFICATION, "A proposição simples \"P\", sozinha, é um exemplo de:",
     {"Tautologia.", "Contradição.", "Contingência.", "Conjunção."}, 2,
     "P pode ser verdadeira ou falsa, então seu valor depende do caso: é contingência."},
};

size_t LogicQuestionCount(void) { return sizeof(bank) / sizeof(*bank); }

const LogicQuestion *LogicQuestionAt(size_t index)
{
    return index < LogicQuestionCount() ? &bank[index] : NULL;
}

const char *LogicTopicName(LogicTopic topic)
{
    static const char *names[TOPIC_COUNT] = {
        "Proposições", "Verdadeiro e falso", "Negação", "Conjunção (E)",
        "Disjunção (OU)", "Condicional (SE... ENTÃO)", "Tautologia, contradição e contingência"};
    return (unsigned)topic < TOPIC_COUNT ? names[topic] : "";
}
