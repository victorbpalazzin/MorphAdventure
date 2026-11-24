/* Diretor de áudio (src/audio/audio_cues.c): lógica pura, sem raylib e sem som. */
#include "audio/audio_cues.h"
#include "core/game.h"
#include "morph_check.h"
#include <math.h>
#include <string.h>

#define DT (1.0 / 60)
static AudioDirector d;
static GameState g;

static unsigned Count(AudioCue cue)
{
    unsigned n = 0;
    for (unsigned i = 0; i < d.cue_count; i++) n += d.cues[i].cue == cue;
    return n;
}
static void Frame(void) { AudioDirectorUpdate(&d, &g, -1, DT); }
static void Event(GameEventType type, int boost)
{
    AudioDirectorTap(&d, (AudioTap){TAP_GAME_EVENT, (int)type, boost, 0});
}
static void Fresh(GameMode mode)
{
    AudioDirectorReset(&d);
    memset(&g, 0, sizeof(g));
    g.mode = mode; g.run_id = 1; g.world_id = 1; g.lives = 3;
    g.minimum_distance = 1000;
    Frame(); /* primeiro quadro só registra */
    MORPH_CHECK(d.cue_count == 0);
}

/* Um evento real -> exatamente um efeito; condição contínua não repete. */
static void CheckOneCuePerEvent(void)
{
    Fresh(GAME_PLAY);
    for (int i = 0; i < 30; i++) { Frame(); MORPH_CHECK(d.cue_count == 0); } /* nada acontece sozinho */
    g.distance = 250; Event(EVENT_FRAGMENT_COLLECTED, 0); Frame();
    MORPH_CHECK(Count(CUE_COLLECT) == 1 && d.cue_count == 1);
    for (int i = 0; i < 30; i++) { Frame(); MORPH_CHECK(Count(CUE_COLLECT) == 0); }
    /* duas coletas no mesmo quadro (ímã): dois sons, um por coleta */
    g.distance = 500; Event(EVENT_FRAGMENT_COLLECTED, 0); Event(EVENT_FRAGMENT_COLLECTED, 0); Frame();
    MORPH_CHECK(Count(CUE_COLLECT) == 2);
    Event(EVENT_BOOST_COLLECTED, BOOST_SHIELD); Frame(); MORPH_CHECK(Count(CUE_BOOST_SHIELD) == 1);
    Event(EVENT_BOOST_COLLECTED, BOOST_MAGNET); Frame(); MORPH_CHECK(Count(CUE_BOOST_MAGNET) == 1);
    Event(EVENT_BOOST_COLLECTED, BOOST_SLOW); Frame(); MORPH_CHECK(Count(CUE_BOOST_SLOW) == 1);
    Event(EVENT_BOOST_COLLECTED, BOOST_DOUBLE); Frame(); MORPH_CHECK(Count(CUE_BOOST_DOUBLE) == 1);
    Event(EVENT_BOOST_COLLECTED, BOOST_PHASE); Frame(); MORPH_CHECK(Count(CUE_BOOST_PHASE) == 1);
    Event(EVENT_BOOST_COLLECTED, BOOST_LIFE); Frame(); MORPH_CHECK(Count(CUE_LIFE_UP) == 1);
    Event(EVENT_DAMAGE, 0); Frame(); MORPH_CHECK(Count(CUE_HIT) == 1);
    for (int i = 0; i < 10; i++) Frame();
    Event(EVENT_SHIELD_ABSORBED, 0); Frame(); MORPH_CHECK(Count(CUE_SHIELD_BREAK) == 1);
    Event(EVENT_PORTAL_CHARGED, 0); Frame(); MORPH_CHECK(Count(CUE_BATTERY_FULL) == 1);
    Event(EVENT_OBSTACLE_PASSED, 0); Frame(); MORPH_CHECK(d.cue_count == 0); /* pontuação não compete com a trilha */
    AudioDirectorTap(&d, (AudioTap){TAP_FLAP, 0, 0, 0}); Frame();
    MORPH_CHECK(Count(CUE_FLAP) == 1 && d.cues[0].variant >= 0 && d.cues[0].variant < 3);
    MORPH_CHECK(d.cues[0].pitch > 0.95f && d.cues[0].pitch < 1.05f);
}

/* Vários eventos idênticos no mesmo instante não empilham (intervalo mínimo). */
static void CheckCooldown(void)
{
    Fresh(GAME_PLAY);
    for (int i = 0; i < 5; i++) AudioDirectorTap(&d, (AudioTap){TAP_FLAP, 0, 0, 0});
    Frame(); MORPH_CHECK(Count(CUE_FLAP) == 1);
    AudioDirectorTap(&d, (AudioTap){TAP_FLAP, 0, 0, 0}); Frame(); MORPH_CHECK(Count(CUE_FLAP) == 0);
    for (int i = 0; i < 4; i++) Frame();
    AudioDirectorTap(&d, (AudioTap){TAP_FLAP, 0, 0, 0}); Frame(); MORPH_CHECK(Count(CUE_FLAP) == 1);
    for (int i = 0; i < 200; i++) AudioDirectorTap(&d, (AudioTap){TAP_FLAP, 0, 0, 0});
    MORPH_CHECK(d.tap_overflow && d.tap_count == AUDIO_MAX_TAPS);  /* fila limitada, sem estouro de memória */
    Frame(); MORPH_CHECK(!d.tap_overflow && d.tap_count == 0);
}

/* Coletas sobem numa escala pentatônica até a bateria encher. */
static void CheckCollectMelody(void)
{
    for (int w = 0; w < MORPH_WORLD_COUNT; w++) {
        unsigned req = GameEnergyRequired(w);
        float prev = 0;
        for (unsigned k = 1; k <= req; k++) {
            float p = AudioCollectPitch(k, req);
            MORPH_CHECK(p >= prev && p > 0.7f && p < 2.01f);
            prev = p;
        }
        MORPH_CHECK(fabsf(AudioCollectPitch(1, req) - powf(2, -5 / 12.0f)) < 1e-4f);
        MORPH_CHECK(fabsf(AudioCollectPitch(req, req) - 2.0f) < 1e-4f);
    }
}

static void CheckTransitionsAndMusic(void)
{
    Fresh(GAME_INTRO);
    MORPH_CHECK(d.plan.track == MUSIC_MENU && !d.plan.paused);
    g.mode = GAME_PLAY; Frame();
    MORPH_CHECK(Count(CUE_UI_START) == 1 && d.plan.track == MUSIC_AR && d.plan.level == 1);
    g.mode = GAME_PAUSED; g.resume_mode = GAME_PLAY; Frame();
    MORPH_CHECK(Count(CUE_UI_PAUSE) == 1 && d.plan.paused && d.plan.track == MUSIC_AR); /* mesma faixa, congelada */
    g.mode = GAME_PLAY; Frame(); MORPH_CHECK(Count(CUE_UI_RESUME) == 1 && !d.plan.paused);
    g.mode = GAME_PORTAL; Frame(); MORPH_CHECK(Count(CUE_PORTAL_OPEN) == 1 && d.plan.track == MUSIC_AR);
    Event(EVENT_PORTAL_ENTERED, 0); g.mode = GAME_TRANSITION; Frame();
    MORPH_CHECK(Count(CUE_PORTAL_ENTER) == 1 && d.plan.track == MUSIC_AR && d.plan.level == 0); /* fade-out */
    g.world = 1; g.world_id++; Frame();
    MORPH_CHECK(Count(CUE_WORLD_WHOOSH) == 1 && d.plan.track == MUSIC_AGUA && d.plan.level == 1); /* entra a nova */
    g.mode = GAME_PLAY; Frame(); MORPH_CHECK(d.cue_count == 0 && d.plan.track == MUSIC_AGUA);
    for (int w = 0; w < MORPH_WORLD_COUNT; w++) MORPH_CHECK((int)AudioWorldTrack(w) == (int)MUSIC_AR + w);
    g.mode = GAME_LOST; Frame(); MORPH_CHECK(Count(CUE_DEFEAT) == 1 && d.plan.track == MUSIC_DEFEAT);
    g.mode = GAME_PLAY; g.world = 0; g.run_id++; g.world_id++; Frame();
    MORPH_CHECK(Count(CUE_UI_CONFIRM) == 1 && Count(CUE_WORLD_WHOOSH) == 0 && d.plan.track == MUSIC_AR);
    g.mode = GAME_PAUSED; Frame(); g.mode = GAME_INTRO; Frame(); MORPH_CHECK(Count(CUE_UI_BACK) == 1 && d.plan.track == MUSIC_MENU);
    /* navegação: foco mudou na mesma tela -> um toque; mudança de tela não conta */
    AudioDirectorUpdate(&d, &g, 0, DT); AudioDirectorUpdate(&d, &g, 1, DT);
    MORPH_CHECK(Count(CUE_UI_MOVE) == 1);
    g.mode = GAME_WON; AudioDirectorUpdate(&d, &g, 0, DT);
    MORPH_CHECK(Count(CUE_UI_MOVE) == 0 && d.plan.track == MUSIC_VICTORY);
}

/* Batalha: aparição, ataques, energia, megarraio, fases (camadas) e derrota. */
static void CheckBoss(void)
{
    Fresh(GAME_TRANSITION);
    g.world = 5;
    g.mode = GAME_BOSS; g.boss.stage = BOSS_INTRO; g.boss.phase = 1; g.boss.segments = 3; g.boss.energy.required = 4;
    Frame();
    MORPH_CHECK(Count(CUE_BOSS_APPEAR) == 1 && Count(CUE_WORLD_WHOOSH) == 1 && d.plan.track == MUSIC_BOSS);
    MORPH_CHECK(d.plan.layers[0] == 1 && d.plan.layers[1] == 0 && d.plan.layers[2] == 0);
    g.boss.stage = BOSS_FIGHT; Frame();
    g.boss.warning = 65; Frame(); MORPH_CHECK(Count(CUE_BOSS_WARNING) == 1);
    Frame(); MORPH_CHECK(Count(CUE_BOSS_WARNING) == 0);                       /* aviso contínuo não repete */
    g.boss.warning = 0; g.boss.projectile_count = 1; Frame(); MORPH_CHECK(Count(CUE_BOSS_SHOT) == 1);
    Frame(); MORPH_CHECK(Count(CUE_BOSS_SHOT) == 0);
    for (unsigned k = 1; k <= 4; k++) {
        g.boss.energy.collected = k; Event(EVENT_FRAGMENT_COLLECTED, 0); Frame();
        MORPH_CHECK(Count(CUE_COMBAT_ENERGY) == 1 && Count(CUE_COLLECT) == 0);
        MORPH_CHECK(Count(CUE_BATTERY_READY) == (k == 4 ? 1u : 0u));
    }
    g.boss.beam_active = true; Frame(); MORPH_CHECK(Count(CUE_BEAM) == 1);
    for (int i = 0; i < 20; i++) { Frame(); MORPH_CHECK(Count(CUE_BEAM) == 0); }
    g.boss.segments = 2; Frame(); MORPH_CHECK(Count(CUE_BOSS_HURT) == 1);
    g.boss.beam_active = false; g.boss.phase = 2; g.boss.energy.collected = 0; Frame();
    MORPH_CHECK(Count(CUE_BOSS_PHASE) == 1 && d.plan.layers[1] == 1 && d.plan.layers[2] == 0);
    g.boss.segments = 1; g.boss.phase = 3; Frame();
    MORPH_CHECK(d.plan.layers[0] == 1 && d.plan.layers[1] == 1 && d.plan.layers[2] == 1);
    g.boss.segments = 0; g.boss.stage = BOSS_DYING; Frame();
    MORPH_CHECK(Count(CUE_BOSS_DEFEAT) == 1 && d.plan.track == MUSIC_BOSS && d.plan.level == 0); /* encerramento */
    g.boss.stage = BOSS_DONE; g.mode = GAME_WON; Frame(); MORPH_CHECK(d.plan.track == MUSIC_VICTORY);
    /* nova aventura: o estado antigo do boss não gera efeitos fantasmas */
    memset(&g.boss, 0, sizeof(g.boss)); g.mode = GAME_PLAY; g.world = 0; g.run_id++; Frame();
    MORPH_CHECK(Count(CUE_BOSS_HURT) == 0 && Count(CUE_BOSS_SHOT) == 0 && d.plan.track == MUSIC_AR);
}

static void CheckHazardsAndEnvironment(void)
{
    Fresh(GAME_PLAY);
    AudioDirectorTap(&d, (AudioTap){TAP_HAZARD_SPAWN, 3, 0, 0.6f}); Frame();
    MORPH_CHECK(Count(CUE_HAZARD_WARNING) == 1 && d.cues[0].param == 3 && d.cues[0].pan > 0.5f);
    AudioDirectorTap(&d, (AudioTap){TAP_HAZARD_LAUNCH, 3, 0, 0.6f}); Frame();
    MORPH_CHECK(Count(CUE_HAZARD) == 1 && d.cues[0].param == 3);
    AudioDirectorTap(&d, (AudioTap){TAP_ENV_WARNING, 4, 0, 0}); Frame(); MORPH_CHECK(Count(CUE_ENV_WARNING) == 1);
    AudioDirectorTap(&d, (AudioTap){TAP_ENV_ACTIVE, 4, 0, 0}); Frame();
    MORPH_CHECK(Count(CUE_ENV_EVENT) == 1 && d.cues[0].param == 4);
    MORPH_CHECK(strcmp(AudioCueName(CUE_BEAM), "beam") == 0 && strcmp(AudioCueName((AudioCue)999), "?") == 0);
}

int main(void)
{
    CheckOneCuePerEvent();
    CheckCooldown();
    CheckCollectMelody();
    CheckTransitionsAndMusic();
    CheckBoss();
    CheckHazardsAndEnvironment();
    puts("PASS audio-cues: um efeito por evento, intervalos minimos, melodia da coleta, plano musical por tela, "
         "camadas do boss por fase, megarraio e derrota, perigos e ambiente");
    return 0;
}
