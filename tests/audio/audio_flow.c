/* Jogo completo (módulos reais + coordenador real + composition.c) e o diretor de
 * áudio: cada evento real produz exatamente o efeito esperado, e o plano musical
 * acompanha menu, seis mundos, portais, batalha em fases, vitória e derrota.
 * Se houver dispositivo de som, o áudio também é tocado de verdade. */
#include "raylib.h"
#include "core/game.h"
#include "boss/boss.h"
#include "audio/audio.h"
#include "items/items_internal.h"
#include "obstacles/obstacles.h"
#include "player/player.h"
#include "ui/ui.h"
#include "ui/ui_internal.h"
#include "morph_check.h"
#include <string.h>

GameModules MorphCreateModules(void);
static Game game;
static unsigned counts[CUE_COUNT];
static MusicTrack tracks_seen[64];
static unsigned track_count;
static bool sound;
static bool realtime;  /* --tempo-real: 60 quadros/s, para gravar a mixagem final do jogo */

static void Record(void)
{
    AudioFrame(&game.state, UiInspect().focus, 1.0 / 60);  /* atualiza o diretor (e toca, se houver som) */
    AudioDirector *d = AudioDirectorShared();
    for (unsigned i = 0; i < d->cue_count; i++) counts[d->cues[i].cue]++;
    if (d->plan.track != MUSIC_NONE && (track_count == 0 || tracks_seen[track_count - 1] != d->plan.track) && track_count < 64)
        tracks_seen[track_count++] = d->plan.track;
}
static void Advance(GameInput in)
{
    GameAdvance(&game, 1.0 / 60, in); MORPH_CHECK(game.state.mode != GAME_ERROR); Record();
    if (realtime) WaitTime(1.0 / 60);
}
static void Tick(void) { GameInput in = {.focused = true}; Advance(in); }
static void Command(GameCommand c) { GameInput in = {.focused = true, .command = c}; GameAdvance(&game, 0, in); Record(); }
static PlayerView Player(void) { return game.modules.player.view(game.modules.player.life.context); }
static void Fly(void)
{
    PlayerView p = Player();
    GameInput in = {.focused = true, .impulse_pressed = p.position.y > 270 && p.vertical_velocity > 0};
    Advance(in);
}
static unsigned Delta(unsigned before[CUE_COUNT], AudioCue c) { return counts[c] - before[c]; }
static void Snapshot(unsigned out[CUE_COUNT]) { memcpy(out, counts, sizeof(counts)); }
static void SetEnergy(unsigned count)
{
    ItemsState *s = game.modules.items.life.context; s->collected = count;
    for (unsigned i = 0; i < s->required; i++) s->collected_ids[i] = i < count;
}

static void Gameplay(void)
{
    unsigned b[CUE_COUNT];
    MORPH_CHECK(game.state.mode == GAME_INTRO && AudioDirectorShared()->plan.track == MUSIC_MENU);
    Snapshot(b); Command(COMMAND_START);
    MORPH_CHECK(Delta(b, CUE_UI_START) == 1 && AudioDirectorShared()->plan.track == MUSIC_AR);
    /* batida de asa: um som por impulso aplicado */
    Snapshot(b);
    for (int i = 0; i < 5; i++) { GameInput in = {.focused = true, .impulse_pressed = true}; Advance(in); for (int k = 0; k < 10; k++) Tick(); }
    MORPH_CHECK(Delta(b, CUE_FLAP) == 5);
    /* coleta real: fragmento sobre o jogador -> um som e 35 pontos */
    ItemsState *items = game.modules.items.life.context;
    Snapshot(b); uint64_t score = game.state.score;
    items->fragment_count = 1; items->fragments[0] = (ItemFragmentView){0, Player().position, 23, 0, 0};
    Tick();
    MORPH_CHECK(Delta(b, CUE_COLLECT) == 1 && game.state.score == score + 35 && items->collected == 1);
    for (int i = 0; i < 30; i++) Tick();
    MORPH_CHECK(Delta(b, CUE_COLLECT) == 1);                         /* nada de repetição por quadro */
    /* boosts reais */
    static const struct { BoostType type; AudioCue cue; } boosts[] = {
        {BOOST_SHIELD, CUE_BOOST_SHIELD}, {BOOST_MAGNET, CUE_BOOST_MAGNET}, {BOOST_SLOW, CUE_BOOST_SLOW},
        {BOOST_DOUBLE, CUE_BOOST_DOUBLE}, {BOOST_PHASE, CUE_BOOST_PHASE}};
    for (size_t i = 0; i < sizeof(boosts) / sizeof(*boosts); i++) {
        Snapshot(b);
        items->power_count = 1; items->powers[0] = (ItemPowerView){100 + (unsigned)i, boosts[i].type, Player().position, 21};
        Tick();
        MORPH_CHECK(Delta(b, boosts[i].cue) == 1);
    }
    /* escudo absorve o primeiro impacto; depois dano real perde vida */
    items->boost_count = 0;
    items->power_count = 1; items->powers[0] = (ItemPowerView){200, BOOST_SHIELD, Player().position, 21}; Tick();
    Snapshot(b); int lives = game.state.lives;
    for (int i = 0; i < 400 && game.state.lives == lives; i++) Tick();  /* cai até o chão */
    MORPH_CHECK(Delta(b, CUE_SHIELD_BREAK) == 1 && Delta(b, CUE_HIT) == 1 && game.state.lives == lives - 1);
    /* pausa e retomada; painel de som (desenho + clique nos botões) */
    Snapshot(b); Command(COMMAND_TOGGLE_PAUSE); MORPH_CHECK(AudioDirectorShared()->plan.paused);
    AudioSettings set = AudioGetSettings();
    UiSetAudioView((UiAudioView){true, {set.volume[0], set.volume[1], set.volume[2]}, set.muted, 1});
    UiAudioRects rects = UiAudioLayout();
    GameInput click = {.focused = true, .pointer_pressed = true, .pointer_inside = true,
                       .pointer = {rects.plus[1].x + 5, rects.plus[1].y + 5}};
    GameAdvance(&game, 0, click);
    UiAudioRequest req = UiTakeAudioRequest();
    MORPH_CHECK(game.state.mode == GAME_PAUSED && req.channel == 1 && req.delta == 1 && !req.toggle_mute);
    click.pointer = (GameVec2){rects.mute.x + 5, rects.mute.y + 5}; GameAdvance(&game, 0, click);
    MORPH_CHECK(UiTakeAudioRequest().toggle_mute && game.state.mode == GAME_PAUSED);   /* clique no painel não retoma */
    MORPH_CHECK(UiTakeAudioRequest().channel == -1);                                    /* pedido consumido uma vez */
    RenderTexture2D canvas = LoadRenderTexture(960, 540);
    BeginTextureMode(canvas); ClearBackground(BLACK); GameDraw(&game); EndTextureMode();
    Image img = LoadImageFromTexture(canvas.texture); ImageFlipVertical(&img);
    MORPH_CHECK(ExportImage(img, "pausa-som.png")); UnloadImage(img); UnloadRenderTexture(canvas);
    Command(COMMAND_RESUME); MORPH_CHECK(Delta(b, CUE_UI_PAUSE) == 1 && Delta(b, CUE_UI_RESUME) == 1 && !AudioDirectorShared()->plan.paused);
}

/* Perigos e eventos ambientais reais: aviso e lançamento uma vez por perigo. */
static void Hazards(void)
{
    unsigned b[CUE_COUNT];
    Snapshot(b);
    game.state.lives = 99;
    uint32_t seen[64]; unsigned seen_count = 0, launched = 0, fresh = 0; /* fresh: nasceram nesta janela */
    for (int i = 0; i < 60 * 25; i++) {
        Fly();
        ObstaclesSnapshot o = ObstaclesInspect();
        for (unsigned h = 0; h < o.hazard_count; h++) {
            /* ids recomeçam a cada mundo (o piloto pode atravessar um portal aqui) */
            uint32_t key = (game.state.world_id << 16) ^ o.hazards[h].id;
            bool known = false;
            for (unsigned k = 0; k < seen_count; k++) known = known || seen[k] == key;
            if (!known && seen_count < 64) { seen[seen_count++] = key; fresh += o.hazards[h].age < 3; }
        }
    }
    launched = Delta(b, CUE_HAZARD);
    printf("perigos: %u vistos (%u nasceram aqui), %u avisos, %u lancamentos; ambiente: %u avisos, %u eventos\n", seen_count, fresh,
           Delta(b, CUE_HAZARD_WARNING), launched, Delta(b, CUE_ENV_WARNING), Delta(b, CUE_ENV_EVENT));
    /* um aviso por perigo que nasceu aqui; um lançamento por perigo (os já existentes podem lançar aqui) */
    MORPH_CHECK(fresh >= 2 && Delta(b, CUE_HAZARD_WARNING) == fresh && launched >= fresh - 1 && launched <= seen_count);
    MORPH_CHECK(Delta(b, CUE_ENV_WARNING) >= 2 && Delta(b, CUE_ENV_EVENT) >= 2);
    MORPH_CHECK(Delta(b, CUE_ENV_EVENT) <= Delta(b, CUE_ENV_WARNING));
    game.state.lives = 3;
}

/* Seis portais, trilhas na ordem certa, batalha em fases e vitória. */
static void Campaign(void)
{
    unsigned b[CUE_COUNT];
    Command(COMMAND_RESTART); Command(COMMAND_START);
    track_count = 0; Record();
    Snapshot(b);
    for (int world = 0; world < 6; world++) {
        MORPH_CHECK(game.state.world == world && AudioDirectorShared()->plan.track == AudioWorldTrack(world));
        SetEnergy(GameEnergyRequired(world));
        for (int i = 0; i < 900 && game.state.mode == GAME_PLAY; i++) Fly();
        MORPH_CHECK(game.state.mode == GAME_PORTAL);
        for (int i = 0; i < 400 && game.state.mode != GAME_PLAY && game.state.mode != GAME_BOSS; i++) Tick();
    }
    MORPH_CHECK(Delta(b, CUE_PORTAL_OPEN) == 6 && Delta(b, CUE_PORTAL_ENTER) == 6 && Delta(b, CUE_WORLD_WHOOSH) == 6);
    MORPH_CHECK(Delta(b, CUE_BOSS_APPEAR) == 1 && game.state.mode == GAME_BOSS);
    static const MusicTrack order[] = {MUSIC_AR, MUSIC_AGUA, MUSIC_FLORESTA, MUSIC_GELO, MUSIC_TERRA, MUSIC_FOGO, MUSIC_BOSS};
    MORPH_CHECK(track_count == 7);
    for (int i = 0; i < 7; i++) MORPH_CHECK(tracks_seen[i] == order[i]);
    /* batalha */
    Snapshot(b);
    game.state.lives = 99;
    for (int i = 0; i < 180; i++) Tick();
    MORPH_CHECK(game.state.boss.stage == BOSS_FIGHT && AudioDirectorShared()->plan.layers[1] == 0);
    for (int i = 0; i < 600; i++) Fly();
    MORPH_CHECK(Delta(b, CUE_BOSS_WARNING) >= 1 && Delta(b, CUE_BOSS_SHOT) >= 1);
    for (unsigned shot = 0; shot < 3; shot++) {
        for (unsigned k = 0; k < game.state.boss.energy.required; k++) {
            game.state.boss.energy.present = true; game.state.boss.energy.position = Player().position;
            for (int i = 0; i < 70 && game.state.boss.energy.collected <= k; i++) Fly();
        }
        MORPH_CHECK(game.state.boss.energy.collected == game.state.boss.energy.required);
        GameInput fire = {.focused = true, .attack_pressed = true}; Advance(fire);
        MORPH_CHECK(game.state.boss.beam_active);
        for (int i = 0; i < 90; i++) Fly();
        if (shot == 0) MORPH_CHECK(AudioDirectorShared()->plan.layers[1] == 1 && AudioDirectorShared()->plan.layers[2] == 0);
        if (shot == 1) MORPH_CHECK(AudioDirectorShared()->plan.layers[2] == 1);
    }
    MORPH_CHECK(Delta(b, CUE_COMBAT_ENERGY) == 12 && Delta(b, CUE_BATTERY_READY) == 3 && Delta(b, CUE_BEAM) == 3);
    MORPH_CHECK(Delta(b, CUE_BOSS_HURT) == 3 && Delta(b, CUE_BOSS_PHASE) == 2 && Delta(b, CUE_BOSS_DEFEAT) == 1);
    MORPH_CHECK(AudioDirectorShared()->plan.level == 0);           /* trilha da batalha se encerra */
    for (int i = 0; i < 260 && game.state.mode == GAME_BOSS; i++) Tick();
    MORPH_CHECK(game.state.mode == GAME_WON && AudioDirectorShared()->plan.track == MUSIC_VICTORY);
    /* derrota */
    Snapshot(b);
    Command(COMMAND_RETRY);
    game.state.lives = 1;
    for (int i = 0; i < 600 && game.state.mode == GAME_PLAY; i++) Tick();
    MORPH_CHECK(game.state.mode == GAME_LOST && Delta(b, CUE_DEFEAT) == 1 && AudioDirectorShared()->plan.track == MUSIC_DEFEAT);
    Snapshot(b); Command(COMMAND_RESTART);
    MORPH_CHECK(Delta(b, CUE_UI_BACK) == 1 && AudioDirectorShared()->plan.track == MUSIC_MENU);
}

int main(int argc, char **argv)
{
    MORPH_CHECK(argc == 2 || argc == 3);
    realtime = argc == 3 && strcmp(argv[2], "--tempo-real") == 0;
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(960, 540, "Morph Adventure - audio"); MORPH_CHECK(IsWindowReady());
    ItemsSetSeed(5);
    sound = AudioInitFrom(argv[1], "audio-flow-prefs.cfg");
    GameModules modules = MorphCreateModules();
    MORPH_CHECK(GameInit(&game, &modules) && game.ready);
    Record();
    Gameplay();
    Hazards();
    Campaign();
    AudioStats st = AudioGetStats();
    unsigned total = 0;
    for (int c = 0; c < CUE_COUNT; c++) total += counts[c];
    printf("som real: %s | efeitos decididos: %u | efeitos tocados: %u\n", sound ? "sim" : "nao (modo silencioso)", total, st.cues_played);
    if (sound) MORPH_CHECK(st.cues_played > 0 && st.cues_played <= total);
    GameShutdown(&game);
    AudioShutdown();
    remove("audio-flow-prefs.cfg");
    CloseWindow();
    puts("PASS audio-flow: modulos reais, um efeito por evento (asa, coleta, boosts, escudo, dano, perigos, ambiente), "
         "seis portais com trilhas em ordem, batalha em fases, megarraio, vitoria e derrota");
    return 0;
}
