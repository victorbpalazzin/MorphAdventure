#include "audio_cues.h"
#include "core/game.h"
#include <math.h>
#include <string.h>

/* Intervalo mínimo entre repetições do mesmo efeito (segundos): evita
 * empilhamento quando vários eventos idênticos chegam quase juntos. Coletas
 * NÃO têm intervalo: cada coleta precisa do seu som (a polifonia absorve). */
static const float cooldowns[CUE_COUNT] = {
    [CUE_FLAP] = 0.05f, [CUE_HIT] = 0.12f,
    [CUE_HAZARD] = 0.15f, [CUE_HAZARD_WARNING] = 0.25f, [CUE_ENV_EVENT] = 1.0f, [CUE_ENV_WARNING] = 1.0f,
    [CUE_BOSS_SHOT] = 0.07f, [CUE_UI_MOVE] = 0.03f, [CUE_UI_VOLUME] = 0.03f,
};

static AudioDirector shared;
AudioDirector *AudioDirectorShared(void) { return &shared; }

void AudioDirectorReset(AudioDirector *d)
{
    memset(d, 0, sizeof(*d));
    d->seed = 0x5a17u;
    d->ui_focus = -1;
    d->plan.level = 1;
    d->plan.duck = 1;
    d->plan.fade_seconds = 1.5f;
}

void AudioDirectorTap(AudioDirector *d, AudioTap tap)
{
    if (d->tap_count >= AUDIO_MAX_TAPS) { d->tap_overflow = true; return; }
    d->taps[d->tap_count++] = tap;
}

static float Random(AudioDirector *d)
{
    d->seed = d->seed * 1664525u + 1013904223u;
    return (float)((d->seed >> 8) / 16777216.0);
}

/* Pequena variação de afinação (±spread) para efeitos repetitivos. */
static float Vary(AudioDirector *d, float spread) { return 1.0f + (Random(d) * 2 - 1) * spread; }

static void Emit(AudioDirector *d, AudioCue cue, int param, float pitch, float gain, float pan)
{
    if (cue <= CUE_NONE || cue >= CUE_COUNT || d->cue_count >= AUDIO_MAX_CUES) return;
    if (d->cooldown[cue] > 0) return;
    d->cooldown[cue] = cooldowns[cue];
    AudioCueEvent e = {cue, param, 0, pitch, gain, pan};
    if (cue == CUE_FLAP) e.variant = (int)(Random(d) * 3) % 3;
    d->cues[d->cue_count++] = e;
}

float AudioCollectPitch(unsigned collected, unsigned required)
{
    /* Pentatônica maior a partir de uma quarta abaixo do original (Ré6 -> Lá5),
     * subindo até perto de uma oitava acima ao completar a bateria. */
    static const int scale[] = {-5, -3, 0, 2, 4, 7, 9, 12};
    if (!required || !collected) return 1.0f;
    unsigned k = collected > required ? required : collected;
    float step = required > 1 ? (float)(k - 1) * 7.0f / (float)(required - 1) : 7.0f;
    int index = (int)floorf(step + 0.5f);
    if (index < 0) index = 0;
    if (index > 7) index = 7;
    return powf(2.0f, (float)scale[index] / 12.0f);
}

MusicTrack AudioWorldTrack(int world)
{
    return world >= 0 && world < MORPH_WORLD_COUNT ? (MusicTrack)(MUSIC_AR + world) : MUSIC_NONE;
}

static unsigned Collected(const GameState *g)
{
    unsigned required = GameEnergyRequired(g->world);
    if (!required || g->minimum_distance <= 0) return 0;
    return (unsigned)floorf(g->distance / g->minimum_distance * (float)required + 0.5f);
}

static void HandleTap(AudioDirector *d, const GameState *g, AudioTap t)
{
    switch (t.kind) {
    case TAP_FLAP: Emit(d, CUE_FLAP, 0, Vary(d, 0.04f), 0.55f + 0.1f * Random(d), (Random(d) - 0.5f) * 0.15f); break;
    case TAP_HAZARD_SPAWN: Emit(d, CUE_HAZARD_WARNING, t.param, Vary(d, 0.03f), 0.5f, t.pan); break;
    case TAP_HAZARD_LAUNCH: Emit(d, CUE_HAZARD, t.param, Vary(d, 0.05f), 0.75f, t.pan); break;
    case TAP_ENV_WARNING: Emit(d, CUE_ENV_WARNING, t.param, 1.0f, 0.6f, 0); break;
    case TAP_ENV_ACTIVE: Emit(d, CUE_ENV_EVENT, t.param, 1.0f, 0.8f, 0); break;
    case TAP_GAME_EVENT:
        switch ((GameEventType)t.param) {
        case EVENT_FRAGMENT_COLLECTED:
            if (g->boss.stage != BOSS_DORMANT) {
                static const float steps[] = {1.0f, 1.2599f, 1.4983f, 2.0f}; /* tônica, terça, quinta, oitava */
                unsigned k = g->boss.energy.collected ? g->boss.energy.collected - 1 : 0;
                Emit(d, CUE_COMBAT_ENERGY, 0, steps[k % 4], 0.85f, 0);
            } else {
                Emit(d, CUE_COLLECT, 0, AudioCollectPitch(Collected(g), GameEnergyRequired(g->world)), 0.8f, 0);
            }
            break;
        case EVENT_BOOST_COLLECTED: {
            static const AudioCue boosts[] = {CUE_BOOST_SHIELD, CUE_BOOST_MAGNET, CUE_BOOST_SLOW,
                                              CUE_BOOST_DOUBLE, CUE_LIFE_UP, CUE_BOOST_PHASE};
            if (t.boost >= 0 && t.boost < (int)(sizeof(boosts) / sizeof(*boosts))) Emit(d, boosts[t.boost], 0, 1.0f, 0.85f, 0);
            break;
        }
        case EVENT_DAMAGE: Emit(d, CUE_HIT, 0, Vary(d, 0.02f), 0.9f, 0); break;
        case EVENT_SHIELD_ABSORBED: Emit(d, CUE_SHIELD_BREAK, 0, 1.0f, 0.85f, 0); break;
        case EVENT_PORTAL_CHARGED: Emit(d, CUE_BATTERY_FULL, 0, 1.0f, 0.85f, 0); break;
        case EVENT_PORTAL_ENTERED: Emit(d, CUE_PORTAL_ENTER, 0, 1.0f, 0.9f, 0); break;
        default: break; /* obstáculos ultrapassados: silenciosos (pontuação não compete com a trilha) */
        }
        break;
    }
}

static bool Menu(GameMode m) { return m == GAME_INTRO || m == GAME_PAUSED || m == GAME_LOST || m == GAME_WON; }

static void Transitions(AudioDirector *d, const GameState *g, int ui_focus)
{
    GameMode before = d->mode, now = g->mode;
    if (now != before) {
        if (now == GAME_PAUSED) Emit(d, CUE_UI_PAUSE, 0, 1, 0.7f, 0);
        else if (before == GAME_PAUSED && now != GAME_INTRO) Emit(d, CUE_UI_RESUME, 0, 1, 0.7f, 0);
        if (now == GAME_PLAY && before == GAME_INTRO) Emit(d, CUE_UI_START, 0, 1, 0.85f, 0);
        else if (now == GAME_PLAY && (before == GAME_LOST || before == GAME_WON)) Emit(d, CUE_UI_CONFIRM, 0, 1, 0.8f, 0);
        if (now == GAME_INTRO && (before == GAME_PAUSED || before == GAME_LOST || before == GAME_WON)) Emit(d, CUE_UI_BACK, 0, 1, 0.75f, 0);
        if (now == GAME_LOST) Emit(d, CUE_DEFEAT, 0, 1, 0.9f, 0);
        if (now == GAME_PORTAL && before == GAME_PLAY) Emit(d, CUE_PORTAL_OPEN, 0, 1, 0.85f, 0);
        if (now == GAME_BOSS && before == GAME_TRANSITION) {
            Emit(d, CUE_WORLD_WHOOSH, 0, 1, 0.8f, 0);
            Emit(d, CUE_BOSS_APPEAR, 0, 1, 0.95f, 0);
        }
    }
    if (g->world_id != d->world_id && g->run_id == d->run_id && now == GAME_TRANSITION) Emit(d, CUE_WORLD_WHOOSH, 0, 1, 0.8f, 0);
    if (Menu(now) && now == before && ui_focus >= 0 && d->ui_focus >= 0 && ui_focus != d->ui_focus) Emit(d, CUE_UI_MOVE, 0, 1, 0.6f, 0);

    const BossState *b = &g->boss;
    if (b->stage != BOSS_DORMANT && d->boss_stage != BOSS_DORMANT && g->run_id == d->run_id) {
        if (b->warning > 0 && !d->boss_warning) Emit(d, CUE_BOSS_WARNING, 0, 1, 0.7f, 0);
        if (b->projectile_count > d->boss_projectiles) Emit(d, CUE_BOSS_SHOT, 0, Vary(d, 0.05f), 0.75f, 0.4f);
        if (b->energy.collected >= b->energy.required && d->combat_collected < b->energy.required && b->energy.required)
            Emit(d, CUE_BATTERY_READY, 0, 1, 0.85f, 0);
        if (b->beam_active && !d->beam_active) Emit(d, CUE_BEAM, 0, 1, 1.0f, 0);
        if (b->segments < d->boss_segments) Emit(d, CUE_BOSS_HURT, 0, 1, 0.9f, 0.3f);
        if (b->phase > d->boss_phase && b->stage == BOSS_FIGHT) Emit(d, CUE_BOSS_PHASE, 0, 1, 0.9f, 0);
        if (b->stage == BOSS_DYING && d->boss_stage != BOSS_DYING) Emit(d, CUE_BOSS_DEFEAT, 0, 1, 1.0f, 0);
    }
}

static void Plan(AudioDirector *d, const GameState *g)
{
    MusicPlan *p = &d->plan;
    p->paused = g->mode == GAME_PAUSED;
    p->duck = 1;
    GameMode mode = g->mode == GAME_PAUSED ? g->resume_mode : g->mode;
    for (int i = 0; i < AUDIO_BOSS_LAYERS; i++) p->layers[i] = 0;
    switch (mode) {
    case GAME_INTRO: p->track = MUSIC_MENU; p->level = 1; p->fade_seconds = 1.5f; break;
    case GAME_PLAY: case GAME_PORTAL:
        p->track = AudioWorldTrack(g->world); p->level = 1; p->fade_seconds = 2.5f;
        d->last_world_track = p->track;
        break;
    case GAME_TRANSITION:
        /* Portal atravessado: a faixa antiga some; após o escurecimento entra a do novo mundo. */
        p->track = AudioWorldTrack(g->world);
        p->level = p->track == d->last_world_track ? 0 : 1;
        p->fade_seconds = p->level > 0 ? 2.5f : 1.2f;
        break;
    case GAME_BOSS:
        p->track = MUSIC_BOSS;
        p->layers[0] = 1;
        p->layers[1] = g->boss.phase >= 2 ? 1 : 0;
        p->layers[2] = g->boss.phase >= 3 ? 1 : 0;
        p->level = g->boss.stage == BOSS_DYING || g->boss.stage == BOSS_DONE ? 0 : 1;
        p->fade_seconds = p->level > 0 ? 2.0f : 1.0f;  /* encerramento dramático junto ao efeito de derrota */
        break;
    case GAME_WON: p->track = MUSIC_VICTORY; p->level = 1; p->fade_seconds = 0.6f; break;
    case GAME_LOST: p->track = MUSIC_DEFEAT; p->level = 1; p->fade_seconds = 0.8f; break;
    default: p->track = MUSIC_NONE; p->level = 0; p->fade_seconds = 1.0f; break;
    }
}

void AudioDirectorUpdate(AudioDirector *d, const GameState *g, int ui_focus, double seconds)
{
    d->cue_count = 0;
    float dt = isfinite(seconds) && seconds > 0 ? (float)fmin(seconds, 0.25) : 0;
    for (int i = 0; i < CUE_COUNT; i++) d->cooldown[i] = fmaxf(0, d->cooldown[i] - dt);
    if (!d->started) {
        /* Primeiro quadro: só registra o estado, não dispara transições fictícias. */
        d->started = true;
        d->mode = g->mode; d->world_id = g->world_id; d->run_id = g->run_id;
    }
    if (g->run_id != d->run_id) {  /* nova aventura: nada do estado antigo do boss vale */
        d->boss_stage = BOSS_DORMANT;
        d->boss_projectiles = d->boss_segments = d->boss_phase = d->combat_collected = 0;
        d->boss_warning = d->beam_active = false;
    }
    for (unsigned i = 0; i < d->tap_count; i++) HandleTap(d, g, d->taps[i]);
    d->tap_count = 0;
    d->tap_overflow = false;
    Transitions(d, g, ui_focus);
    Plan(d, g);
    d->mode = g->mode; d->world = g->world; d->lives = g->lives; d->ui_focus = ui_focus;
    d->world_id = g->world_id; d->run_id = g->run_id;
    d->boss_stage = g->boss.stage; d->boss_projectiles = g->boss.projectile_count;
    d->boss_segments = g->boss.segments; d->boss_phase = g->boss.phase;
    d->combat_collected = g->boss.energy.collected;
    d->boss_warning = g->boss.warning > 0; d->beam_active = g->boss.beam_active;
}

const char *AudioCueName(AudioCue cue)
{
    static const char *names[CUE_COUNT] = {
        "none", "flap", "hit", "defeat", "life_up", "collect", "battery_full",
        "boost_shield", "boost_magnet", "boost_slow", "boost_double", "boost_phase", "shield_break",
        "portal_open", "portal_enter", "world_whoosh", "env_warning", "env", "hazard_warning", "hazard",
        "boss_appear", "boss_warning", "boss_shot", "combat_energy", "battery_ready", "beam", "boss_hurt",
        "boss_phase", "boss_defeat", "ui_move", "ui_confirm", "ui_back", "ui_pause", "ui_resume", "ui_start", "ui_volume"};
    return cue >= 0 && cue < CUE_COUNT ? names[cue] : "?";
}
