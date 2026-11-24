#ifndef MORPH_INPUT_GAMEPAD_H
#define MORPH_INPUT_GAMEPAD_H
/* Controle (DualShock 4 e outros reconhecidos pela raylib 5.5), sem raylib aqui.
 * Disponível SOMENTE na versão educativa (MORPH_EDUCATIONAL=ON): no arcade o
 * main.c não lê o controle e o jogo se comporta exatamente como antes.
 *
 * main.c lê a raylib pelos botões SEMÂNTICOS (GAMEPAD_BUTTON_RIGHT_FACE_DOWN...),
 * que a raylib/GLFW já traduz pela base SDL_GameControllerDB para cada modelo.
 * Nenhum número bruto de botão é presumido. Este módulo transforma esse retrato
 * em ações e as aplica ao MESMO GameInput/navegação que teclado e mouse usam. */
#include "core/game_types.h"

typedef enum PadButton {
    PAD_CROSS,    /* X (cruz): voar no jogo; confirmar em menus e perguntas */
    PAD_CIRCLE,   /* Círculo: voltar/cancelar onde a ação já existe */
    PAD_SQUARE,   /* Quadrado: megarraio na batalha final */
    PAD_OPTIONS,  /* OPTIONS: pausar/retomar */
    PAD_UP, PAD_DOWN, PAD_LEFT, PAD_RIGHT, /* direcional digital */
    PAD_TRIANGLE, /* Triângulo: liga/desliga o som */
    PAD_L1, PAD_R1, /* na pausa: canal de volume anterior/seguinte */
    PAD_BUTTON_COUNT
} PadButton;

/* Estado bruto de um quadro. */
typedef struct PadSnapshot {
    bool connected;
    int id;                       /* índice da raylib; troca de id = novo controle */
    bool down[PAD_BUTTON_COUNT];
    float stick_x, stick_y;       /* analógico esquerdo, -1..1 (y positivo = baixo) */
} PadSnapshot;

#define PAD_STICK_ENGAGE 0.55f    /* histerese: liga acima disto... */
#define PAD_STICK_RELEASE 0.35f   /* ...e só solta abaixo disto */
#define PAD_REPEAT_DELAY 0.40f    /* s até a primeira repetição de navegação */
#define PAD_REPEAT_INTERVAL 0.18f /* s entre repetições seguintes */

typedef struct PadState {
    bool connected;
    int id;
    bool previous[PAD_BUTTON_COUNT];
    bool stick_engaged;
    int held_direction;           /* -1, 0, +1 */
    bool held_horizontal;
    float repeat_timer;
} PadState;

typedef struct PadActions {
    bool cross, circle, square, options; /* bordas de pressionamento */
    bool triangle, l1, r1;
    int navigate;                        /* -1 anterior, +1 próxima, 0 nada */
    bool horizontal;                     /* o passo veio de ←/→ (na pausa ajusta volume) */
} PadActions;

/* Controles de som pelo controle (pausa: L1/R1 canal, ←/→ volume; Triângulo: mudo). */
typedef struct PadAudio { int select; int adjust; bool toggle_mute; } PadAudio;
PadAudio PadAudioActions(PadActions actions, GameMode mode);

/* true apenas no build educativo; main.c consulta antes de ler o controle. */
bool PadSupported(void);
void PadReset(PadState *state);
/* Conexão/desconexão e troca de controle limpam o estado; um botão já
 * pressionado no momento da conexão só conta depois de solto. */
PadActions PadUpdate(PadState *state, PadSnapshot snapshot, double seconds);

/* Aplica as ações ao fluxo compartilhado de entrada conforme o modo atual.
 * navigation/activate seguem UiQueueNavigation(direção, ativar). */
void PadApply(PadActions actions, GameMode mode, GameInput *input, int *navigation, bool *activate);

const char *PadButtonName(PadButton button);

#endif
