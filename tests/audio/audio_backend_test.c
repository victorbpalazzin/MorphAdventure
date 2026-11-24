/* Reprodução real com raylib 5.5 (src/audio/audio.c). Sem placa de som, o teste
 * verifica o modo silencioso e informa SKIP nas partes que exigem dispositivo. */
#include "audio/audio.h"
#include "core/game.h"
#include "raylib.h"
#include "morph_check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static GameState g;
static void Frames(double seconds) { for (double t = 0; t < seconds - 1e-9; t += 1.0 / 60) AudioFrame(&g, -1, 1.0 / 60); }
static void Mode(GameMode m) { g.mode = m; AudioFrame(&g, -1, 1.0 / 60); }

static bool Same(AudioSettings a, AudioSettings b)
{
    for (int c = 0; c < AUDIO_CHANNELS; c++) if (fabsf(a.volume[c] - b.volume[c]) > 1e-5f) return false;
    return a.muted == b.muted;
}

static void CheckSettings(void)
{
    AudioSettings s, d = AudioDefaultSettings();
    char text[256];
    MORPH_CHECK(AudioSettingsFormat(d, text, sizeof(text)) > 0);
    MORPH_CHECK(AudioSettingsParse(text, &s) && Same(s, d));
    MORPH_CHECK(AudioSettingsParse("geral=0.33\nmusica=7\nefeitos=-1\nmudo=1\n", &s));
    MORPH_CHECK(fabsf(s.volume[AUDIO_MASTER] - 0.3f) < 1e-6f && s.volume[AUDIO_MUSIC] == 1.0f && s.volume[AUDIO_SFX] == 0.0f && s.muted);
    s = d;
    MORPH_CHECK(!AudioSettingsParse("lixo\n=\nmusica=abc\n", &s) && Same(s, d)); /* texto inválido ignorado */
}

static void CheckMissingDirectory(void)
{
    MORPH_CHECK(!AudioInitFrom("pasta-que-nao-existe/assets/audio", "audio-test-prefs.cfg"));
    MORPH_CHECK(strstr(AudioStatus(), "nao encontrada") != NULL);
    g.mode = GAME_PLAY; Frames(0.5);                     /* jogo segue em silêncio, sem travar */
    AudioPlayCue((AudioCueEvent){CUE_BEAM, 0, 0, 1, 1, 0});
    AudioToggleMute(); AudioToggleMute();
    AudioShutdown();
}

static void CheckDevice(const char *assets, const char *broken)
{
    remove("audio-test-prefs.cfg");
    bool ok = AudioInitFrom(assets, "audio-test-prefs.cfg");
    AudioStats st = AudioGetStats();
    if (!st.device) {
        printf("SKIP (sem dispositivo de som): %s\n", AudioStatus());
        MORPH_CHECK(!ok && strstr(AudioStatus(), "dispositivo") != NULL);
        AudioShutdown();
        return;
    }
    MORPH_CHECK(ok && st.enabled);
    printf("dispositivo OK: %u efeitos e %u musicas\n", st.sounds_loaded, st.music_loaded);
    MORPH_CHECK(st.sounds_loaded == 47 && st.sounds_missing == 0 && st.music_loaded == 12 && st.music_missing == 0);
    MORPH_CHECK(AudioInitFrom(assets, "audio-test-prefs.cfg"));   /* segunda chamada não reabre o dispositivo */
    MORPH_CHECK(AudioGetStats().sounds_loaded == 47);
    /* todos os efeitos tocam (com variações de arquivo) */
    for (int c = CUE_NONE + 1; c < CUE_COUNT; c++)
        for (int p = 0; p < 6; p++) AudioPlayCue((AudioCueEvent){(AudioCue)c, p, p % 3, 1, 0.5f, 0});
    MORPH_CHECK(AudioGetStats().voices_busy > 20 && AudioGetStats().cues_played == (CUE_COUNT - 1) * 6);
    /* polifonia limitada: dezenas de coletas não criam vozes infinitas */
    for (int i = 0; i < 50; i++) AudioPlayCue((AudioCueEvent){CUE_COLLECT, 0, 0, 1, 0.5f, 0});
    MORPH_CHECK(AudioGetStats().voices_busy < 120);
    /* música do menu entra com fade, sem salto */
    memset(&g, 0, sizeof(g)); g.run_id = 1; g.world_id = 1; g.lives = 3; g.minimum_distance = 1000;
    Mode(GAME_INTRO);
    float first = AudioGetStats().music_gain;
    Frames(0.5);
    float mid = AudioGetStats().music_gain;
    Frames(2.0);
    AudioSettings s = AudioGetSettings();
    float full = s.volume[AUDIO_MASTER] * s.volume[AUDIO_MUSIC];
    MORPH_CHECK(AudioGetStats().current == MUSIC_MENU);
    MORPH_CHECK(first < mid && mid < full && fabsf(AudioGetStats().music_gain - full) < 1e-3f);
    /* pausa congela a música; retomar não reinicia */
    Mode(GAME_PLAY); Frames(3.0);
    MORPH_CHECK(AudioGetStats().current == MUSIC_AR && fabsf(AudioGetStats().music_gain - full) < 1e-3f);
    g.resume_mode = GAME_PLAY; Mode(GAME_PAUSED); Frames(1.0);
    MORPH_CHECK(AudioGetStats().music_paused && AudioGetStats().current == MUSIC_AR);
    Mode(GAME_PLAY); Frames(0.1);
    MORPH_CHECK(!AudioGetStats().music_paused && AudioGetStats().current == MUSIC_AR);
    /* portal: a faixa sai, a do novo mundo entra */
    Mode(GAME_PORTAL); Mode(GAME_TRANSITION); Frames(1.5);
    MORPH_CHECK(AudioGetStats().music_gain < 0.05f);
    g.world = 1; g.world_id++; Frames(3.0); Mode(GAME_PLAY);
    MORPH_CHECK(AudioGetStats().current == MUSIC_AGUA && fabsf(AudioGetStats().music_gain - full) < 1e-3f);
    /* batalha com camadas; vitória e derrota */
    g.mode = GAME_BOSS; g.world = 5; g.boss.stage = BOSS_FIGHT; g.boss.phase = 1; g.boss.segments = 3; Frames(3.0);
    MORPH_CHECK(AudioGetStats().current == MUSIC_BOSS);
    g.boss.phase = 3; g.boss.segments = 1; Frames(3.0);
    MORPH_CHECK(AudioGetStats().current == MUSIC_BOSS && AudioGetStats().music_gain > 0);
    g.boss.stage = BOSS_DYING; Frames(1.5); MORPH_CHECK(AudioGetStats().music_gain < 0.05f);
    g.boss.stage = BOSS_DONE; Mode(GAME_WON); Frames(1.0); MORPH_CHECK(AudioGetStats().current == MUSIC_VICTORY);
    memset(&g.boss, 0, sizeof(g.boss)); g.run_id++; Mode(GAME_PLAY); Mode(GAME_LOST); Frames(1.0);
    MORPH_CHECK(AudioGetStats().current == MUSIC_DEFEAT);
    /* volume, mudo e preferências salvas */
    AudioToggleMute(); Frames(0.1); MORPH_CHECK(AudioGetSettings().muted && AudioGetStats().music_gain == 0);
    AudioToggleMute(); MORPH_CHECK(!AudioGetSettings().muted);
    AudioAdjust(AUDIO_MUSIC, -3);
    MORPH_CHECK(fabsf(AudioGetSettings().volume[AUDIO_MUSIC] - (s.volume[AUDIO_MUSIC] - 0.3f)) < 1e-4f);
    FILE *f = fopen("audio-test-prefs.cfg", "r");
    MORPH_CHECK(f != NULL);
    char saved[256] = {0}; MORPH_CHECK(fread(saved, 1, sizeof(saved) - 1, f) > 0); fclose(f);
    AudioSettings back;
    MORPH_CHECK(AudioSettingsParse(saved, &back) && fabsf(back.volume[AUDIO_MUSIC] - AudioGetSettings().volume[AUDIO_MUSIC]) < 1e-4f);
    AudioShutdown();
    /* preferências recarregadas na próxima abertura */
    MORPH_CHECK(AudioInitFrom(assets, "audio-test-prefs.cfg"));
    MORPH_CHECK(fabsf(AudioGetSettings().volume[AUDIO_MUSIC] - back.volume[AUDIO_MUSIC]) < 1e-4f);
    AudioShutdown();
    /* arquivos ausentes misturados com válidos: carrega o resto e não trava */
    MORPH_CHECK(AudioInitFrom(broken, "audio-test-prefs.cfg"));
    st = AudioGetStats();
    MORPH_CHECK(st.sounds_missing == 1 && st.music_missing == 1 && st.sounds_loaded == 46 && st.music_loaded == 11);
    AudioPlayCue((AudioCueEvent){CUE_BEAM, 0, 0, 1, 1, 0});            /* removido: ignorado com segurança */
    memset(&g, 0, sizeof(g)); g.mode = GAME_PLAY; Frames(1.0);         /* ar.ogg removido: segue sem música */
    MORPH_CHECK(AudioGetStats().current == MUSIC_NONE);
    AudioShutdown();
    remove("audio-test-prefs.cfg");
}

int main(int argc, char **argv)
{
    MORPH_CHECK(argc == 3);
    SetTraceLogLevel(LOG_WARNING);
    CheckSettings();
    CheckMissingDirectory();
    CheckDevice(argv[1], argv[2]);
    puts("PASS audio-backend: preferencias, modo silencioso, carregamento, efeitos, fades, pausa, portal, "
         "camadas do boss, vitoria/derrota, mudo e arquivos ausentes");
    return 0;
}
