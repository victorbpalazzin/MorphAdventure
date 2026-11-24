#ifndef MORPH_AUDIO_H
#define MORPH_AUDIO_H
/* Reprodução de áudio com raylib 5.5 (dispositivo, recursos, música, efeitos,
 * volumes e preferências). A decisão do QUE tocar fica em audio_cues (puro).
 *
 * Falhas nunca interrompem o jogo: sem dispositivo ou sem arquivos, o jogo
 * continua em silêncio e AudioStatus() descreve o motivo. */
#include "audio/audio_cues.h"

typedef enum AudioChannel { AUDIO_MASTER, AUDIO_MUSIC, AUDIO_SFX, AUDIO_CHANNELS } AudioChannel;

typedef struct AudioSettings {
    float volume[AUDIO_CHANNELS];   /* 0..1 em passos de 0,1 */
    bool muted;
} AudioSettings;

typedef struct AudioStats {
    bool device, enabled;
    unsigned sounds_loaded, sounds_missing, music_loaded, music_missing;
    unsigned cues_played, voices_busy;
    MusicTrack current;
    float music_gain;               /* ganho efetivo da faixa atual (após fades e volumes) */
    bool music_paused;
    char directory[512];
} AudioStats;

/* Abre o dispositivo uma única vez e carrega tudo de <exe>/assets/audio
 * (ou ./assets/audio). settings_path: arquivo de preferências; NULL usa o padrão. */
bool AudioInit(const char *settings_path);
/* Igual, com a pasta de recursos explícita (testes e ferramentas). */
bool AudioInitFrom(const char *assets_dir, const char *settings_path);
void AudioShutdown(void);
/* Uma vez por quadro, depois de GameAdvance: decide e toca. */
void AudioFrame(const GameState *game, int ui_focus, double seconds);

AudioSettings AudioGetSettings(void);
void AudioSetSettings(AudioSettings settings);   /* aplica e salva */
void AudioAdjust(AudioChannel channel, int steps); /* ±1 = ±10% (com efeito de confirmação) */
void AudioToggleMute(void);
AudioSettings AudioDefaultSettings(void);
/* Preferências em texto simples "chave=valor" (seguras para pasta portátil). */
bool AudioSettingsParse(const char *text, AudioSettings *out);
int AudioSettingsFormat(AudioSettings settings, char *out, int size);

AudioStats AudioGetStats(void);
const char *AudioStatus(void);
/* Para testes: toca um efeito diretamente. */
void AudioPlayCue(AudioCueEvent cue);

#endif
