"""Efeitos sonoros originais de Morph Adventure (camadas sintetizadas).

Cada função retorna (áudio, pico_dBFS). Pico mais baixo = efeito frequente/discreto.
A afinação segue a paleta do jogo (Ré maior / pentatônica), então coletas e
interface "conversam" com a trilha em vez de soarem como bipes isolados.
"""
import numpy as np
import morphsynth as ms

SR = ms.SR
N = ms.note
F = lambda name: float(ms.hz(N(name)))


def t_axis(seconds):
    n = ms.samples(seconds)
    return n, np.arange(n) / SR


def pad_to(x, seconds):
    n = ms.samples(seconds)
    if x.ndim == 1:
        out = np.zeros(n)
    else:
        out = np.zeros((n, x.shape[1]))
    out[: min(n, len(x))] += x[:n]
    return out


def place(buf, x, at):
    """Soma x em buf (mono/estéreo) no tempo `at` em segundos."""
    i = ms.samples(at) if at > 0 else 0
    x = ms.tail_fade(x)
    if buf.ndim == 2 and x.ndim == 1:
        x = ms.stereo(x)
    if buf.ndim == 1 and x.ndim == 2:
        x = x.mean(axis=1)
    e = min(len(buf), i + len(x))
    buf[i:e] += x[: e - i]
    return buf


def space(x, seconds=1.2, amount=0.25, damp=7000, predelay=0.01):
    # fade curto no fim do buffer seco: nenhuma camada termina com corte abrupto (clique)
    x = ms.fade(np.asarray(x, dtype=float), 0, min(0.12, len(x) / ms.SR * 0.25))
    wet = ms.reverb(x, seconds, predelay, damp)
    dry = ms.stereo(x)
    out = np.zeros((max(len(dry), len(wet)), 2))
    out[: len(dry)] += dry
    out[: len(wet)] += wet * amount
    return out


def trim(x, seconds):
    return ms.fade(x[: ms.samples(seconds)], 0, min(0.15, seconds * 0.3))


def chirp(f0, f1, seconds, curve="exp"):
    n, t = t_axis(seconds)
    f = f0 * (f1 / f0) ** (t / seconds) if curve == "exp" else f0 + (f1 - f0) * t / seconds
    return np.sin(2 * np.pi * np.cumsum(f) / SR)


def sparkle(seconds, density=40, low=5000, high=11000, seed=1):
    rng = np.random.default_rng(seed)
    out = np.zeros(ms.samples(seconds))
    for _ in range(int(seconds * density)):
        f = rng.uniform(low, high)
        g = ms.inst_glass(f, 0.05, 0.25 * rng.random() + 0.05)[: ms.samples(0.25)]
        at = rng.random() * seconds * 0.8
        place(out, g * np.exp(-at * 3), at)
    return out

# ================================================================ personagem

def flap(seed):
    """Batida de asa: sopro curto com leve "flutter", muito discreto (toca muitas vezes)."""
    rng = np.random.default_rng(seed)
    n, t = t_axis(0.2)
    body = ms.sweep(ms.noise(n, "pink"), "bp", 500 * (2.6 ** (t / 0.2)) * rng.uniform(0.9, 1.1), q=1.3)
    env = (t / 0.025).clip(0, 1) * np.exp(-t / 0.05)
    flutter = 1 - 0.35 * (np.sin(2 * np.pi * rng.uniform(55, 70) * t) > 0)
    thump = np.sin(2 * np.pi * np.cumsum(140 * np.exp(-t / 0.04) + 70) / SR) * np.exp(-t / 0.03) * 0.5
    return (body * env * flutter + thump) * 0.8, -12.0


def hit():
    """Dano: impacto grave + mordida áspera + tom que cai + brilho de proteção."""
    n, t = t_axis(0.9)
    boom = np.sin(2 * np.pi * np.cumsum(55 + 140 * np.exp(-t / 0.05)) / SR) * np.exp(-t / 0.18)
    crunch = ms.saturate(ms.biquad(ms.noise(n), "bp", 1800, 0.8) * np.exp(-t / 0.03), 3) * 0.5
    fall = (ms.saw(F("F#4") * 2 ** (-t / 0.25), n) * 0.5 + ms.saw(F("G4") * 2 ** (-t / 0.25), n) * 0.5)
    fall = ms.biquad(fall, "lp", 2200) * np.exp(-t / 0.12) * 0.35
    shimmer = pad_to(sparkle(0.6, 25, 4000, 9000, 3) * 0.6, 0.9)
    shimmer = np.roll(shimmer, ms.samples(0.18))
    return space(boom + crunch + fall + shimmer * 0.5, 0.8, 0.18), -3.0


def defeat():
    """Queda/derrota: três tons descendentes suaves, impacto abafado e respiração final."""
    out = np.zeros((ms.samples(2.0), 2))
    for k, name in enumerate(("A4", "F4", "D4")):
        place(out, ms.stereo(ms.inst_ep(F(name), 0.3, 0.6, bright=0.5), (k - 1) * 0.3), k * 0.18)
    n, t = t_axis(1.2)
    thud = np.sin(2 * np.pi * np.cumsum(45 + 60 * np.exp(-t / 0.08)) / SR) * np.exp(-t / 0.3)
    place(out, thud * 0.8, 0.55)
    place(out, ms.biquad(ms.noise(n, "pink"), "lp", 600) * np.exp(-t / 0.4) * 0.3, 0.55)
    return space(out, 1.8, 0.35, 4500), -4.0


def life_up():
    """Coração: arpejo maior caloroso + duas batidas de coração macias."""
    out = np.zeros((ms.samples(2.2), 2))
    for k, name in enumerate(("D5", "F#5", "A5", "D6")):
        place(out, ms.stereo(ms.inst_ep(F(name), 0.4, 0.55 + 0.05 * k), 0.3 * (k - 1.5) / 1.5), k * 0.07)
    n, t = t_axis(0.25)
    beat = np.sin(2 * np.pi * np.cumsum(70 + 60 * np.exp(-t / 0.03)) / SR) * np.exp(-t / 0.06) * 0.6
    place(out, beat, 0.32)
    place(out, beat * 0.8, 0.5)
    place(out, ms.stereo(sparkle(0.8, 30, 6000, 11000, 7) * 0.35), 0.2)
    return space(out, 1.4, 0.3), -4.0

# ================================================================ coleta e progresso

def collect(root="D6", letter=False, seed=11):
    """Coleta: pingo cristalino em duas notas (fundamental + quinta) com faísca e "pop" grave.

    O jogo altera a afinação por SetSoundPitch a cada coleta (escala pentatônica),
    formando uma pequena melodia ascendente enquanto a bateria enche."""
    out = np.zeros((ms.samples(1.5), 2))
    f = F(root)
    place(out, ms.stereo(ms.inst_bell(f, 0.2, 0.8, ratio=2.0, decay=0.7, index=1.2), -0.15), 0)
    place(out, ms.stereo(ms.inst_glass(f * 1.5, 0.15, 0.55), 0.2), 0.055)
    n, t = t_axis(0.08)
    place(out, np.sin(2 * np.pi * np.cumsum(220 * np.exp(-t / 0.02) + 90) / SR) * np.exp(-t / 0.02) * 0.35, 0)
    place(out, ms.stereo(sparkle(0.35, 40, 7000, 12000, seed) * 0.4, 0), 0.02)
    if letter:  # letra: "virar de página" suave + vogal sintetizada, mesma família sonora
        n, t = t_axis(0.12)
        page = ms.biquad(ms.noise(n), "bp", 3800, 1.0) * np.sin(np.pi * t / 0.12) ** 2 * 0.25
        place(out, page, 0)
        place(out, ms.stereo(ms.formant(ms.saw(f / 2, ms.samples(0.18)), "a") * np.exp(-np.arange(ms.samples(0.18)) / (0.05 * SR)) * 0.12), 0.01)
    return space(out, 0.9, 0.22, 9000), -7.0


def battery_full():
    """Bateria completa: arpejo ascendente + acorde que floresce + brilho."""
    out = np.zeros((ms.samples(2.6), 2))
    for k, name in enumerate(("D5", "F#5", "A5", "D6", "E6", "A6")):
        place(out, ms.stereo(ms.inst_bell(F(name), 0.3, 0.55, ratio=2.0, decay=1.2, index=1.0), 0.6 * (k / 5 - 0.5)), k * 0.06)
    chord = sum(ms.inst_pad(F(n), 0.6, 0.6, bright=4000, attack=0.05, release=0.7, detune=12) for n in ("D5", "A5", "F#6"))
    place(out, chord, 0.35)
    place(out, ms.stereo(sparkle(1.2, 45, 6000, 12000, 5) * 0.5), 0.3)
    return space(out, 1.8, 0.35), -4.0


def boost(kind):
    """Cada boost tem assinatura própria, mas todos dividem o mesmo "brilho de energia"."""
    out = np.zeros((ms.samples(2.0), 2))
    if kind == "shield":  # domo de energia: acorde que sobe + anel de vidro
        n, t = t_axis(0.8)
        dome = ms.supersaw(F("A4") * (1 + 0.5 * (t / 0.8)), n, 5, 14)
        dome = ms.sweep(dome, "lp", 600 + 5000 * (t / 0.8), q=1.4)
        place(out, (dome.T * (np.sin(np.pi * t / 0.8) ** 0.7)).T * 0.6, 0)
        place(out, ms.stereo(ms.inst_glass(F("E6"), 0.5, 0.7)), 0.45)
    elif kind == "magnet":  # zumbido magnético que se alinha
        n, t = t_axis(0.9)
        hum = ms.sine(F("D3") * (1 + 0.02 * np.sin(2 * np.pi * 18 * t)), n) * 0.6
        hum += ms.sine(F("A3"), n) * 0.3 * np.sin(2 * np.pi * 9 * t) ** 2
        fm = np.sin(2 * np.pi * F("D5") * t + 3 * np.exp(-t / 0.2) * np.sin(2 * np.pi * F("D5") * 1.5 * t))
        place(out, (hum + fm * 0.35) * np.sin(np.pi * t / 0.9), 0)
        place(out, ms.stereo(ms.drum_wood(0.8, 1800)), 0)
    elif kind == "slow":  # tempo desacelera: acorde que "afunda" como fita parando
        n, t = t_axis(1.2)
        drop = 2 ** (-1.0 * (t / 1.2) ** 1.5)
        x = sum(ms.saw(F(nm) * drop, n) for nm in ("D4", "A4", "F#5")) / 3
        x = ms.sweep(x, "lp", 4000 * drop ** 2 + 200, q=1.0)
        place(out, ms.chorus(x * np.minimum(1, t / 0.05) * np.exp(-t / 0.5) * 0.7, mix=0.6), 0)
        place(out, ms.stereo(ms.cymbal(0.5, 0.6, reverse=True)), 0)
    elif kind == "double":  # x2: salto de oitava brilhante, dois "pingos" casados
        for k, (name, at) in enumerate((("A5", 0), ("A6", 0.09), ("E6", 0.18), ("A6", 0.27))):
            place(out, ms.stereo(ms.inst_bell(F(name), 0.2, 0.6, ratio=3.0, decay=0.8), 0.4 * (-1) ** k), at)
        place(out, ms.stereo(sparkle(0.6, 50, 6000, 12000, 9) * 0.45), 0.05)
    elif kind == "phase":  # intangível: sopro fantasmagórico com coro
        n, t = t_axis(1.1)
        ghost = ms.inst_choir(F("A4"), 0.5, 0.8, vowel="u")[:n] + ms.inst_choir(F("E5"), 0.5, 0.6, vowel="o")[:n]
        place(out, ms.chorus(ghost, depth_ms=9, rate=1.2, mix=0.7), 0)
        air = ms.sweep(ms.noise(n), "bp", 2000 + 3000 * t, q=1.5) * np.sin(np.pi * t / 1.1) * 0.25
        place(out, air, 0)
    return space(out, 1.3, 0.3), -5.0


def shield_break():
    """Escudo se rompe: estilhaço de vidro + descarga de energia descendente."""
    out = np.zeros((ms.samples(1.4), 2))
    rng = np.random.default_rng(4)
    for k in range(14):
        f = rng.uniform(2500, 9000)
        place(out, ms.stereo(ms.inst_glass(f, 0.05, 0.4)[: ms.samples(0.4)], rng.uniform(-0.8, 0.8)), rng.random() * 0.12)
    n, t = t_axis(0.5)
    zap = ms.supersaw(F("E6") * 2 ** (-3 * t / 0.5), n, 3, 20)
    place(out, (ms.biquad(zap, "lp", 5000).T * np.exp(-t / 0.15)).T * 0.4, 0)
    place(out, ms.biquad(ms.noise(n), "hp", 3000) * np.exp(-t / 0.05) * 0.6, 0)
    return space(out, 1.0, 0.25), -4.0

# ================================================================ portais

def portal_open():
    """Portal se abre: vórtice ressonante girando + acorde ascendente + brilho (~2,4 s)."""
    n, t = t_axis(2.6)
    swirl = ms.sweep(ms.noise(n, "pink"), "bp", 400 * (1 + 3 * t / 2.6) * (1 + 0.3 * np.sin(2 * np.pi * 3.2 * t)), q=4.0)
    pan = np.sin(2 * np.pi * 0.9 * t)
    st = np.stack([swirl * (1 - pan) * 0.5, swirl * (1 + pan) * 0.5], axis=1)
    env = np.clip(t / 1.8, 0, 1) ** 1.5 * np.exp(-np.clip(t - 2.1, 0, None) / 0.2)
    out = st * env[:, None] * 0.6
    chord = sum(ms.inst_pad(F(nm), 2.0, 0.55, bright=5000, attack=1.2, release=0.4, detune=16) for nm in ("D4", "A4", "E5", "F#5"))
    place(out, chord[:n] * 0.8, 0.1)
    place(out, ms.stereo(sparkle(2.2, 30, 5000, 12000, 21) * 0.5), 0.4)
    return space(out, 2.0, 0.35), -4.0


def portal_enter():
    """Travessia: sucção rápida + florescer brilhante."""
    out = np.zeros((ms.samples(2.4), 2))
    n, t = t_axis(0.45)
    suck = ms.sweep(ms.noise(n), "bp", 300 * 30 ** (t / 0.45), q=2.5) * (t / 0.45) ** 2
    place(out, suck * 0.9, 0)
    for k, nm in enumerate(("D5", "A5", "D6", "F#6", "A6")):
        place(out, ms.stereo(ms.inst_bell(F(nm), 0.5, 0.6, ratio=2.0, decay=1.4, index=0.8), 0.5 * (k / 2 - 1)), 0.42 + k * 0.03)
    place(out, ms.stereo(sparkle(0.9, 60, 6000, 12000, 31) * 0.5), 0.42)
    return space(out, 1.8, 0.4), -4.0


def world_whoosh():
    """Troca de mundo sob o escurecimento: swell reverso + queda de sub + abertura."""
    out = np.zeros((ms.samples(2.2), 2))
    place(out, ms.stereo(ms.cymbal(0.8, 0.9, reverse=True)), 0)
    n, t = t_axis(1.2)
    sub = np.sin(2 * np.pi * np.cumsum(90 * 2 ** (-t / 0.5)) / SR) * np.exp(-t / 0.5) * 0.9
    place(out, sub, 0.85)
    chord = sum(ms.inst_strings(F(nm), 0.6, 0.45, attack=0.4, release=0.8) for nm in ("A3", "E4", "A4", "C#5"))
    place(out, chord, 0.9)
    return space(out, 2.2, 0.4), -4.0

# ================================================================ perigos ambientais

def env_event(world):
    """Evento ambiental de cada mundo (~3 s), acompanha o envelope seno do jogo."""
    n, t = t_axis(3.2)
    env = np.sin(np.pi * np.clip(t / 3.0, 0, 1)) ** 1.2
    rng = np.random.default_rng(100 + world)
    if world == 0:   # Ar: rajada com assobio
        x = ms.sweep(ms.noise(n, "pink"), "bp", 500 + 1800 * env, q=2.0)
        whistle = ms.sine(F("A5") * (1 + 0.04 * np.sin(2 * np.pi * 0.7 * t)), n) * 0.08 * env
        y = np.stack([x * (0.6 + 0.4 * np.sin(2 * np.pi * 0.3 * t)), x * (0.6 - 0.4 * np.sin(2 * np.pi * 0.3 * t))], 1) + ms.stereo(whistle)
    elif world == 1:  # Água: corrente submersa
        x = ms.sweep(ms.noise(n, "brown"), "lp", 200 + 700 * env, q=1.4) * 2
        y = ms.stereo(x, width=1.0) + ms.texture_water(3.2, 0.6, bubbles=1.0, seed=world)[:n] * 0.5
    elif world == 2:  # Floresta: onda de folhas e galhos
        x = ms.biquad(ms.noise(n) * (rng.random(n) > 0.6), "bp", 4200, 0.8) * 0.6
        creak = ms.biquad(ms.saw(90 + 20 * np.sin(2 * np.pi * 0.8 * t), n), "bp", 700, 6) * 0.15
        y = ms.stereo(x + creak, width=1.0)
    elif world == 3:  # Gelo: nevasca cristalina
        x = ms.sweep(ms.noise(n, "pink"), "bp", 1500 + 3500 * env, q=1.5)
        y = ms.stereo(x, width=1.0) + ms.texture_ice(3.2, 0.8, seed=world)[:n] * 0.6
    elif world == 4:  # Terra: redemoinho de areia com ronco
        x = ms.texture_sand(3.2, 1.2, seed=world)[:n]
        rumble = ms.biquad(ms.noise(n, "brown"), "lp", 110) * 1.5
        y = x + ms.stereo(rumble)
    else:            # Fogo: erupção (estrondo + chiado de lava)
        boom = np.sin(2 * np.pi * np.cumsum(38 + 50 * np.exp(-t / 0.3)) / SR) * np.exp(-t / 1.0)
        y = ms.stereo(boom * 1.2) + ms.texture_crackle(3.2, 1.0, density=80, seed=world)[:n]
    return space((y.T * env).T, 1.2, 0.2), -6.0


def env_warning():
    """Aviso discreto e tonal de evento ambiental (os 85 passos de antecipação)."""
    out = np.zeros((ms.samples(1.2), 2))
    for k, nm in enumerate(("A4", "E5")):
        place(out, ms.stereo(ms.inst_mallet(F(nm), 0.3, 0.55, wood=False), 0.3 * (k * 2 - 1)), k * 0.16)
    return space(out, 1.5, 0.35), -10.0


def hazard(kind):
    """Lançamento de perigo horizontal: assinatura curta por tipo (0,5-1 s)."""
    n, t = t_axis(1.0)
    if kind == "wind":
        x = ms.sweep(ms.noise(n, "pink"), "bp", 400 + 2500 * np.sin(np.pi * t) ** 2, q=3.0) * np.sin(np.pi * t) ** 2
        y = np.stack([x * (1 - t), x * t], 1) * 1.6
    elif kind == "jelly":
        zap = np.sin(2 * np.pi * np.cumsum(300 + 200 * np.sin(2 * np.pi * 14 * t)) / SR) * np.exp(-t / 0.3)
        bloop = ms.formant(ms.saw(F("E3") * (1 + 0.5 * np.exp(-t / 0.1)), n), "o") * np.exp(-t / 0.2) * 0.3
        y = ms.stereo(zap * 0.4 + bloop + ms.biquad(ms.noise(n), "hp", 6000) * (ms.RNG.random(n) > 0.995) * 0.6)
    elif kind == "vine":
        crack = ms.biquad(ms.noise(n), "bp", 2500, 1.5) * np.exp(-t / 0.015) * 1.5
        rustle = ms.biquad(ms.noise(n) * (ms.RNG.random(n) > 0.7), "bp", 3500, 0.8) * np.exp(-t / 0.35) * 0.5
        whip = ms.sweep(ms.noise(n), "bp", 6000 * np.exp(-t / 0.05) + 600, q=4) * np.exp(-t / 0.08) * 0.8
        y = ms.stereo(crack + rustle + whip)
    elif kind == "ice":
        whoosh = ms.sweep(ms.noise(n), "bp", 1500 + 5000 * t, q=3) * np.sin(np.pi * t) * 0.7
        shard = sum(ms.inst_glass(f, 0.1, 0.5)[:n] for f in (3100, 4700, 6900)) * 0.6
        y = ms.stereo(whoosh + shard)
    elif kind == "rock":
        roll = ms.biquad(ms.noise(n, "brown"), "lp", 250) * (0.6 + 0.4 * np.sin(2 * np.pi * 11 * t)) * np.sin(np.pi * t) * 2
        thud = np.sin(2 * np.pi * np.cumsum(60 + 80 * np.exp(-t / 0.04)) / SR) * np.exp(-t / 0.2)
        y = ms.stereo(roll + thud * 0.8)
    else:  # fire
        roar = ms.sweep(ms.noise(n), "bp", 800 + 2000 * np.sin(np.pi * t), q=1.2) * np.sin(np.pi * t) ** 0.5
        crack = ms.texture_crackle(1.0, 1.0, density=120, seed=8)[:n]
        y = ms.stereo(ms.saturate(roar * 1.5, 1.5)) + crack * 0.6
    return space(y, 0.8, 0.15), -8.0


def hazard_warning():
    """Pequeno "tell" quando o perigo aparece em aviso (antes de ser perigoso)."""
    out = ms.inst_glass(F("B5"), 0.1, 0.6)[: ms.samples(0.5)] + ms.inst_glass(F("F#6"), 0.1, 0.35)[: ms.samples(0.5)]
    return space(out, 0.8, 0.25), -14.0

# ================================================================ Grootspectro

def boss_appear():
    """Aparição: estrondo, coro sombrio crescendo e o motivo do Titã nos graves (~3 s)."""
    out = np.zeros((ms.samples(4.0), 2))
    place(out, ms.stereo(ms.cymbal(0.7, 1.5, reverse=True)), 0)
    place(out, ms.stereo(ms.drum_taiko(1.0, 42, 2.5)), 1.45)
    for k, nm in enumerate(("E2", "F2", "E2", "A#1")):
        place(out, ms.inst_brass(F(nm), 0.55, 0.7, bright=0.5), 1.5 + k * 0.4)
    choir = sum(ms.inst_choir(F(nm), 2.0, 0.6) for nm in ("E3", "B3", "G4"))
    place(out, choir, 1.0)
    return space(out, 3.0, 0.4, 5000), -3.0


def boss_warning():
    """Aviso de ataque: sussurro espectral ascendente (65 passos de antecipação)."""
    n, t = t_axis(1.1)
    rise = ms.formant(ms.saw(F("E3") * 2 ** (t / 1.1 * 7 / 12), n), "u") * (t / 1.1) * 0.5
    air = ms.sweep(ms.noise(n), "bp", 800 + 3000 * t, q=3) * (t / 1.1) ** 2 * 0.4
    return space(ms.chorus(rise + air, mix=0.5), 1.0, 0.3, 5000), -9.0


def boss_shot():
    """Projétil espectral: lamento curto que cai, com cauda de vento."""
    n, t = t_axis(0.6)
    wail = ms.formant(ms.saw(F("B4") * 2 ** (-t / 0.6), n), "o") * np.exp(-t / 0.2) * 0.6
    whoosh = ms.sweep(ms.noise(n), "bp", 3000 * np.exp(-t / 0.2) + 300, q=2) * np.exp(-t / 0.15) * 0.5
    return space(wail + whoosh, 0.9, 0.25, 5000), -9.0


def combat_energy():
    """Energia de combate: coleta multicolorida (acorde com cores do arco-íris)."""
    out = np.zeros((ms.samples(1.5), 2))
    for k, nm in enumerate(("E5", "G#5", "B5", "D#6")):
        place(out, ms.stereo(ms.inst_bell(F(nm), 0.2, 0.55, ratio=2.0, decay=0.8, index=1.4), 0.5 * (k / 1.5 - 1)), k * 0.035)
    place(out, ms.stereo(sparkle(0.5, 60, 5000, 12000, 41) * 0.5), 0.02)
    return space(out, 1.0, 0.3), -6.0


def battery_ready():
    """Megarraio pronto: acorde arco-íris sustentado com pulso."""
    out = np.zeros((ms.samples(1.6), 2))
    chord = sum(ms.inst_pad(F(nm), 0.9, 0.6, bright=6000, attack=0.05, release=0.5, detune=20) for nm in ("E4", "B4", "E5", "G#5", "B5"))
    place(out, ms.tremolo(chord, 8.0, 0.4), 0)
    for k, nm in enumerate(("E6", "G#6", "B6", "E7")):
        place(out, ms.stereo(ms.inst_glass(F(nm), 0.2, 0.5)), 0.05 + k * 0.08)
    return space(out, 1.4, 0.35), -5.0


def beam():
    """Megarraio arco-íris, alinhado à animação: carga 0-0,58 s, impacto em 0,58 s, cauda até ~1,45 s."""
    total = 2.6
    out = np.zeros((ms.samples(total), 2))
    charge = 0.58
    n, t = t_axis(charge)
    # 1-3. carregamento e preparação: acorde que sobe em espiral + sucção
    rise = sum(ms.saw(F(nm) * 2 ** (t / charge), n) for nm in ("E4", "B4", "G#5")) / 3
    rise = ms.sweep(rise, "lp", 500 + 6000 * (t / charge) ** 2, q=2.5)
    suck = ms.sweep(ms.noise(n), "bp", 300 * 25 ** (t / charge), q=3) * (t / charge) ** 2
    place(out, (rise * 0.45 + suck * 0.6) * (t / charge) ** 1.5, 0)
    # 4. disparo: impacto grave + corpo energético largo
    n2, t2 = t_axis(total - charge)
    boom = np.sin(2 * np.pi * np.cumsum(40 + 120 * np.exp(-t2 / 0.06)) / SR) * np.exp(-t2 / 0.35) * 1.2
    body = ms.supersaw(F("E3"), n2, 7, 30) + ms.supersaw(F("B3"), n2, 7, 30) * 0.7 + ms.supersaw(F("E4"), n2, 7, 30) * 0.5
    body = ms.sweep(body, "lp", 7000 * np.exp(-t2 / 0.5) + 400, q=0.9)
    body = ms.saturate(body * 1.3, 1.8) * (np.exp(-t2 / 0.6) * np.minimum(1, t2 / 0.005))[:, None] * 0.5
    crack = ms.biquad(ms.noise(n2), "hp", 2000) * np.exp(-t2 / 0.04) * 0.9
    place(out, ms.stereo(boom + crack) + body, charge)
    # 5. cauda: arpejo arco-íris descendo e faíscas acompanhando o feixe
    for k, nm in enumerate(("E7", "B6", "G#6", "E6", "B5", "G#5", "E5")):
        place(out, ms.stereo(ms.inst_bell(F(nm), 0.2, 0.4, ratio=2.0, decay=0.6), 0.7 * np.sin(k)), charge + 0.05 + k * 0.08)
    place(out, ms.stereo(sparkle(0.8, 70, 5000, 12000, 51) * 0.6), charge + 0.02)
    return space(out, 2.0, 0.35), -1.5


def boss_hurt():
    """Grootspectro ferido: urro grave com formantes + estalo de madeira espectral."""
    n, t = t_axis(1.3)
    growl = ms.formant(ms.saw(F("E2") * (1 + 0.08 * np.sin(2 * np.pi * 23 * t)) * 2 ** (-t / 2), n), "a") * np.exp(-t / 0.5)
    growl = ms.saturate(growl * 2, 1.6) * 0.7
    crack = ms.biquad(ms.noise(n), "bp", 1500, 2) * np.exp(-t / 0.02) * 1.2
    return space(growl + crack, 1.8, 0.35, 4000), -3.0


def boss_phase():
    """Transição de fase: golpe de tambor, metal híbrido e coro que sobe meio tom."""
    out = np.zeros((ms.samples(2.4), 2))
    place(out, ms.stereo(ms.drum_taiko(1.0, 48, 2.0)), 0)
    place(out, ms.stereo(ms.cymbal(0.8, 2.0)), 0)
    for nm in ("E3", "B3", "E4", "G4"):
        place(out, ms.inst_brass(F(nm), 0.5, 0.75, bright=1.2), 0.0)
    for nm in ("F3", "C4", "F4", "Ab4"):
        place(out, ms.inst_choir(F(nm), 1.0, 0.45), 0.55)
    return space(out, 2.5, 0.35, 5000), -2.0


def boss_defeat():
    """Derrota do chefe (~3,6 s, como a animação): colapso, coro desfazendo e resolução em Mi maior."""
    out = np.zeros((ms.samples(5.0), 2))
    place(out, ms.stereo(ms.drum_taiko(1.0, 38, 3.0)), 0)
    n, t = t_axis(2.2)
    collapse = ms.formant(ms.saw(F("E3") * 2 ** (-2 * t / 2.2), n), "a") * np.exp(-t / 0.9) * 0.6
    place(out, ms.stereo(collapse), 0.05)
    rng = np.random.default_rng(6)
    for k in range(10):  # estalos da madeira espectral se desfazendo
        place(out, ms.stereo(ms.drum_wood(0.8, rng.uniform(300, 900)), rng.uniform(-0.8, 0.8)), 0.3 + k * 0.17 + rng.random() * 0.05)
    chord = sum(ms.inst_strings(F(nm), 1.6, 0.5, attack=0.8, release=1.2) for nm in ("E3", "B3", "E4", "G#4", "B4"))
    place(out, chord, 2.0)
    place(out, ms.stereo(sparkle(2.0, 40, 5000, 12000, 61) * 0.5), 2.2)
    return space(out, 3.0, 0.4), -2.0

# ================================================================ interface

def ui_tone(notes, gap=0.05, vel=0.5, dur=0.25):
    out = np.zeros((ms.samples(dur + gap * len(notes) + 0.9), 2))
    for k, nm in enumerate(notes):
        place(out, ms.stereo(ms.inst_mallet(F(nm), 0.2, vel, wood=False), 0.2 * (k - (len(notes) - 1) / 2)), k * gap)
    return space(out, 0.7, 0.2)


def ui_start():
    """Início da aventura: swell + motivo de Morph em sinos (5-1-2-3)."""
    out = np.zeros((ms.samples(2.4), 2))
    place(out, ms.stereo(ms.cymbal(0.5, 0.8, reverse=True)), 0)
    for k, nm in enumerate(("A5", "D6", "E6", "F#6")):
        place(out, ms.stereo(ms.inst_bell(F(nm), 0.4, 0.6, ratio=3.0, decay=1.6), 0.3 * (k - 1.5)), 0.75 + k * 0.12)
    chord = sum(ms.inst_pad(F(nm), 0.8, 0.5, bright=4500, attack=0.05, release=0.9) for nm in ("D4", "A4", "F#5"))
    place(out, chord, 0.75)
    return space(out, 1.8, 0.35), -4.0


SFX = {
    "flap_1": lambda: flap(1), "flap_2": lambda: flap(2), "flap_3": lambda: flap(3),
    "hit": hit, "defeat": defeat, "life_up": life_up,
    "collect": lambda: collect("D6"), "battery_full": battery_full,
    "boost_shield": lambda: boost("shield"), "boost_magnet": lambda: boost("magnet"),
    "boost_slow": lambda: boost("slow"), "boost_double": lambda: boost("double"), "boost_phase": lambda: boost("phase"),
    "shield_break": shield_break,
    "portal_open": portal_open, "portal_enter": portal_enter, "world_whoosh": world_whoosh,
    "env_ar": lambda: env_event(0), "env_agua": lambda: env_event(1), "env_floresta": lambda: env_event(2),
    "env_gelo": lambda: env_event(3), "env_terra": lambda: env_event(4), "env_fogo": lambda: env_event(5),
    "env_warning": env_warning, "hazard_warning": hazard_warning,
    "hazard_wind": lambda: hazard("wind"), "hazard_jelly": lambda: hazard("jelly"), "hazard_vine": lambda: hazard("vine"),
    "hazard_ice": lambda: hazard("ice"), "hazard_rock": lambda: hazard("rock"), "hazard_fire": lambda: hazard("fire"),
    "boss_appear": boss_appear, "boss_warning": boss_warning, "boss_shot": boss_shot,
    "combat_energy": combat_energy, "battery_ready": battery_ready, "beam": beam,
    "boss_hurt": boss_hurt, "boss_phase": boss_phase, "boss_defeat": boss_defeat,
    "ui_move": lambda: (ui_tone(["A5"], vel=0.35, dur=0.12), -16.0),
    "ui_confirm": lambda: (ui_tone(["D6", "A6"], 0.06, 0.5), -10.0),
    "ui_back": lambda: (ui_tone(["A5", "D5"], 0.07, 0.45), -11.0),
    "ui_pause": lambda: (ui_tone(["F#5", "D5", "A4"], 0.05, 0.4), -11.0),
    "ui_resume": lambda: (ui_tone(["A4", "D5", "F#5"], 0.05, 0.4), -11.0),
    "ui_volume": lambda: (ui_tone(["E6"], vel=0.35, dur=0.1), -14.0),
    "ui_start": ui_start,
}

try:  # efeitos exclusivos da versão educativa (presentes apenas na branch educativa)
    import sfx_edu
    SFX.update(sfx_edu.build(globals()))
except ImportError:
    pass
