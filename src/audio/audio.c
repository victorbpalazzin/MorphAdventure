#include "audio.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOICES_MAX 4      /* polifonia por efeito (aliases que compartilham os dados) */
#define FILES_MAX 6       /* variações de arquivo por efeito (ex.: batidas de asa) */

typedef struct SfxDef { AudioCue cue; const char *files[FILES_MAX]; int voices; float gain; } SfxDef;

/* Ganho base por efeito: mistura relativa pensada para não competir com a trilha. */
static const SfxDef sfx_defs[] = {
    {CUE_FLAP, {"flap_1", "flap_2", "flap_3"}, 3, 0.55f},
    {CUE_HIT, {"hit"}, 2, 0.9f}, {CUE_DEFEAT, {"defeat"}, 1, 0.9f}, {CUE_LIFE_UP, {"life_up"}, 1, 0.85f},
    {CUE_COLLECT, {"collect"}, 4, 0.75f}, {CUE_BATTERY_FULL, {"battery_full"}, 1, 0.85f},
    {CUE_BOOST_SHIELD, {"boost_shield"}, 1, 0.8f}, {CUE_BOOST_MAGNET, {"boost_magnet"}, 1, 0.8f},
    {CUE_BOOST_SLOW, {"boost_slow"}, 1, 0.8f}, {CUE_BOOST_DOUBLE, {"boost_double"}, 1, 0.8f},
    {CUE_BOOST_PHASE, {"boost_phase"}, 1, 0.8f}, {CUE_SHIELD_BREAK, {"shield_break"}, 2, 0.85f},
    {CUE_PORTAL_OPEN, {"portal_open"}, 1, 0.8f}, {CUE_PORTAL_ENTER, {"portal_enter"}, 1, 0.85f},
    {CUE_WORLD_WHOOSH, {"world_whoosh"}, 1, 0.8f},
    {CUE_ENV_WARNING, {"env_warning"}, 1, 0.6f},
    {CUE_ENV_EVENT, {"env_ar", "env_agua", "env_floresta", "env_gelo", "env_terra", "env_fogo"}, 1, 0.65f},
    {CUE_HAZARD_WARNING, {"hazard_warning"}, 2, 0.5f},
    /* ordem de ObstacleHazardType: JELLY, ICE, FIRE, WIND, VINE, ROCK */
    {CUE_HAZARD, {"hazard_jelly", "hazard_ice", "hazard_fire", "hazard_wind", "hazard_vine", "hazard_rock"}, 2, 0.7f},
    {CUE_BOSS_APPEAR, {"boss_appear"}, 1, 0.95f}, {CUE_BOSS_WARNING, {"boss_warning"}, 2, 0.7f},
    {CUE_BOSS_SHOT, {"boss_shot"}, 3, 0.7f}, {CUE_COMBAT_ENERGY, {"combat_energy"}, 3, 0.8f},
    {CUE_BATTERY_READY, {"battery_ready"}, 1, 0.85f}, {CUE_BEAM, {"beam"}, 1, 1.0f},
    {CUE_BOSS_HURT, {"boss_hurt"}, 1, 0.9f}, {CUE_BOSS_PHASE, {"boss_phase"}, 1, 0.9f},
    {CUE_BOSS_DEFEAT, {"boss_defeat"}, 1, 1.0f},
    {CUE_UI_MOVE, {"ui_move"}, 2, 0.6f}, {CUE_UI_CONFIRM, {"ui_confirm"}, 1, 0.75f}, {CUE_UI_BACK, {"ui_back"}, 1, 0.7f},
    {CUE_UI_PAUSE, {"ui_pause"}, 1, 0.7f}, {CUE_UI_RESUME, {"ui_resume"}, 1, 0.7f}, {CUE_UI_START, {"ui_start"}, 1, 0.85f},
    {CUE_UI_VOLUME, {"ui_volume"}, 2, 0.7f},
    /* versão educativa */
    {CUE_LETTER, {"letter"}, 4, 0.75f}, {CUE_QUIZ_OPEN, {"quiz_open"}, 1, 0.8f}, {CUE_QUIZ_SELECT, {"quiz_select"}, 2, 0.6f},
    {CUE_QUIZ_CORRECT, {"quiz_correct"}, 1, 0.85f}, {CUE_QUIZ_WRONG, {"quiz_wrong"}, 1, 0.75f},
    {CUE_QUIZ_TICK, {"quiz_tick"}, 2, 0.6f}, {CUE_QUIZ_TIMEOUT, {"quiz_timeout"}, 1, 0.8f}, {CUE_QUIZ_NEW, {"quiz_new"}, 1, 0.7f},
    {CUE_QUIZ_UNLOCK, {"quiz_unlock"}, 1, 0.85f}, {CUE_REVIVE_OPEN, {"revive_open"}, 1, 0.85f},
    {CUE_REVIVE_SUCCESS, {"revive_success"}, 1, 0.9f}, {CUE_REVIVE_FAIL, {"revive_fail"}, 1, 0.85f},
};

typedef struct MusicDef { MusicTrack track; const char *files[AUDIO_BOSS_LAYERS]; bool loop; } MusicDef;
static const MusicDef music_defs[] = {
    {MUSIC_MENU, {"menu"}, true}, {MUSIC_AR, {"ar"}, true}, {MUSIC_AGUA, {"agua"}, true},
    {MUSIC_FLORESTA, {"floresta"}, true}, {MUSIC_GELO, {"gelo"}, true}, {MUSIC_TERRA, {"terra"}, true},
    {MUSIC_FOGO, {"fogo"}, true}, {MUSIC_BOSS, {"boss_1", "boss_2", "boss_3"}, true},
    {MUSIC_VICTORY, {"vitoria"}, false}, {MUSIC_DEFEAT, {"derrota"}, false},
};

typedef struct SfxSlot {
    bool valid[FILES_MAX];
    Sound source[FILES_MAX];
    Sound voice[FILES_MAX][VOICES_MAX];
    bool paused[FILES_MAX][VOICES_MAX];
    int next[FILES_MAX];
    int voices;
    float gain;
} SfxSlot;

typedef struct Deck {
    MusicTrack track;
    float gain, target, rate;
    float layer[AUDIO_BOSS_LAYERS];
    bool active;
} Deck;

static struct {
    bool device, enabled, paused;
    char dir[512], settings_path[600], status[256];
    SfxSlot sfx[CUE_COUNT];
    Music music[MUSIC_COUNT][AUDIO_BOSS_LAYERS];
    bool music_valid[MUSIC_COUNT][AUDIO_BOSS_LAYERS];
    bool music_loop[MUSIC_COUNT];
    Deck deck[2];
    int current;
    float duck;
    AudioSettings settings;
    AudioStats stats;
    MusicTrack requested;
} A;

/* ------------------------------------------------------------ limitador final
 * Roda na thread de áudio sobre a mixagem completa (float estéreo da raylib).
 * Ganho cai instantaneamente quando o pico passa de -1 dBFS e volta em ~120 ms:
 * nenhuma combinação de volumes, música e efeitos chega a clipar. */
static float limiter_gain = 1.0f;
static unsigned long limiter_hits;
static void Limiter(void *data, unsigned int frames)
{
    float *x = data;
    const float ceiling = 0.89f, release = 1.0f - 1.0f / (0.12f * 48000.0f);
    for (unsigned int i = 0; i < frames; i++) {
        float l = x[2 * i], r = x[2 * i + 1];
        float peak = fmaxf(fabsf(l), fabsf(r)) * limiter_gain;
        if (peak > ceiling) { limiter_gain *= ceiling / peak; limiter_hits++; }
        else limiter_gain = fminf(1.0f, 1.0f - (1.0f - limiter_gain) * release);
        x[2 * i] = l * limiter_gain;
        x[2 * i + 1] = r * limiter_gain;
    }
}

/* ------------------------------------------------------------ preferências */
AudioSettings AudioDefaultSettings(void)
{
    AudioSettings s = {{0.8f, 0.7f, 0.9f}, false};
    return s;
}

static float Clamp01(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
static float Step10(float v) { return floorf(Clamp01(v) * 10 + 0.5f) / 10; }

bool AudioSettingsParse(const char *text, AudioSettings *out)
{
    if (!text || !out) return false;
    AudioSettings s = AudioDefaultSettings();
    static const char *keys[AUDIO_CHANNELS] = {"geral", "musica", "efeitos"};
    bool any = false;
    const char *p = text;
    while (*p) {
        char line[96] = {0};
        int n = 0;
        while (*p && *p != '\n' && n < 95) line[n++] = *p++;
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '#') continue;
        *eq = '\0';
        char *value = eq + 1, *end = NULL;
        double v = strtod(value, &end);
        if (end == value || !isfinite(v)) continue;
        for (int c = 0; c < AUDIO_CHANNELS; c++)
            if (strcmp(line, keys[c]) == 0) { s.volume[c] = Step10((float)v); any = true; }
        if (strcmp(line, "mudo") == 0) { s.muted = v != 0; any = true; }
    }
    if (any) *out = s;
    return any;
}

int AudioSettingsFormat(AudioSettings s, char *out, int size)
{
    return snprintf(out, (size_t)size, "# Morph Adventure - preferencias de audio (0.0 a 1.0)\n"
                    "geral=%.1f\nmusica=%.1f\nefeitos=%.1f\nmudo=%d\n",
                    Step10(s.volume[AUDIO_MASTER]), Step10(s.volume[AUDIO_MUSIC]), Step10(s.volume[AUDIO_SFX]), s.muted ? 1 : 0);
}

static void SaveSettings(void)
{
    if (!A.settings_path[0]) return;
    char text[256];
    AudioSettingsFormat(A.settings, text, sizeof(text));
    FILE *f = fopen(A.settings_path, "w");
    if (!f) return; /* pasta somente leitura: preferências valem só nesta sessão */
    fputs(text, f);
    fclose(f);
}

static void LoadSettings(void)
{
    A.settings = AudioDefaultSettings();
    FILE *f = A.settings_path[0] ? fopen(A.settings_path, "r") : NULL;
    if (!f) return;
    char text[512] = {0};
    size_t n = fread(text, 1, sizeof(text) - 1, f);
    text[n] = '\0';
    fclose(f);
    AudioSettingsParse(text, &A.settings);
}

/* ------------------------------------------------------------ recursos */
static bool FindDirectory(const char *explicit_dir)
{
    if (explicit_dir) {
        snprintf(A.dir, sizeof(A.dir), "%s", explicit_dir);
        return DirectoryExists(explicit_dir);
    }
    const char *app = GetApplicationDirectory();
    const char *candidates[] = {TextFormat("%sassets/audio", app), "assets/audio", "../assets/audio"};
    for (size_t i = 0; i < sizeof(candidates) / sizeof(*candidates); i++) {
        if (DirectoryExists(candidates[i])) {
            snprintf(A.dir, sizeof(A.dir), "%s", candidates[i]);
            return true;
        }
    }
    A.dir[0] = '\0';
    return false;
}

static const char *Existing(const char *sub, const char *name)
{
    static char path[700];
    static const char *exts[] = {".wav", ".ogg"};
    for (int e = 0; e < 2; e++) {
        snprintf(path, sizeof(path), "%s/%s/%s%s", A.dir, sub, name, exts[e]);
        if (FileExists(path)) return path;
    }
    return NULL;
}

static void LoadAll(void)
{
    for (size_t i = 0; i < sizeof(sfx_defs) / sizeof(*sfx_defs); i++) {
        const SfxDef *d = &sfx_defs[i];
        SfxSlot *slot = &A.sfx[d->cue];
        slot->voices = d->voices < 1 ? 1 : d->voices > VOICES_MAX ? VOICES_MAX : d->voices;
        slot->gain = d->gain;
        for (int f = 0; f < FILES_MAX && d->files[f]; f++) {
            const char *path = Existing("sfx", d->files[f]);
            Sound s = path ? LoadSound(path) : (Sound){0};
            if (!path || !IsSoundValid(s)) { A.stats.sounds_missing++; TraceLog(LOG_WARNING, "AUDIO: efeito ausente: %s", d->files[f]); continue; }
            slot->source[f] = s;
            slot->valid[f] = true;
            slot->voice[f][0] = s;
            for (int v = 1; v < slot->voices; v++) slot->voice[f][v] = LoadSoundAlias(s);
            A.stats.sounds_loaded++;
        }
    }
    for (size_t i = 0; i < sizeof(music_defs) / sizeof(*music_defs); i++) {
        const MusicDef *d = &music_defs[i];
        A.music_loop[d->track] = d->loop;
        for (int l = 0; l < AUDIO_BOSS_LAYERS && d->files[l]; l++) {
            const char *path = Existing("music", d->files[l]);
            Music m = path ? LoadMusicStream(path) : (Music){0};
            if (!path || !IsMusicValid(m)) { A.stats.music_missing++; TraceLog(LOG_WARNING, "AUDIO: musica ausente: %s", d->files[l]); continue; }
            m.looping = d->loop;
            A.music[d->track][l] = m;
            A.music_valid[d->track][l] = true;
            A.stats.music_loaded++;
        }
    }
}

bool AudioInit(const char *settings_path) { return AudioInitFrom(NULL, settings_path); }

bool AudioInitFrom(const char *assets_dir, const char *settings_path)
{
    if (A.device || A.enabled) return A.enabled; /* nunca inicializa duas vezes */
    memset(&A, 0, sizeof(A));
    A.duck = 1;
    const char *app = GetApplicationDirectory();
    if (settings_path) snprintf(A.settings_path, sizeof(A.settings_path), "%s", settings_path);
    else snprintf(A.settings_path, sizeof(A.settings_path), "%smorph-audio.cfg", app);
    LoadSettings();
    if (!FindDirectory(assets_dir)) {
        snprintf(A.status, sizeof(A.status), "Audio desativado: pasta assets/audio nao encontrada.");
        TraceLog(LOG_WARNING, "AUDIO: %s", A.status);
        return false;
    }
    InitAudioDevice();
    A.device = IsAudioDeviceReady();
    if (!A.device) {
        snprintf(A.status, sizeof(A.status), "Audio desativado: dispositivo de som indisponivel.");
        TraceLog(LOG_WARNING, "AUDIO: %s", A.status);
        return false;
    }
    LoadAll();
    limiter_gain = 1.0f;
    AttachAudioMixedProcessor(Limiter);
    A.enabled = A.stats.sounds_loaded + A.stats.music_loaded > 0;
    snprintf(A.status, sizeof(A.status), "Audio: %u efeitos e %u musicas carregados (%u e %u ausentes) de %s",
             A.stats.sounds_loaded, A.stats.music_loaded, A.stats.sounds_missing, A.stats.music_missing, A.dir);
    TraceLog(LOG_INFO, "AUDIO: %s", A.status);
    return A.enabled;
}

void AudioShutdown(void)
{
    if (A.device) {
        for (int t = 0; t < MUSIC_COUNT; t++)
            for (int l = 0; l < AUDIO_BOSS_LAYERS; l++)
                if (A.music_valid[t][l]) { StopMusicStream(A.music[t][l]); UnloadMusicStream(A.music[t][l]); }
        for (int c = 0; c < CUE_COUNT; c++) {
            SfxSlot *slot = &A.sfx[c];
            for (int f = 0; f < FILES_MAX; f++) {
                if (!slot->valid[f]) continue;
                for (int v = 1; v < slot->voices; v++) UnloadSoundAlias(slot->voice[f][v]);
                UnloadSound(slot->source[f]);
            }
        }
        DetachAudioMixedProcessor(Limiter);
        CloseAudioDevice();
    }
    char path[600];
    memcpy(path, A.settings_path, sizeof(path));
    memset(&A, 0, sizeof(A));
    memcpy(A.settings_path, path, sizeof(path));
}

/* ------------------------------------------------------------ volumes */
static float Volume(AudioChannel c)
{
    if (A.settings.muted) return 0;
    return A.settings.volume[AUDIO_MASTER] * A.settings.volume[c];
}

AudioSettings AudioGetSettings(void) { return A.settings; }

void AudioSetSettings(AudioSettings s)
{
    for (int c = 0; c < AUDIO_CHANNELS; c++) s.volume[c] = Step10(s.volume[c]);
    A.settings = s;
    SaveSettings();
}

void AudioAdjust(AudioChannel channel, int steps)
{
    if (channel < 0 || channel >= AUDIO_CHANNELS || !steps) return;
    AudioSettings s = A.settings;
    s.volume[channel] = Step10(s.volume[channel] + 0.1f * (float)steps);
    s.muted = false;
    AudioSetSettings(s);
    AudioPlayCue((AudioCueEvent){CUE_UI_VOLUME, 0, 0, 0.8f + 0.4f * s.volume[channel], 1.0f, 0});
}

void AudioToggleMute(void)
{
    AudioSettings s = A.settings;
    s.muted = !s.muted;
    AudioSetSettings(s);
    if (!s.muted) AudioPlayCue((AudioCueEvent){CUE_UI_CONFIRM, 0, 0, 1, 0.8f, 0});
}

/* ------------------------------------------------------------ efeitos */
void AudioPlayCue(AudioCueEvent e)
{
    if (!A.enabled || e.cue <= CUE_NONE || e.cue >= CUE_COUNT) return;
    SfxSlot *slot = &A.sfx[e.cue];
    int file = e.cue == CUE_ENV_EVENT || e.cue == CUE_HAZARD ? e.param : e.variant;
    if (file < 0 || file >= FILES_MAX || !slot->valid[file]) file = 0;
    if (!slot->valid[file]) return;
    /* próxima voz livre; se todas tocam, reaproveita a mais antiga (rodízio) */
    int v = slot->next[file];
    for (int k = 0; k < slot->voices; k++) {
        int candidate = (slot->next[file] + k) % slot->voices;
        if (!IsSoundPlaying(slot->voice[file][candidate])) { v = candidate; break; }
    }
    slot->next[file] = (v + 1) % slot->voices;
    Sound s = slot->voice[file][v];
    SetSoundVolume(s, Clamp01(e.gain * slot->gain * Volume(AUDIO_SFX)));
    SetSoundPitch(s, e.pitch > 0.1f ? e.pitch : 1.0f);
    SetSoundPan(s, Clamp01(0.5f + 0.5f * e.pan));
    slot->paused[file][v] = false;
    PlaySound(s);
    A.stats.cues_played++;
}

static void PauseEffects(bool pause)
{
    for (int c = 0; c < CUE_COUNT; c++) {
        SfxSlot *slot = &A.sfx[c];
        for (int f = 0; f < FILES_MAX; f++) {
            if (!slot->valid[f]) continue;
            for (int v = 0; v < slot->voices; v++) {
                Sound s = slot->voice[f][v];
                if (pause && IsSoundPlaying(s)) { PauseSound(s); slot->paused[f][v] = true; }
                else if (!pause && slot->paused[f][v]) { ResumeSound(s); slot->paused[f][v] = false; }
            }
        }
    }
}

/* ------------------------------------------------------------ música */
static bool TrackValid(MusicTrack t) { return t > MUSIC_NONE && t < MUSIC_COUNT && A.music_valid[t][0]; }

static void DeckStop(Deck *d)
{
    if (d->active && TrackValid(d->track))
        for (int l = 0; l < AUDIO_BOSS_LAYERS; l++) if (A.music_valid[d->track][l]) StopMusicStream(A.music[d->track][l]);
    memset(d, 0, sizeof(*d));
}

static void DeckStart(Deck *d, MusicTrack t, float fade)
{
    DeckStop(d);
    if (!TrackValid(t)) return;
    d->track = t;
    d->active = true;
    d->gain = 0;
    d->rate = fade > 0.01f ? 1.0f / fade : 100.0f;
    for (int l = 0; l < AUDIO_BOSS_LAYERS; l++) {
        if (!A.music_valid[t][l]) continue;
        /* camadas começam juntas na mesma posição: permanecem sincronizadas */
        SeekMusicStream(A.music[t][l], 0);
        PlayMusicStream(A.music[t][l]);
    }
}

static float Approach(float value, float target, float step)
{
    return value < target ? fminf(target, value + step) : fmaxf(target, value - step);
}

static void MusicFrame(const MusicPlan *p, float dt)
{
    Deck *cur = &A.deck[A.current];
    if (p->track != A.requested) {
        A.requested = p->track;
        if (!cur->active || cur->track != p->track) {
            Deck *old = cur;
            old->target = 0;
            old->rate = p->fade_seconds > 0.01f ? 1.0f / p->fade_seconds : 100.0f;
            A.current ^= 1;
            cur = &A.deck[A.current];
            DeckStart(cur, p->track, p->fade_seconds);
        }
    }
    if (cur->active) {
        cur->target = p->level;
        cur->rate = p->fade_seconds > 0.01f ? 1.0f / p->fade_seconds : 100.0f;
        for (int l = 0; l < AUDIO_BOSS_LAYERS; l++) cur->layer[l] = Approach(cur->layer[l], l == 0 && cur->track != MUSIC_BOSS ? 1 : p->layers[l], dt / 2.0f);
        /* faixa sem loop terminou (vitória): segue para o tema de abertura, suave */
        if (!A.music_loop[cur->track] && !A.paused && !IsMusicStreamPlaying(A.music[cur->track][0])) {
            if (cur->track == MUSIC_VICTORY) { cur->target = 0; A.current ^= 1; DeckStart(&A.deck[A.current], MUSIC_MENU, 4.0f); A.deck[A.current].target = 0.8f; }
            else cur->active = false;
        }
    }
    A.duck = Approach(A.duck, p->duck, dt / 0.6f);
    for (int i = 0; i < 2; i++) {
        Deck *d = &A.deck[i];
        if (!d->active) continue;
        d->gain = Approach(d->gain, d->target, dt * d->rate);
        if (d->gain <= 0.0001f && d->target <= 0 && d != &A.deck[A.current]) { DeckStop(d); continue; }
        for (int l = 0; l < AUDIO_BOSS_LAYERS; l++) {
            if (!A.music_valid[d->track][l]) continue;
            float layer = d->track == MUSIC_BOSS ? d->layer[l] : (l == 0 ? 1.0f : 0.0f);
            SetMusicVolume(A.music[d->track][l], Clamp01(d->gain * layer * A.duck * Volume(AUDIO_MUSIC)));
            if (!A.paused) UpdateMusicStream(A.music[d->track][l]);
        }
    }
    A.stats.current = cur->active ? cur->track : MUSIC_NONE;
    A.stats.music_gain = cur->active ? cur->gain * A.duck * Volume(AUDIO_MUSIC) : 0;
}

static void PauseMusic(bool pause)
{
    for (int i = 0; i < 2; i++) {
        Deck *d = &A.deck[i];
        if (!d->active) continue;
        for (int l = 0; l < AUDIO_BOSS_LAYERS; l++) {
            if (!A.music_valid[d->track][l]) continue;
            if (pause) PauseMusicStream(A.music[d->track][l]);
            else ResumeMusicStream(A.music[d->track][l]);
        }
    }
}

void AudioFrame(const GameState *game, int ui_focus, double seconds)
{
    AudioDirector *director = AudioDirectorShared();
    AudioDirectorUpdate(director, game, ui_focus, seconds);
    if (!A.enabled) return;
    float dt = isfinite(seconds) && seconds > 0 ? (float)fmin(seconds, 0.1) : 0;
    const MusicPlan *plan = &director->plan;
    /* efeitos de pausa/retomada tocam mesmo com o resto congelado */
    for (unsigned i = 0; i < director->cue_count; i++) {
        AudioCue c = director->cues[i].cue;
        if (!plan->paused || c == CUE_UI_PAUSE || c == CUE_UI_MOVE || c == CUE_UI_BACK || c == CUE_UI_VOLUME)
            AudioPlayCue(director->cues[i]);
    }
    if (plan->paused != A.paused) {
        A.paused = plan->paused;
        PauseMusic(A.paused);
        PauseEffects(A.paused);
    }
    MusicFrame(plan, A.paused ? 0 : dt);
    A.stats.music_paused = A.paused;
}

AudioStats AudioGetStats(void)
{
    AudioStats s = A.stats;
    s.device = A.device;
    s.enabled = A.enabled;
    snprintf(s.directory, sizeof(s.directory), "%s", A.dir);
    unsigned busy = 0;
    for (int c = 0; c < CUE_COUNT; c++)
        for (int f = 0; f < FILES_MAX; f++)
            if (A.sfx[c].valid[f])
                for (int v = 0; v < A.sfx[c].voices; v++) busy += IsSoundPlaying(A.sfx[c].voice[f][v]);
    s.voices_busy = busy;
    return s;
}

const char *AudioStatus(void) { return A.status[0] ? A.status : "Audio nao inicializado."; }
