#ifndef MORPH_AUDIO_CUES_H
#define MORPH_AUDIO_CUES_H
/* Diretor de áudio: lógica PURA (sem raylib) que decide o que soa.
 *
 * Entradas, uma vez por quadro renderizado:
 *  - "taps" de eventos reais do jogo, enfileirados por composition.c durante
 *    GameAdvance (coleta, boost, dano, portal, batida de asa, perigos, ambiente);
 *  - transições do GameState (modo, mundo, fases e ataques do boss);
 *  - foco da UI (navegação em menus).
 * Saídas: lista de efeitos a tocar e o plano musical (faixa, camadas, nível,
 * pausa, atenuação). Cada evento gera um único efeito; nada é disparado só
 * porque uma condição continua verdadeira. */
#include "core/game_types.h"

typedef enum AudioCue {
    CUE_NONE,
    CUE_FLAP, CUE_HIT, CUE_DEFEAT, CUE_LIFE_UP, CUE_COLLECT, CUE_BATTERY_FULL,
    CUE_BOOST_SHIELD, CUE_BOOST_MAGNET, CUE_BOOST_SLOW, CUE_BOOST_DOUBLE, CUE_BOOST_PHASE, CUE_SHIELD_BREAK,
    CUE_PORTAL_OPEN, CUE_PORTAL_ENTER, CUE_WORLD_WHOOSH,
    CUE_ENV_WARNING, CUE_ENV_EVENT,          /* param: mundo 0..5 */
    CUE_HAZARD_WARNING, CUE_HAZARD,          /* param: ObstacleHazardType */
    CUE_BOSS_APPEAR, CUE_BOSS_WARNING, CUE_BOSS_SHOT, CUE_COMBAT_ENERGY, CUE_BATTERY_READY,
    CUE_BEAM, CUE_BOSS_HURT, CUE_BOSS_PHASE, CUE_BOSS_DEFEAT,
    CUE_UI_MOVE, CUE_UI_CONFIRM, CUE_UI_BACK, CUE_UI_PAUSE, CUE_UI_RESUME, CUE_UI_START, CUE_UI_VOLUME,
    /* Versão educativa */
    CUE_LETTER, CUE_QUIZ_OPEN, CUE_QUIZ_SELECT, CUE_QUIZ_CORRECT, CUE_QUIZ_WRONG, CUE_QUIZ_TICK,
    CUE_QUIZ_TIMEOUT, CUE_QUIZ_NEW, CUE_QUIZ_UNLOCK, CUE_REVIVE_OPEN, CUE_REVIVE_SUCCESS, CUE_REVIVE_FAIL,
    CUE_COUNT
} AudioCue;

typedef enum MusicTrack {
    MUSIC_NONE, MUSIC_MENU, MUSIC_AR, MUSIC_AGUA, MUSIC_FLORESTA, MUSIC_GELO, MUSIC_TERRA, MUSIC_FOGO,
    MUSIC_BOSS, MUSIC_VICTORY, MUSIC_DEFEAT, MUSIC_COUNT
} MusicTrack;

#define AUDIO_BOSS_LAYERS 3
#define AUDIO_MAX_CUES 24
#define AUDIO_MAX_TAPS 64
#define AUDIO_QUIZ_DUCK 0.3f   /* música do mundo atenuada para favorecer leitura e concentração */

typedef enum AudioTapKind {
    TAP_GAME_EVENT,      /* param: GameEventType, boost: BoostType */
    TAP_FLAP,
    TAP_HAZARD_SPAWN,    /* param: tipo do perigo (ainda em aviso) */
    TAP_HAZARD_LAUNCH,   /* param: tipo do perigo (ficou perigoso) */
    TAP_ENV_WARNING,     /* param: mundo */
    TAP_ENV_ACTIVE       /* param: mundo */
} AudioTapKind;

typedef struct AudioTap { AudioTapKind kind; int param; int boost; float pan; } AudioTap;

typedef struct AudioCueEvent {
    AudioCue cue;
    int param;           /* mundo/tipo para efeitos com variações por contexto */
    int variant;         /* variação de arquivo (ex.: 3 batidas de asa) */
    float pitch;         /* 1 = original */
    float gain;          /* 0..1, antes dos volumes do jogador */
    float pan;           /* -1 esquerda .. 1 direita */
} AudioCueEvent;

typedef struct MusicPlan {
    MusicTrack track;
    float level;                         /* 0..1: alvo da faixa atual */
    float layers[AUDIO_BOSS_LAYERS];     /* camadas da batalha (fases 1, 2 e 3) */
    float fade_seconds;                  /* tempo de transição para o novo alvo */
    float duck;                          /* 1 normal; AUDIO_QUIZ_DUCK durante os desafios */
    bool paused;                         /* pausa/perda de foco congelam músicas e efeitos */
} MusicPlan;

typedef struct AudioDirector {
    bool started;
    GameMode mode;
    int world, lives, ui_focus;
    uint32_t world_id, run_id;
    unsigned boss_projectiles, boss_segments, boss_phase, combat_collected;
    BossStage boss_stage;
    bool boss_warning, beam_active;
    unsigned quiz_serial;                /* desafio educativo: muda a cada pergunta/fase */
    int quiz_phase, quiz_second;
    float cooldown[CUE_COUNT];
    uint32_t seed;
    unsigned tap_count;
    bool tap_overflow;
    AudioTap taps[AUDIO_MAX_TAPS];
    unsigned cue_count;
    AudioCueEvent cues[AUDIO_MAX_CUES];
    MusicPlan plan;
    MusicTrack last_world_track;
} AudioDirector;

void AudioDirectorReset(AudioDirector *director);
/* Enfileira um evento real (chamado por composition.c dentro de GameAdvance). */
void AudioDirectorTap(AudioDirector *director, AudioTap tap);
/* Processa taps + transições; preenche cues[] e plan. seconds = tempo do quadro. */
void AudioDirectorUpdate(AudioDirector *director, const GameState *game, int ui_focus, double seconds);
/* Instância compartilhada usada pelo jogo (composition.c alimenta, main.c consome). */
AudioDirector *AudioDirectorShared(void);
MusicTrack AudioWorldTrack(int world);
const char *AudioCueName(AudioCue cue);
/* Semitons da coleta k (1..required): pequena melodia pentatônica ascendente. */
float AudioCollectPitch(unsigned collected, unsigned required);

#endif
