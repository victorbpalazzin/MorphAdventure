"""Composições originais de Morph Adventure.

Motivo de Morph (assinatura do jogo), em graus da escala: 5-1-2-3 (subida que
"olha para cima") e a resposta 5-3-2-1. Ele reaparece transformado em todos os
mundos (modo, ritmo, timbre e registro mudam), na vitória e, em contraponto
ao motivo cromático de Grootspectro (1-b2-1-#4), na batalha final.
"""
import numpy as np
import morphsynth as ms
from music_core import Song, voicing, bass_of, section, parse_melody

N = ms.note


def phrase(text, bars, bpb=4):
    """Valida que a frase preenche exatamente `bars` compassos (evita erros de escrita)."""
    total = sum(d for _, d in parse_melody(text))
    assert abs(total - bars * bpb) < 1e-6, f"frase com {total} tempos, esperado {bars * bpb}: {text[:40]}..."
    return text


def drums_soft(s, bars, kick=(0,), snare=(), hats=(), shaker=(), rim=(), vel=1.0, bus="drums"):
    for bar in bars:
        for b in kick:
            s.hit(bus, "kick_soft", lambda: ms.drum_kick(0.8, tone=50, punch=0.6, decay=0.4, click=0.4), bar, b, vel * 0.9)
        for b in snare:
            s.hit(bus, "snare_soft", lambda: ms.drum_snare(0.7, tone=200, snap=0.7, decay=0.2), bar, b, vel * 0.6)
        for b in hats:
            s.hit(bus, "hat", lambda: ms.drum_hat(0.5), bar, b, vel * (0.55 if b % 1 else 0.4), pan=0.25)
        for b in shaker:
            s.hit(bus, "shaker", lambda: ms.drum_shaker(0.5), bar, b, vel * (0.5 if b % 0.5 else 0.7), pan=-0.3)
        for b in rim:
            s.hit(bus, "wood", lambda: ms.drum_wood(0.6, 1050), bar, b, vel * 0.32, pan=0.15)


def pads(s, bus, prog, bars, inst=ms.inst_pad, low=N("G3"), high=N("E5"), vel=0.55, beats=4, **kw):
    for i, bar in enumerate(bars):
        sym = prog[i % len(prog)]
        s.chord_notes(bus, inst, voicing(sym, low, high), bar, 0, beats, vel, **kw)


def arpeggio(s, bus, prog, bars, inst, pattern, step=0.5, low=N("D4"), high=N("D6"), vel=0.5, length=None, **kw):
    for i, bar in enumerate(bars):
        notes = voicing(prog[i % len(prog)], low, high, spread=False)
        notes = notes + [n + 12 for n in notes]
        beat = 0.0
        for k, idx in enumerate(pattern):
            if idx is not None:
                n = notes[idx % len(notes)]
                accent = 1.0 if beat % 1 == 0 else 0.82
                s.note(bus, inst, n, bar, beat, length or step * 1.6, vel * accent, pan=0.35 * np.sin(k * 1.3), **kw)
            beat += step
            if beat >= 4:
                break


def bassline(s, bus, prog, bars, rhythm, inst=ms.inst_bass, vel=0.75, shift=0, **kw):
    for i, bar in enumerate(bars):
        sym = prog[i % len(prog)]
        root = bass_of(sym, shift)
        for beat, dur, iv in rhythm:
            s.note(bus, inst, root + iv, bar, beat, dur, vel * (1 if beat == 0 else 0.8), **kw)

# ===================================================================== MENU

def menu():
    """Ré maior, 96 bpm. Abertura cativante que apresenta o motivo de Morph."""
    s = Song("menu", 96, 48, seed=101)
    s.bus("pad", gain=0.9, reverb=0.45, hp=140)
    s.bus("ep", gain=0.8, reverb=0.3, chorus=0.35, delay=0.12, delay_beats=0.75)
    s.bus("arp", gain=0.55, reverb=0.25, delay=0.3, delay_beats=0.75, hp=300, duck=0.35)
    s.bus("lead", gain=0.95, reverb=0.3, delay=0.18, delay_beats=1.5)
    s.bus("bells", gain=0.5, reverb=0.6, hp=600)
    s.bus("bass", gain=0.85, reverb=0.0, lp=2500, duck=0.3)
    s.bus("drums", gain=0.8, reverb=0.12)
    s.bus("strings", gain=0.7, reverb=0.45, hp=200)
    s.bus("tex", gain=0.5, reverb=0.3)
    A = ["Dmaj9", "Bm11", "Gmaj7#11", "A6", "D/F#", "Gmaj7", "Em9", "A7sus4"]
    B = ["Gmaj7", "A6", "F#m7", "Bm9", "Gmaj9", "A", "D/F#", "A7sus4"]
    melA = phrase("A4/.5 D5/.5 E5/.5 F#5/2.5 r/.5 A5/.5 F#5/.5 E5/.5 D5/2 B4/.5 D5/.5 E5/.5 G5/2.5 "
            "F#5/1 E5/1 C#5/1 A4/1 A4/.5 D5/.5 E5/.5 F#5/1.5 A5/1 B5/1.5 A5/.5 G5/1 F#5/1 "
            "E5/1 F#5/.5 G5/.5 B5/1 A5/1 G5/.5 F#5/.5 E5/1 C#5/2", 8)
    melB = phrase("D6/1.5 B5/.5 A5/1 G5/1 A5/1.5 E5/.5 C#6/2 C#6/1 B5/.5 A5/.5 F#5/2 B5/1 A5/.5 F#5/.5 D5/2 "
            "G5/.5 A5/.5 B5/.5 D6/2.5 C#6/1 B5/1 A5/1 E5/1 F#5/1 A5/1 G5/1 E5/1 A5/4", 8)
    # Introdução (1-8): pad, EP e brilhos; motivo apresentado delicadamente nos sinos
    pads(s, "pad", A, section(0, 8), vel=0.5, bright=1800, attack=1.5)
    for bar in section(0, 8):
        s.chord_notes("ep", ms.inst_ep, voicing(A[bar % 8], N("F#3"), N("A4")), bar, 0, 3.5, 0.45, strum=0.03)
        s.chord_notes("ep", ms.inst_ep, voicing(A[bar % 8], N("F#3"), N("A4"))[1:], bar, 2.5, 1.5, 0.3)
    s.add("tex", ms.texture_wind(s.time(8), 0.08, center=1800), 0)
    s.melody("bells", ms.inst_bell, "A5/.5 D6/.5 E6/.5 F#6/2.5 r/.5 A6/.5 F#6/.5 E6/.5 D6/2", 4, vel=0.45, ratio=3.0, decay=2.0)
    s.melody("bells", ms.inst_bell, "B5/.5 D6/.5 E6/.5 G6/2.5 F#6/1 E6/1 C#6/2", 6, vel=0.4, ratio=3.0, decay=2.0)
    s.add("drums", ms.riser(s.spb * 4, 0.35, 400, 7000), s.time(7))
    # Seção A (9-24): groove leve, baixo, arpejo e o tema completo
    for rep, start in enumerate((8, 16)):
        bars = section(start, 8)
        pads(s, "pad", A, bars, vel=0.42, bright=2400)
        arpeggio(s, "arp", A, bars, ms.inst_pluck, [0, 2, 4, 1, 3, 5, 2, 4], step=0.5, low=N("D4"), vel=0.42, bright=0.8)
        bassline(s, "bass", A, bars, [(0, 1.5, 0), (1.5, 0.5, 12), (2, 1.5, 0), (3.5, 0.5, 7)], ms.inst_bass, kind="pluck", bright=700)
        drums_soft(s, bars, kick=(0, 2.5), snare=(1, 3) if rep else (3,), shaker=[i * 0.5 for i in range(8)], rim=(1.75, 3.5))
        s.melody("lead", ms.inst_lead, melA, start, vel=0.62, bright=3000, vibrato=0.005)
        if rep:
            s.melody("bells", ms.inst_bell, melA, start, transpose=12, vel=0.22, ratio=3.0, decay=1.4)
    s.add("drums", ms.cymbal(0.5, 3.0), s.time(16))
    # Seção B (25-32): elevação com cordas e contracanto
    bars = section(24, 8)
    pads(s, "strings", B, bars, inst=ms.inst_strings, low=N("D4"), high=N("A5"), vel=0.5)
    pads(s, "pad", B, bars, vel=0.35, bright=2800)
    arpeggio(s, "arp", B, bars, ms.inst_pluck, [0, 1, 2, 3, 4, 3, 2, 1], step=0.5, vel=0.38, bright=0.9)
    bassline(s, "bass", B, bars, [(0, 1, 0), (1, 0.5, 0), (1.5, 1, 12), (2.5, 1.5, 7)], ms.inst_bass, kind="sub")
    drums_soft(s, bars, kick=(0, 1.5, 2.5), snare=(1, 3), hats=[i * 0.5 for i in range(8)], shaker=[i * 0.25 for i in range(16)])
    s.melody("lead", ms.inst_lead, melB, 24, vel=0.66, bright=3600)
    for bar in (27, 31):
        for k, b in enumerate((2, 2.5, 3, 3.5)):
            s.hit("drums", "tom", lambda: ms.drum_tom(0.8, 120), bar, b, 0.5 + 0.12 * k, pan=0.3 - 0.2 * k)
    s.add("drums", ms.riser(s.spb * 4, 0.45, 300, 8000), s.time(31))
    # Seção A' (33-44): tutti com motivo dobrado em oitavas
    bars = section(32, 12)
    s.add("drums", ms.cymbal(0.6, 3.0), s.time(32))
    pads(s, "pad", A, bars, vel=0.45, bright=3000)
    pads(s, "strings", A, bars, inst=ms.inst_strings, low=N("A3"), high=N("F#5"), vel=0.38)
    arpeggio(s, "arp", A, bars, ms.inst_pluck, [0, 2, 4, 2, 5, 4, 2, 4], step=0.5, vel=0.42, bright=0.9)
    bassline(s, "bass", A, bars, [(0, 1.5, 0), (1.5, 0.5, 12), (2, 1, 0), (3, 0.5, 7), (3.5, 0.5, 12)], ms.inst_bass, kind="pluck", bright=800)
    drums_soft(s, bars, kick=(0, 1.5, 2.5), snare=(1, 3), hats=[i * 0.5 for i in range(8)], shaker=[i * 0.25 for i in range(16)])
    s.melody("lead", ms.inst_lead, melA, 32, vel=0.68, bright=3800)
    s.melody("bells", ms.inst_bell, melA, 32, transpose=12, vel=0.25, ratio=3.0, decay=1.4)
    s.melody("lead", ms.inst_lead, "A5/.5 D6/.5 E6/.5 F#6/2.5 r/.5 A6/.5 F#6/.5 E6/.5 D6/2 B5/.5 D6/.5 E6/.5 G6/2.5 F#6/1 E6/1 C#6/2",
             40, vel=0.5, bright=3200)
    # Final (45-48): desaceleração que volta ao início sem emenda
    bars = section(44, 4)
    pads(s, "pad", ["Gmaj7", "A6", "Bm9", "A7sus4"], bars, vel=0.45, bright=1600, attack=1.2)
    for bar in bars:
        s.chord_notes("ep", ms.inst_ep, voicing(["Gmaj7", "A6", "Bm9", "A7sus4"][bar - 44], N("F#3"), N("A4")), bar, 0, 3.5, 0.4, strum=0.04)
    s.melody("bells", ms.inst_bell, "D6/1 E6/1 F#6/2 A5/1 E6/1 D6/2 B5/4 r/4", 44, vel=0.35, ratio=3.0, decay=2.4)
    s.add("tex", ms.texture_wind(s.time(4), 0.08, center=1800), s.time(44))
    return s.render(reverb_seconds=2.6, reverb_damp=7500)


# ===================================================================== AR

def ar():
    """Sol lídio, 108 bpm. Liberdade e altitude: flauta, harpa, pads aéreos e vento estilizado."""
    s = Song("ar", 108, 48, seed=202)
    s.bus("pad", gain=0.8, reverb=0.5, hp=220, width=0.4)
    s.bus("harp", gain=0.6, reverb=0.35, delay=0.22, delay_beats=0.75, hp=250)
    s.bus("flute", gain=0.95, reverb=0.4, delay=0.15, delay_beats=1.5)
    s.bus("bells", gain=0.42, reverb=0.65, hp=900)
    s.bus("bass", gain=0.7, lp=1800, duck=0.25)
    s.bus("drums", gain=0.72, reverb=0.15)
    s.bus("strings", gain=0.65, reverb=0.5, hp=250)
    s.bus("wind", gain=0.55, reverb=0.2, hp=180)
    A = ["Gmaj9", "A/G", "Em9", "Cmaj7#11", "Gmaj7/B", "Cmaj9", "Am9", "D69"]
    B = ["Em9", "Cmaj7", "G/B", "D6", "Em9", "Cmaj7#11", "Am9", "D7sus4"]
    melA = phrase("D5/.5 G5/.5 A5/.5 B5/2.5 C#6/1 B5/.5 A5/.5 E5/2 r/.5 E5/.5 G5/.5 B5/.5 D6/1 B5/1 "
                  "A5/1.5 F#5/.5 E5/1 G5/1 D5/.5 G5/.5 A5/.5 B5/1.5 D6/1 E6/1.5 D6/.5 B5/1 G5/1 "
                  "A5/1 B5/.5 C6/.5 E6/1 D6/1 D6/1 B5/.5 A5/.5 F#5/2", 8)
    melB = phrase("E6/2 D6/1 B5/1 C6/1.5 B5/.5 G5/2 B5/1 A5/.5 G5/.5 D6/2 A5/3 r/1 "
                  "G6/1.5 F#6/.5 E6/1 B5/1 C6/1 E6/1 G6/1 E6/1 E6/1.5 D6/.5 C6/1 A5/1 D6/2 A5/1 F#5/1", 8)
    groove = dict(kick=(0, 2.75), snare=(), shaker=[i * 0.5 for i in range(8)], rim=(1, 3))
    # 1-4: céu aberto
    s.add("wind", ms.texture_wind(s.time(48), 0.12, center=900, seed=21), 0)
    pads(s, "pad", A, section(0, 4), vel=0.45, bright=2600, attack=1.6, detune=20, voices=7)
    arpeggio(s, "harp", A, section(0, 4), ms.inst_pluck, [0, 1, 2, 3, 4, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 5], step=0.25,
             low=N("G3"), high=N("G6"), vel=0.35, bright=0.95, decay=0.997)
    s.melody("bells", ms.inst_glass, "D6/1 A6/1 F#6/2 E6/1 B6/1 A6/2", 2, vel=0.4)
    # 5-20: tema na flauta
    for rep, start in enumerate((4, 12)):
        bars = section(start, 8)
        pads(s, "pad", A, bars, vel=0.38, bright=3000, detune=20, voices=7)
        arpeggio(s, "harp", A, bars, ms.inst_pluck, [0, 2, 4, 2, 5, 4, 2, 4] if rep == 0 else [0, 2, 4, 6, 5, 4, 2, 1],
                 step=0.5, low=N("G3"), high=N("G6"), vel=0.4, bright=0.95)
        bassline(s, "bass", A, bars, [(0, 2.5, 0), (2.75, 1.25, 7)], kind="sub", vel=0.6)
        drums_soft(s, bars, **groove, vel=0.85)
        for bar in bars:
            s.hit("drums", "clap", lambda: ms.drum_clap(0.6), bar, 2, 0.35, pan=0.2)
        s.melody("flute", ms.inst_flute, melA, start, vel=0.62 if rep == 0 else 0.68)
        if rep:
            s.melody("bells", ms.inst_glass, melA, start, transpose=12, vel=0.18)
    # 21-28: o voo sobe (cordas, flauta no agudo)
    bars = section(20, 8)
    s.add("drums", ms.cymbal(0.45, 3.0), s.time(20))
    pads(s, "strings", B, bars, inst=ms.inst_strings, low=N("D4"), high=N("B5"), vel=0.45, attack=0.6)
    pads(s, "pad", B, bars, vel=0.3, bright=3600, detune=22, voices=7)
    arpeggio(s, "harp", B, bars, ms.inst_pluck, [0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 3, 4, 5], step=0.25,
             low=N("B3"), high=N("B6"), vel=0.33, bright=1.0)
    bassline(s, "bass", B, bars, [(0, 1.5, 0), (1.5, 1, 7), (2.5, 1.5, 12)], kind="sub", vel=0.62)
    drums_soft(s, bars, kick=(0, 1.75, 2.75), shaker=[i * 0.25 for i in range(16)], rim=(1, 3), hats=(0.5, 1.5, 2.5, 3.5), vel=0.9)
    for bar in bars:
        s.hit("drums", "clap", lambda: ms.drum_clap(0.6), bar, 1, 0.3, pan=-0.2)
        s.hit("drums", "clap", lambda: ms.drum_clap(0.6), bar, 3, 0.35, pan=0.2)
    s.melody("flute", ms.inst_flute, melB, 20, vel=0.72)
    s.add("drums", ms.riser(s.spb * 4, 0.35, 500, 9000), s.time(27))
    # 29-44: tema com contracanto nas cordas e sinos
    bars = section(28, 16)
    s.add("drums", ms.cymbal(0.5, 3.0), s.time(28))
    pads(s, "pad", A, bars, vel=0.4, bright=3400, detune=20, voices=7)
    pads(s, "strings", A, bars, inst=ms.inst_strings, low=N("B3"), high=N("G5"), vel=0.32)
    arpeggio(s, "harp", A, bars, ms.inst_pluck, [0, 2, 4, 6, 5, 4, 2, 1], step=0.5, low=N("G3"), high=N("G6"), vel=0.4, bright=0.95)
    bassline(s, "bass", A, bars, [(0, 1.5, 0), (1.5, 1, 12), (2.75, 1.25, 7)], kind="sub", vel=0.62)
    drums_soft(s, bars, kick=(0, 1.75, 2.75), hats=(0.5, 1.5, 2.5, 3.5), shaker=[i * 0.25 for i in range(16)], rim=(1, 3))
    for bar in bars:
        s.hit("drums", "clap", lambda: ms.drum_clap(0.6), bar, 2, 0.4, pan=0.15)
    s.melody("flute", ms.inst_flute, melA, 28, vel=0.7)
    s.melody("flute", ms.inst_flute, melA, 36, vel=0.66, transpose=12, legato=0.9)
    s.melody("bells", ms.inst_glass, melA, 28, transpose=12, vel=0.15)
    s.melody("strings", ms.inst_strings, "B4/4 A4/4 G4/4 G4/4 B4/4 C5/4 C5/4 A4/4", 36, vel=0.35)
    # 45-48: planando (respiro antes do loop)
    bars = section(44, 4)
    pads(s, "pad", ["Cmaj7#11", "Gmaj9/B", "Am9", "D7sus4"], bars, vel=0.42, bright=2400, attack=1.2, detune=20, voices=7)
    arpeggio(s, "harp", ["Cmaj7#11", "Gmaj9/B", "Am9", "D7sus4"], bars, ms.inst_pluck, [0, 2, 4, 6, 4, 2], step=0.75,
             low=N("G3"), high=N("G6"), vel=0.32, bright=0.9)
    s.melody("bells", ms.inst_glass, "F#6/2 E6/2 D6/4 C6/2 B5/2 A5/4", 44, vel=0.32)
    return s.render(reverb_seconds=2.9, reverb_damp=9000, predelay=0.03)

# ===================================================================== ÁGUA

def inst_drop(f, dur, vel=0.6):
    """Gota cristalina: FM curto com leve queda de afinação."""
    n = ms.samples(0.9)
    t = np.arange(n) / ms.SR
    fr = f * (1 + 0.04 * np.exp(-t / 0.01))
    x = np.sin(2 * np.pi * np.cumsum(fr) / ms.SR + 1.6 * np.exp(-t / 0.05) * np.sin(2 * np.pi * f * 2.0 * t))
    return x * np.exp(-t / 0.18) * vel * 0.3


def inst_voice(f, dur, vel=0.6, vowel="o"):
    """Voz sintética etérea (formantes) para o canto do motivo."""
    n = ms.samples(dur + 0.8)
    t = np.arange(n) / ms.SR
    vib = 1 + 0.006 * np.sin(2 * np.pi * 5.0 * t) * np.clip((t - 0.2) / 0.4, 0, 1)
    src = ms.saw(f * vib, n) + 0.5 * ms.saw(f * vib * 1.004, n)
    x = ms.formant(ms.biquad(src, "lp", 4000), vowel, q=8)
    x += ms.biquad(ms.noise(n), "bp", 2500, 2) * 0.05
    return x * ms.adsr(n, 0.12, 0.3, 0.85, 0.6, hold=dur) * vel * 0.22


def agua():
    """Mi dórico, 84 bpm. Profundidade e fluidez: EP líquido, sub, gotas, voz etérea."""
    s = Song("agua", 84, 40, seed=303)
    s.bus("pad", gain=0.75, reverb=0.55, hp=160, lp=3200)
    s.bus("ep", gain=0.8, reverb=0.35, chorus=0.5, delay=0.25, delay_beats=0.75, lp=5000)
    s.bus("drops", gain=0.45, reverb=0.5, delay=0.45, delay_beats=0.75, hp=600)
    s.bus("voice", gain=0.9, reverb=0.55, delay=0.2, delay_beats=1.5)
    s.bus("bass", gain=0.9, lp=900, duck=0.2)
    s.bus("drums", gain=0.7, reverb=0.2, lp=6000)
    s.bus("water", gain=0.5, reverb=0.3)
    A = ["Em9", "A13", "Cmaj7", "D69", "Em9", "A9", "Gmaj7/B", "B7sus4"]
    B = ["Am9", "Em/G", "Fmaj7#11", "B7sus4", "Am9", "Em/G", "Fmaj7#11", "B7"]
    melA = phrase("B4/1 E5/1 F#5/1 G5/1 A5/2 F#5/1 C#5/1 B4/1 E5/1 G5/1 B5/1 A5/3 F#5/1 "
                  "B5/1.5 G5/.5 F#5/1 E5/1 F#5/2 E5/1 C#5/1 D5/1 F#5/1 B5/1.5 A5/.5 F#5/4", 8)
    melB = phrase("C6/2 B5/1 A5/1 G5/2 E5/1 B4/1 C5/1 E5/1 A5/1 G5/1 F#5/3 D#5/1 "
                  "A5/1 C6/1 E6/2 D6/1 B5/1 G5/2 A5/2 B5/1 C6/1 B5/4", 8)
    s.add("water", ms.texture_water(s.time(40), 0.22, bubbles=0.35, seed=31), 0)
    groove = dict(kick=(0, 1.75), rim=(1, 2.5, 3.25), shaker=[i * 0.5 + 0.25 for i in range(8)])
    # 1-4: mergulho
    pads(s, "pad", A, section(0, 4), vel=0.5, bright=1400, attack=2.0)
    for bar in section(0, 4):
        for b in (0.5, 1.75, 3.0):
            s.note("drops", inst_drop, voicing(A[bar], N("E5"), N("E6"), spread=False)[int(b * 2) % 3], bar, b, 0.3, 0.5)
    # 5-20: groove submerso
    for rep, start in enumerate((4, 12)):
        bars = section(start, 8)
        pads(s, "pad", A, bars, vel=0.4, bright=1800)
        for i, bar in enumerate(bars):
            v = voicing(A[i], N("E3"), N("B4"))
            s.chord_notes("ep", ms.inst_ep, v, bar, 0, 1.5, 0.5, strum=0.02)
            s.chord_notes("ep", ms.inst_ep, v[1:], bar, 1.75, 1.0, 0.36)
            s.chord_notes("ep", ms.inst_ep, v, bar, 3.0, 1.0, 0.4, strum=0.015)
            for k, b in enumerate((0.75, 2.25, 3.5)):
                s.note("drops", inst_drop, voicing(A[i], N("E5"), N("E6"), spread=False)[(k + bar) % 3], bar, b, 0.3, 0.42)
        bassline(s, "bass", A, bars, [(0, 1.5, 0), (1.75, 0.75, 0), (2.5, 1.5, 7)], kind="sub", vel=0.75)
        drums_soft(s, bars, **groove, snare=(3,) if rep else (), vel=0.8)
        if rep:
            s.melody("voice", inst_voice, melA, start, vel=0.62)
    # 21-28: corrente misteriosa (voz em primeiro plano)
    bars = section(20, 8)
    pads(s, "pad", B, bars, vel=0.45, bright=1600)
    for i, bar in enumerate(bars):
        v = voicing(B[i], N("E3"), N("C5"))
        s.chord_notes("ep", ms.inst_ep, v, bar, 0, 2.5, 0.42, strum=0.03)
        s.chord_notes("ep", ms.inst_ep, v[1:], bar, 2.5, 1.5, 0.32)
    bassline(s, "bass", B, bars, [(0, 3, 0), (3, 1, 7)], kind="sub", vel=0.72)
    drums_soft(s, bars, kick=(0,), rim=(2.5,), shaker=[i * 0.5 + 0.25 for i in range(8)], vel=0.7)
    s.melody("voice", inst_voice, melB, 20, vel=0.68, vowel="u")
    s.melody("drops", inst_drop, melB, 20, transpose=12, vel=0.18)
    # 29-36: tema completo com tudo
    bars = section(28, 8)
    pads(s, "pad", A, bars, vel=0.42, bright=2000)
    for i, bar in enumerate(bars):
        v = voicing(A[i], N("E3"), N("B4"))
        s.chord_notes("ep", ms.inst_ep, v, bar, 0, 1.5, 0.5, strum=0.02)
        s.chord_notes("ep", ms.inst_ep, v, bar, 1.75, 1.0, 0.38)
        s.chord_notes("ep", ms.inst_ep, v[1:], bar, 3.0, 1.0, 0.4)
    bassline(s, "bass", A, bars, [(0, 1.5, 0), (1.75, 0.75, 0), (2.5, 1.0, 7), (3.5, 0.5, 10)], kind="sub", vel=0.78)
    drums_soft(s, bars, kick=(0, 1.75, 2.5), snare=(3,), rim=(1, 2.5, 3.25), shaker=[i * 0.25 for i in range(16)], vel=0.85)
    s.melody("voice", inst_voice, melA, 28, vel=0.7, vowel="a")
    s.melody("drops", inst_drop, melA, 28, transpose=12, vel=0.2)
    # 37-40: retorno à superfície
    bars = section(36, 4)
    pads(s, "pad", ["Cmaj7", "D69", "Em9", "B7sus4"], bars, vel=0.45, bright=1300, attack=1.5)
    s.melody("drops", inst_drop, "G5/1 B5/1 D6/2 F#5/1 A5/1 E6/2 G5/2 B5/2 F#5/4", 36, vel=0.35)
    return s.render(reverb_seconds=3.6, reverb_damp=4200, predelay=0.04)

# ===================================================================== FLORESTA

def inst_wood_flute(f, dur, vel=0.6):
    """Madeira sopro (timbre de ocarina/clarinete): harmônicos ímpares + sopro."""
    return ms.inst_lead(f, dur, vel, kind="square", breath=0.35, bright=2600, vibrato=0.007)


def inst_kalimba(f, dur, vel=0.6):
    return ms.inst_mallet(f, dur, vel, wood=False)


def floresta():
    """Lá menor/eólio, 92 bpm. Encantamento: kalimba, pizzicato, madeiras e percussão natural."""
    s = Song("floresta", 92, 44, seed=404)
    s.bus("pad", gain=0.7, reverb=0.45, hp=180, lp=3800)
    s.bus("kalimba", gain=0.6, reverb=0.35, delay=0.18, delay_beats=0.75)
    s.bus("pizz", gain=0.62, reverb=0.3, hp=120)
    s.bus("wood", gain=0.95, reverb=0.4, delay=0.12, delay_beats=1.5)
    s.bus("celesta", gain=0.5, reverb=0.5, hp=500)
    s.bus("bass", gain=0.75, lp=1500)
    s.bus("drums", gain=0.75, reverb=0.18)
    s.bus("forest", gain=0.55, reverb=0.25)
    A = ["Amadd9", "Fmaj7", "C", "G/B", "Am", "Fmaj7", "Dm9", "E7sus4"]
    B = ["Dm9", "Am", "E7sus4", "Fmaj7#11", "Dm9", "Am/C", "Bm7b5", "E7"]
    melA = phrase("E5/.5 A5/.5 B5/.5 C6/2.5 r/.5 E6/.5 C6/.5 B5/.5 A5/2 G5/.5 C6/.5 D6/.5 E6/1.5 G6/1 "
                  "D6/2 B5/1 G5/1 E5/.5 A5/.5 B5/.5 C6/1.5 E6/1 F6/1.5 E6/.5 C6/1 A5/1 "
                  "D6/1 E6/.5 F6/.5 A6/1 E6/1 D6/1 B5/1 G#5/2", 8)
    melB = phrase("F5/.5 A5/.5 C6/.5 E6/.5 D6/1 A5/1 C6/.5 B5/.5 A5/.5 E5/.5 A5/2 B5/1 D6/1 G#5/1 B5/1 "
                  "C6/1.5 B5/.5 A5/1 E5/1 F5/.5 A5/.5 C6/.5 E6/.5 F6/1 E6/1 E6/.5 D6/.5 C6/.5 B5/.5 A5/2 "
                  "D6/1 F6/1 B5/1 D6/1 E6/2 G#5/2", 8)
    ost = [0, 2, 4, 2, 5, 4, 2, 4]
    s.add("forest", ms.texture_forest(s.time(44), 0.3, seed=41), 0)
    # 1-4: clareira (kalimba e pássaros)
    pads(s, "pad", A, section(0, 4), vel=0.4, bright=1600, attack=1.8)
    arpeggio(s, "kalimba", A, section(0, 4), inst_kalimba, ost, step=0.5, low=N("A4"), high=N("E6"), vel=0.45)
    # 5-20: o tema na madeira
    for rep, start in enumerate((4, 12)):
        bars = section(start, 8)
        pads(s, "pad", A, bars, vel=0.36, bright=2200)
        arpeggio(s, "kalimba", A, bars, inst_kalimba, ost, step=0.5, low=N("A4"), high=N("E6"), vel=0.42)
        arpeggio(s, "pizz", A, bars, ms.inst_pluck, [0, None, 2, None, 1, None, 3, None], step=0.5,
                 low=N("A2"), high=N("A4"), vel=0.5, bright=0.35, decay=0.993)
        bassline(s, "bass", A, bars, [(0, 2, 0), (2.5, 1.5, 7)], kind="sub", vel=0.62)
        for bar in bars:
            for b, v in ((0, 0.8), (1.5, 0.5), (2.5, 0.6), (3.25, 0.4)):
                s.hit("drums", "frame", lambda: ms.drum_frame(0.7, 140), bar, b, v, pan=-0.2)
            for b in (1, 3):
                s.hit("drums", "frame_hi", lambda: ms.drum_frame(0.6, 260), bar, b, 0.45, pan=0.3)
            for b in [i * 0.5 for i in range(8)]:
                s.hit("drums", "shaker", lambda: ms.drum_shaker(0.4, 0.07), bar, b, 0.35 if b % 1 else 0.5, pan=0.35)
        s.melody("wood", inst_wood_flute, melA, start, vel=0.6 if rep == 0 else 0.66)
        if rep:
            s.melody("celesta", ms.inst_bell, melA, start, transpose=12, vel=0.14, ratio=4.0, decay=1.2)
    # 21-28: curiosidade (pizzicato saltitante + celesta)
    bars = section(20, 8)
    pads(s, "pad", B, bars, vel=0.4, bright=2400)
    arpeggio(s, "pizz", B, bars, ms.inst_pluck, [0, 2, 1, 3, 2, 4, 3, 5], step=0.5, low=N("D3"), high=N("D5"),
             vel=0.5, bright=0.45, decay=0.992)
    bassline(s, "bass", B, bars, [(0, 1, 0), (1.5, 1, 7), (3, 1, 12)], kind="sub", vel=0.6)
    s.melody("celesta", ms.inst_bell, melB, 20, vel=0.4, ratio=4.0, decay=1.6)
    s.melody("wood", inst_wood_flute, "A4/4 A4/4 G#4/4 A4/4 A4/4 G4/4 F4/4 E4/4", 20, vel=0.4)
    for bar in bars:
        for b in (0, 2.5):
            s.hit("drums", "frame", lambda: ms.drum_frame(0.7, 140), bar, b, 0.6, pan=-0.2)
        s.hit("drums", "wood", lambda: ms.drum_wood(0.6, 700), bar, 1.5, 0.4, pan=0.4)
        s.hit("drums", "wood", lambda: ms.drum_wood(0.6, 520), bar, 3.5, 0.35, pan=-0.4)
    # 29-40: floresta viva (tudo junto, tema em oitavas)
    bars = section(28, 12)
    pads(s, "pad", A, bars, vel=0.38, bright=2600)
    arpeggio(s, "kalimba", A, bars, inst_kalimba, ost, step=0.5, low=N("A4"), high=N("E6"), vel=0.4)
    arpeggio(s, "pizz", A, bars, ms.inst_pluck, [0, 2, 1, 3, 0, 2, 1, 3], step=0.5, low=N("A2"), high=N("A4"),
             vel=0.48, bright=0.4, decay=0.993)
    bassline(s, "bass", A, bars, [(0, 1.5, 0), (1.5, 1, 7), (2.5, 1.5, 12)], kind="sub", vel=0.62)
    for bar in bars:
        for b, v in ((0, 0.85), (1.5, 0.5), (2, 0.4), (2.5, 0.65), (3.25, 0.45)):
            s.hit("drums", "frame", lambda: ms.drum_frame(0.7, 140), bar, b, v, pan=-0.2)
        for b in (1, 3):
            s.hit("drums", "frame_hi", lambda: ms.drum_frame(0.6, 260), bar, b, 0.5, pan=0.3)
        for b in [i * 0.25 for i in range(16)]:
            s.hit("drums", "shaker", lambda: ms.drum_shaker(0.4, 0.06), bar, b, 0.3 if b % 0.5 else 0.42, pan=0.35)
    s.melody("wood", inst_wood_flute, melA, 28, vel=0.68)
    s.melody("celesta", ms.inst_bell, melA, 28, transpose=12, vel=0.18, ratio=4.0, decay=1.2)
    s.melody("wood", inst_wood_flute, "E5/4 F5/4 G5/4 G5/4", 36, vel=0.35, transpose=-12)
    # 41-44: o caminho se fecha (prepara o loop)
    bars = section(40, 4)
    pads(s, "pad", ["Fmaj7#11", "C/E", "Dm9", "E7sus4"], bars, vel=0.42, bright=1500, attack=1.4)
    arpeggio(s, "kalimba", ["Fmaj7#11", "C/E", "Dm9", "E7sus4"], bars, inst_kalimba, [0, 2, 4, 2], step=1.0,
             low=N("A4"), high=N("E6"), vel=0.38)
    return s.render(reverb_seconds=2.4, reverb_damp=6000, predelay=0.025)

# ===================================================================== GELO

def inst_tremolo(f, dur, vel=0.5):
    return ms.tremolo(ms.inst_strings(f, dur, vel, attack=0.6, bright=2600), 11.0, 0.55)


def gelo():
    """Si menor, 70 bpm. Contemplação gelada: sinos de vidro, piano esparso, shimmer e tensão sutil."""
    s = Song("gelo", 70, 32, seed=505)
    s.bus("pad", gain=0.75, reverb=0.6, hp=150, width=0.5)
    s.bus("piano", gain=0.85, reverb=0.45)
    s.bus("bells", gain=0.55, reverb=0.7, delay=0.25, delay_beats=1.5, hp=700)
    s.bus("drone", gain=0.7, lp=500)
    s.bus("strings", gain=0.6, reverb=0.5, hp=200)
    s.bus("drums", gain=0.65, reverb=0.4)
    s.bus("ice", gain=0.6, reverb=0.5)
    A = ["Bmadd9", "Gmaj7", "D/F#", "Em9", "Gmaj7#11", "F#7sus4", "Bm/D", "Em9"]
    T = ["Em9", "Cmaj7#11", "Gmaj7", "F#7", "Em9", "Cmaj7#11", "Am9", "F#7"]
    mel = phrase("F#5/1 B5/1 C#6/1 D6/1 r/1 F#6/2 D6/1 C#6/2 A5/2 B5/3 r/1 "
                 "F#5/1 B5/1 C#6/1 D6/1 E6/2 D6/1 C#6/1 D6/1 C#6/1 B5/1 F#5/1 B5/4", 8)
    s.add("ice", ms.texture_ice(s.time(32), 0.5, seed=51), 0)
    s.add("drone", ms.biquad(ms.inst_pad(float(ms.hz(N("B1"))), s.length - 2, 0.5, bright=380, attack=4, release=2), "lp", 300), 0)
    # 1-8: paisagem congelada (sinos e pads)
    pads(s, "pad", A, section(0, 8), vel=0.45, bright=2000, attack=2.5, release=3.0)
    s.melody("bells", ms.inst_glass, mel, 0, transpose=12, vel=0.35)
    # 9-16: piano canta o motivo, mínimo e marcante
    bars = section(8, 8)
    pads(s, "pad", A, bars, vel=0.4, bright=2400, attack=2.0, release=3.0)
    s.melody("piano", ms.inst_piano, mel, 8, vel=0.55)
    for i, bar in enumerate(bars):
        s.chord_notes("piano", ms.inst_piano, voicing(A[i], N("B2"), N("F#4"), spread=True), bar, 0, 3.5, 0.3, strum=0.05)
    # 17-24: a nevasca se aproxima (tensão crescente)
    bars = section(16, 8)
    pads(s, "pad", T, bars, vel=0.38, bright=1500, attack=1.5)
    pads(s, "strings", T, bars, inst=inst_tremolo, low=N("E4"), high=N("B5"), vel=0.42)
    for i, bar in enumerate(bars):
        for b in (0, 1.5, 2, 3.5):
            s.note("drone", ms.inst_bass, bass_of(T[i]), bar, b, 0.45, 0.55 + 0.04 * i, kind="sub")
        s.hit("drums", "taiko", lambda: ms.drum_taiko(0.7, 55, 1.4), bar, 0, 0.35 + 0.05 * i)
        if i >= 4:
            s.hit("drums", "taiko", lambda: ms.drum_taiko(0.7, 55, 1.4), bar, 2.5, 0.3 + 0.05 * i)
        for b in [k * 0.25 for k in range(16)]:  # "tique" de cristal, como gelo estalando
            if (b * 4 + i) % 3 != 1:
                s.hit("drums", "tick", lambda: ms.drum_hat(0.5, tone=11000), bar, b, 0.2 + 0.02 * i, pan=0.5 * np.sin(b * 3))
    s.melody("bells", ms.inst_glass, "B6/2 r/2 C7/2 r/2 B6/1 G6/1 D6/2 r/4 B6/2 r/2 C7/2 r/2 A6/1 F#6/1 C#6/2 r/4", 16, vel=0.32)
    s.add("drums", ms.cymbal(0.5, 4.0, reverse=True), s.time(24) - 4.0)
    # 25-32: o céu limpa (motivo no agudo, retorno à calma)
    bars = section(24, 8)
    pads(s, "pad", A, bars, vel=0.42, bright=2600, attack=2.0, release=3.0)
    s.melody("bells", ms.inst_glass, mel, 24, transpose=12, vel=0.42)
    s.melody("piano", ms.inst_piano, mel, 24, vel=0.4, transpose=-12)
    s.melody("strings", ms.inst_strings, "D5/8 C#5/8 B4/8 B4/8", 24, vel=0.28)
    return s.render(reverb_seconds=5.0, reverb_damp=8000, shimmer=0.35, predelay=0.04)

# ===================================================================== TERRA

def inst_guitar(f, dur, vel=0.6):
    return ms.inst_pluck(f, dur, vel, bright=0.55, decay=0.995, body=True)


def terra_drums(s, bars, full=True, vel=1.0):
    for bar in bars:
        s.hit("drums", "taiko", lambda: ms.drum_taiko(0.9, 62, 1.0), bar, 0, 0.9 * vel)
        s.hit("drums", "taiko", lambda: ms.drum_taiko(0.9, 62, 1.0), bar, 2.5, 0.65 * vel)
        s.hit("drums", "tom_mid", lambda: ms.drum_tom(0.8, 140, 0.4), bar, 1.5, 0.4 * vel, pan=-0.3)
        s.hit("drums", "tom_hi", lambda: ms.drum_tom(0.8, 210, 0.3), bar, 3.25, 0.35 * vel, pan=0.3)
        if full:
            s.hit("drums", "kick", lambda: ms.drum_kick(0.9, 50, 0.8, 0.35), bar, 0, 0.6 * vel)
            s.hit("drums", "kick", lambda: ms.drum_kick(0.9, 50, 0.8, 0.35), bar, 1.75, 0.45 * vel)
            s.hit("drums", "clap", lambda: ms.drum_clap(0.7), bar, 1, 0.45 * vel, pan=-0.1)
            s.hit("drums", "clap", lambda: ms.drum_clap(0.7), bar, 3, 0.5 * vel, pan=0.1)
            for b in [k * 0.25 for k in range(16)]:
                s.hit("drums", "shaker", lambda: ms.drum_shaker(0.5, 0.05), bar, b, (0.42 if b % 0.5 == 0 else 0.25) * vel, pan=0.4)


def terra():
    """Ré dórico, 112 bpm. Aventura em paisagens vastas: tambores, violão, trompas híbridas, areia."""
    s = Song("terra", 112, 48, seed=606)
    s.bus("pad", gain=0.6, reverb=0.4, hp=200, lp=3500)
    s.bus("guitar", gain=0.62, reverb=0.25, delay=0.12, delay_beats=0.75, hp=150)
    s.bus("horn", gain=0.9, reverb=0.35, delay=0.1, delay_beats=1.5)
    s.bus("bass", gain=0.85, lp=1600, duck=0.3)
    s.bus("drums", gain=0.82, reverb=0.2)
    s.bus("sand", gain=0.5, reverb=0.2)
    s.bus("strings", gain=0.55, reverb=0.4, hp=200)
    A = ["Dm7", "Cadd9", "G/B", "Csus4", "Dm7", "Fmaj7", "C/E", "A7sus4"]
    B = ["Bbmaj7", "C", "Dm", "Am7", "Bbmaj7", "C", "Gm7", "A7"]
    melA = phrase("A4/.5 D5/.5 E5/.5 F5/2.5 r/.5 G5/.5 E5/.5 C5/.5 E5/2 D5/.5 G5/.5 A5/.5 B5/2.5 "
                  "C6/1 B5/.5 G5/.5 E5/2 A4/.5 D5/.5 E5/.5 F5/1.5 A5/1 C6/1.5 A5/.5 F5/1 A5/1 "
                  "G5/1 E5/1 C5/1 E5/1 D5/2 C#5/2", 8)
    melB = phrase("F5/1 Bb5/1 D6/2 C6/1 G5/1 E5/2 F5/1 A5/1 D6/2 E6/2 C6/1 A5/1 "
                  "D6/1 F6/1 D6/1 Bb5/1 C6/1 E6/1 G6/2 F6/1 D6/1 Bb5/1 G5/1 A5/2 C#6/2", 8)
    guitar_pat = [0, 2, 1, 3, 2, 4, 3, 2, 0, 2, 1, 3, 2, 4, 5, 4]
    s.add("sand", ms.texture_sand(s.time(48), 0.3, seed=61), 0)
    # 1-4: o chão treme (tambores chamam)
    terra_drums(s, section(0, 4), full=False, vel=0.9)
    pads(s, "pad", A, section(0, 4), vel=0.35, bright=1500, attack=1.5)
    s.add("drums", ms.riser(s.spb * 4, 0.4, 200, 5000), s.time(3))
    # 5-20: jornada (violão + tambores + baixo; trompas a partir da repetição)
    for rep, start in enumerate((4, 12)):
        bars = section(start, 8)
        terra_drums(s, bars, full=True, vel=0.9 + 0.1 * rep)
        arpeggio(s, "guitar", A, bars, inst_guitar, guitar_pat, step=0.25, low=N("D3"), high=N("D5"), vel=0.42, length=0.4)
        bassline(s, "bass", A, bars, [(0, 0.75, 0), (0.75, 0.75, 0), (1.5, 0.5, 12), (2, 0.75, 0), (2.75, 0.75, 7), (3.5, 0.5, 10)],
                 kind="pluck", bright=600, vel=0.65)
        pads(s, "pad", A, bars, vel=0.3, bright=2000)
        s.melody("horn", ms.inst_brass, melA, start, vel=0.55 if rep == 0 else 0.62, bright=0.8 if rep == 0 else 1.0)
    # 21-28: horizonte (melodia heroica, cordas)
    bars = section(20, 8)
    s.add("drums", ms.cymbal(0.55, 3.0), s.time(20))
    terra_drums(s, bars, full=True)
    arpeggio(s, "guitar", B, bars, inst_guitar, guitar_pat, step=0.25, low=N("D3"), high=N("D5"), vel=0.38, length=0.4)
    bassline(s, "bass", B, bars, [(0, 1.5, 0), (1.5, 1, 7), (2.5, 1.5, 12)], kind="sub", vel=0.7)
    pads(s, "strings", B, bars, inst=ms.inst_strings, low=N("D4"), high=N("A5"), vel=0.42)
    s.melody("horn", ms.inst_brass, melB, 20, vel=0.66)
    s.melody("horn", ms.inst_brass, melB, 20, transpose=-12, vel=0.32, bright=0.6)
    # 29-44: tutti com o motivo em contracanto
    bars = section(28, 16)
    s.add("drums", ms.cymbal(0.6, 3.0), s.time(28))
    terra_drums(s, bars, full=True)
    arpeggio(s, "guitar", A, bars, inst_guitar, guitar_pat, step=0.25, low=N("D3"), high=N("D5"), vel=0.42, length=0.4)
    bassline(s, "bass", A, bars, [(0, 0.75, 0), (0.75, 0.75, 0), (1.5, 0.5, 12), (2, 0.75, 0), (2.75, 0.75, 7), (3.5, 0.5, 10)],
             kind="pluck", bright=700, vel=0.68)
    pads(s, "strings", A, bars, inst=ms.inst_strings, low=N("A3"), high=N("F5"), vel=0.34)
    s.melody("horn", ms.inst_brass, melA, 28, vel=0.66)
    s.melody("horn", ms.inst_brass, melA, 36, vel=0.6, transpose=12, bright=0.7)
    s.melody("strings", ms.inst_strings, "F4/4 E4/4 D4/4 E4/4 F4/4 A4/4 G4/4 E4/4", 36, vel=0.35)
    for bar in (35, 43):
        for k, b in enumerate((2, 2.5, 3, 3.5)):
            s.hit("drums", "tom_mid", lambda: ms.drum_tom(0.8, 140, 0.4), bar, b, 0.55 + 0.1 * k, pan=0.4 - 0.25 * k)
    # 45-48: poeira assenta
    bars = section(44, 4)
    terra_drums(s, bars, full=False, vel=0.7)
    pads(s, "pad", ["Bbmaj7", "C", "Dm7", "A7sus4"], bars, vel=0.4, bright=1400, attack=1.0)
    s.melody("horn", ms.inst_brass, "D5/4 F5/4 A5/4 E5/4", 44, vel=0.4, bright=0.5)
    return s.render(reverb_seconds=1.9, reverb_damp=5500, predelay=0.02)

# ===================================================================== FOGO

def inst_stab(f, dur, vel=0.7):
    n = ms.samples(min(dur, 0.5) + 0.2)
    t = np.arange(n) / ms.SR
    x = ms.supersaw(f, n, 5, 16)
    x = ms.sweep(x, "lp", 500 + 4500 * np.exp(-t / 0.08), q=1.2)
    x = ms.saturate(x * 1.5, 2.2)
    return (x.T * ms.adsr(n, 0.003, 0.15, 0.4, 0.12, hold=min(dur, 0.5))).T * vel * 0.32


def fogo_drums(s, bars, mode="full", vel=1.0):
    for bar in bars:
        if mode == "full":
            for b in (0, 1, 2, 3):
                s.hit("drums", "kick_hard", lambda: ms.drum_kick(1.0, 48, 1.2, 0.3, 1.2), bar, b, 0.9 * vel)
            for b in (1, 3):
                s.hit("drums", "snare_hard", lambda: ms.drum_snare(1.0, 185, 1.2, 0.25), bar, b, 0.75 * vel)
                s.hit("drums", "clap", lambda: ms.drum_clap(0.8), bar, b, 0.4 * vel, pan=0.15)
            for b in [k * 0.5 + 0.5 for k in range(4)] + [k * 0.5 for k in range(4, 8)]:
                s.hit("drums", "hat", lambda: ms.drum_hat(0.6), bar, b, (0.5 if b % 1 else 0.32) * vel, pan=0.3)
            s.hit("drums", "hat_open", lambda: ms.drum_hat(0.6, open_=True), bar, 3.5, 0.3 * vel, pan=-0.3)
        elif mode == "half":
            s.hit("drums", "kick_hard", lambda: ms.drum_kick(1.0, 48, 1.2, 0.3, 1.2), bar, 0, 0.85 * vel)
            s.hit("drums", "snare_hard", lambda: ms.drum_snare(1.0, 185, 1.2, 0.3), bar, 2, 0.7 * vel)
            s.hit("drums", "taiko", lambda: ms.drum_taiko(0.9, 58, 1.0), bar, 3.25, 0.5 * vel)
        else:  # toms da introdução
            for b in (0, 0.75, 1.5, 2.5, 3.25):
                s.hit("drums", "tom_low", lambda: ms.drum_tom(0.9, 85, 0.5), bar, b, (0.7 if b == 0 else 0.45) * vel, pan=-0.2)


def fogo():
    """Dó menor, 132 bpm. Urgência e perigo: percussão intensa, reese, stabs, cordas e metais híbridos."""
    s = Song("fogo", 132, 56, seed=707)
    s.bus("strings", gain=0.6, reverb=0.3, hp=180, duck=0.3)
    s.bus("stabs", gain=0.55, reverb=0.2, hp=200, duck=0.45)
    s.bus("brass", gain=0.85, reverb=0.3, duck=0.2)
    s.bus("lead", gain=0.85, reverb=0.25, delay=0.12, delay_beats=0.75, drive=1.2)
    s.bus("bass", gain=0.8, lp=2200, duck=0.55)
    s.bus("choir", gain=0.65, reverb=0.55, hp=200)
    s.bus("drums", gain=0.85, reverb=0.12)
    s.bus("fire", gain=0.45, reverb=0.2)
    A = ["Cm", "Ab", "Eb", "Bb", "Cm", "Abmaj7", "Fm9", "G7sus4"]
    B = ["Fm9", "Abmaj7", "Bb", "Cm", "Fm9", "Db", "Bb", "G"]
    melA = phrase("G4/.5 C5/.5 D5/.5 Eb5/2.5 r/.5 Ab5/.5 G5/.5 Eb5/.5 C5/2 Bb4/.5 Eb5/.5 F5/.5 G5/2.5 "
                  "F5/1 D5/1 Bb4/2 G4/.5 C5/.5 D5/.5 Eb5/1.5 G5/1 Ab5/1.5 G5/.5 Eb5/1 C5/1 "
                  "F5/1 G5/.5 Ab5/.5 C6/1 Ab5/1 G5/2 B4/2", 8)
    melB = phrase("C6/2 Ab5/2 Eb6/2 C6/2 D6/2 Bb5/2 G5/4 C6/2 F6/2 F6/1 Eb6/1 Db6/2 D6/2 F6/2 G6/2 B5/2", 8)
    spic = [0, 0, 2, 0, 1, 0, 2, 3]
    s.add("fire", ms.texture_crackle(s.time(56), 0.35, density=18, seed=71), 0)
    # 1-8: brasa (toms, cordas em ostinato crescendo)
    fogo_drums(s, section(0, 8), mode="toms", vel=0.85)
    for i, bar in enumerate(section(0, 8)):
        arpeggio(s, "strings", A, [bar], ms.inst_spiccato, spic, step=0.5, low=N("C4"), high=N("C5"), vel=0.35 + 0.04 * i)
        s.note("bass", ms.inst_bass, bass_of(A[i]), bar, 0, 3.5, 0.5 + 0.03 * i, kind="reese", bright=500)
    s.melody("brass", ms.inst_brass, "G3/4 Ab3/4 G3/4 F3/4 G3/4 Ab3/4 Bb3/4 B3/4", 0, vel=0.45, bright=0.6)
    s.add("drums", ms.riser(s.spb * 8, 0.55, 200, 9000), s.time(6))
    # 9-24: avanço (tema com tudo)
    s.add("drums", ms.cymbal(0.7, 3.0), s.time(8))
    for rep, start in enumerate((8, 16)):
        bars = section(start, 8)
        fogo_drums(s, bars, mode="full", vel=0.9 + 0.1 * rep)
        arpeggio(s, "strings", A, bars, ms.inst_spiccato, spic, step=0.5, low=N("C4"), high=N("C5"), vel=0.45)
        bassline(s, "bass", A, bars, [(0, 0.5, 0), (0.75, 0.5, 0), (1.5, 0.5, 12), (2, 0.5, 0), (2.75, 0.5, 0), (3.5, 0.5, 10)],
                 kind="reese", bright=900, vel=0.7)
        for i, bar in enumerate(bars):
            notes = voicing(A[i], N("G3"), N("G4"))
            for b in (0, 1.5, 3):
                s.chord_notes("stabs", inst_stab, notes, bar, b, 0.4, 0.5)
        s.melody("lead", ms.inst_brass, melA, start, vel=0.66, bright=1.1)
        if rep:
            s.melody("lead", ms.inst_lead, melA, start, transpose=12, vel=0.3, bright=4200, vibrato=0.004)
    # 25-32: a montanha ruge (meio tempo, coro)
    bars = section(24, 8)
    s.add("drums", ms.cymbal(0.6, 4.0), s.time(24))
    fogo_drums(s, bars, mode="half")
    pads(s, "choir", B, bars, inst=ms.inst_choir, low=N("C4"), high=N("Eb5"), vel=0.6)
    bassline(s, "bass", B, bars, [(0, 3.5, 0)], kind="reese", bright=600, vel=0.65)
    s.melody("brass", ms.inst_brass, melB, 24, vel=0.6)
    arpeggio(s, "strings", B, bars, ms.inst_spiccato, [0, 2, 4, 2], step=1.0, low=N("C4"), high=N("C6"), vel=0.35)
    s.add("drums", ms.riser(s.spb * 8, 0.6, 150, 10000), s.time(30))
    # 33-48: clímax (contracanto e tensão máxima)
    bars = section(32, 16)
    s.add("drums", ms.cymbal(0.75, 3.0), s.time(32))
    fogo_drums(s, bars, mode="full")
    arpeggio(s, "strings", A, bars, ms.inst_spiccato, [0, 2, 1, 3, 0, 2, 1, 3, 0, 2, 1, 3, 2, 4, 3, 2], step=0.25,
             low=N("C4"), high=N("C6"), vel=0.4)
    bassline(s, "bass", A, bars, [(0, 0.5, 0), (0.75, 0.5, 0), (1.5, 0.5, 12), (2, 0.5, 0), (2.75, 0.5, 0), (3.5, 0.5, 10)],
             kind="reese", bright=1100, vel=0.72)
    pads(s, "choir", A, bars, inst=ms.inst_choir, low=N("C4"), high=N("Eb5"), vel=0.42, vowel="o")
    for i, bar in enumerate(bars):
        notes = voicing(A[i % 8], N("G3"), N("G4"))
        for b in (0, 0.75, 1.5, 3):
            s.chord_notes("stabs", inst_stab, notes, bar, b, 0.35, 0.5)
    s.melody("lead", ms.inst_brass, melA, 32, vel=0.7, bright=1.2)
    s.melody("lead", ms.inst_brass, melA, 40, vel=0.66, bright=1.2, transpose=12)
    s.melody("brass", ms.inst_brass, "Eb4/4 C4/4 Bb3/4 D4/4 Eb4/4 C4/4 C4/4 B3/4", 40, vel=0.5)
    # 49-56: nova onda de calor (volta ao início sem emenda)
    bars = section(48, 8)
    fogo_drums(s, bars, mode="toms", vel=0.9)
    for i, bar in enumerate(bars):
        arpeggio(s, "strings", A, [bar], ms.inst_spiccato, spic, step=0.5, low=N("C4"), high=N("C5"), vel=0.5 - 0.02 * i)
        s.note("bass", ms.inst_bass, bass_of(A[i]), bar, 0, 3.5, 0.6 - 0.02 * i, kind="reese", bright=500)
    s.melody("brass", ms.inst_brass, "C4/4 Ab3/4 Eb4/4 D4/4 C4/4 Ab3/4 F3/4 G3/4", 48, vel=0.45, bright=0.6)
    return s.render(reverb_seconds=2.0, reverb_damp=5000, predelay=0.015)

# ===================================================================== GROOTSPECTRO

GROOT = "E3/1 F3/.5 E3/.5 Bb2/1 A2/1"   # 1 - b2 - 1 - #4 - 4: o motivo do Titã Espectral


def _boss_song(seed):
    s = Song("boss", 128, 48, seed=seed)
    for name, kw in (("choir", dict(gain=0.6, reverb=0.55, hp=150)), ("low", dict(gain=0.75, lp=900)),
                     ("brass", dict(gain=0.8, reverb=0.4)), ("tick", dict(gain=0.5, reverb=0.25, hp=800)),
                     ("drums", dict(gain=0.85, reverb=0.15)), ("bass", dict(gain=0.8, lp=2000, drive=1.3)),
                     ("strings", dict(gain=0.6, reverb=0.3, hp=180)), ("lead", dict(gain=0.85, reverb=0.3, delay=0.1, delay_beats=0.75)),
                     ("fx", dict(gain=0.6, reverb=0.4))):
        s.bus(name, **kw)
    return s


def boss():
    """Mi menor, 128 bpm, 48 compassos. Três camadas sincronizadas (fase 1, +2, +3)."""
    A = ["Em", "Cmaj7", "Am9", "B7b9", "Em", "C", "F", "B7"]
    B = ["Cmaj7#11", "Am9", "Fmaj7#11", "B7sus4", "Em", "Cmaj7", "F", "B7"]
    prog = lambda bar: (B if 16 <= bar < 32 else A)[bar % 8]
    hero = phrase("B4/.5 E5/.5 F#5/.5 G5/2.5 r/.5 B5/.5 G5/.5 F#5/.5 E5/2 C5/.5 E5/.5 G5/.5 A5/2.5 B5/1 A5/1 F#5/1 D#5/1 "
                  "B4/.5 E5/.5 F#5/.5 G5/1.5 B5/1 C6/1.5 B5/.5 G5/1 E5/1 F5/1 A5/1 C6/1 A5/1 B5/2 D#5/2", 8)
    # Camada 1: ameaça controlada
    s1 = _boss_song(808)
    for bar in range(48):
        sym = prog(bar)
        s1.chord_notes("choir", ms.inst_choir, voicing(sym, N("B3"), N("E5")), bar, 0, 4, 0.5)
        s1.note("low", ms.inst_strings, bass_of(sym), bar, 0, 4, 0.6, attack=0.2)
        for b in (0, 1, 2, 3):
            s1.note("low", ms.inst_bass, bass_of(sym), bar, b, 0.6, 0.55, kind="sub")
        for b in [k * 0.5 for k in range(8)]:
            s1.hit("tick", "tick", lambda: ms.drum_wood(0.5, 2200), bar, b, 0.5 if b % 1 == 0 else 0.3, pan=0.4 if b % 1 else -0.4)
        if bar % 2 == 0:
            shift = -4 if 16 <= bar < 32 else 0   # na seção B o motivo desce para Dó
            s1.melody("brass", ms.inst_brass, GROOT, bar, vel=0.6, transpose=shift, bright=0.7)
            s1.melody("brass", ms.inst_bell, GROOT, bar, vel=0.25, transpose=shift + 24, ratio=1.41, decay=2.5)
        if bar % 8 == 7:
            s1.add("fx", ms.cymbal(0.5, s1.spb * 4, reverse=True), s1.time(bar))
    # Camada 2: a batalha esquenta
    s2 = _boss_song(809)
    for bar in range(48):
        sym = prog(bar)
        s2.hit("drums", "taiko", lambda: ms.drum_taiko(1.0, 58, 1.1), bar, 0, 0.9)
        s2.hit("drums", "taiko", lambda: ms.drum_taiko(1.0, 58, 1.1), bar, 1.75, 0.55)
        s2.hit("drums", "kick", lambda: ms.drum_kick(1.0, 46, 1.1, 0.3), bar, 2.5, 0.7)
        for b in (1, 3):
            s2.hit("drums", "snare", lambda: ms.drum_snare(1.0, 175, 1.1, 0.28), bar, b, 0.7)
        if bar % 4 == 3:
            for k, b in enumerate((2, 2.5, 3, 3.25, 3.5, 3.75)):
                s2.hit("drums", "tom", lambda: ms.drum_tom(0.9, 95, 0.4), bar, b, 0.45 + 0.07 * k, pan=0.4 - 0.15 * k)
        for b, iv in ((0, 0), (0.5, 0), (1, 12), (1.5, 0), (2, 0), (2.5, 1), (3, 0), (3.5, 7)):
            s2.note("bass", ms.inst_bass, bass_of(sym) + iv, bar, b, 0.42, 0.62, kind="reese", bright=900)
        arpeggio(s2, "strings", [sym], [bar], ms.inst_spiccato, [0, 0, 1, 0, 2, 0, 1, 3], step=0.5, low=N("E4"), high=N("E5"), vel=0.45)
    # Camada 3: urgência máxima (herói contra vilão)
    s3 = _boss_song(810)
    for bar in range(48):
        sym = prog(bar)
        arpeggio(s3, "strings", [sym], [bar], ms.inst_spiccato, [0, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 4, 3], step=0.25,
                 low=N("E5"), high=N("E6"), vel=0.4)
        for b in [k * 0.25 for k in range(16)]:
            s3.hit("drums", "hat", lambda: ms.drum_hat(0.6), bar, b, 0.35 if b % 0.5 else 0.5, pan=0.3)
        if bar % 4 == 0:
            s3.add("fx", ms.cymbal(0.7, 2.5), s3.time(bar))
        s3.chord_notes("choir", ms.inst_choir, voicing(sym, N("E5"), N("B5")), bar, 0, 4, 0.35, vowel="o")
    for start in (0, 8, 32, 40):
        s3.melody("lead", ms.inst_brass, hero, start, vel=0.62, bright=1.2)
    s3.melody("lead", ms.inst_lead, hero, 40, transpose=12, vel=0.25, bright=4000)
    s3.melody("lead", ms.inst_brass, "E5/4 G5/4 A5/4 F#5/4 B5/4 A5/4 G5/4 F#5/4 E5/4 G5/4 A5/4 B5/4 B5/4 C6/4 C6/4 D#6/4", 16, vel=0.5)
    stems = {}
    # Equilíbrio entre camadas: a fase 1 sozinha precisa soar ameaçadora (não "vazia")
    # logo após o Fogo; cada fase seguinte soma ~1,5-3 dB de intensidade.
    balance = {"boss_1": ms.db(5.0), "boss_2": ms.db(-0.5), "boss_3": ms.db(1.5)}
    for name, s in (("boss_1", s1), ("boss_2", s2), ("boss_3", s3)):
        stems[name] = s.render(reverb_seconds=3.0, reverb_damp=5500, master=False) * balance[name]
    # ganho único para as três camadas: a soma completa fica em -16 LUFS sem estourar
    total = stems["boss_1"] + stems["boss_2"] + stems["boss_3"]
    import export
    gain = export.common_gain(total, lufs=-15.5, ceiling_db=-1.5)
    return {k: (v, {"fixed_gain_db": gain}) for k, v in stems.items()}

# ===================================================================== VITÓRIA E DERROTA

def vitoria():
    """Ré maior, 100 bpm. Resolução emocionante da jornada (toca uma vez)."""
    s = Song("vitoria", 100, 16, seed=909, loop=False, tail=6.0)
    s.bus("strings", gain=0.8, reverb=0.5, hp=150)
    s.bus("choir", gain=0.6, reverb=0.6, hp=180)
    s.bus("lead", gain=0.95, reverb=0.35, delay=0.12, delay_beats=1.5)
    s.bus("piano", gain=0.7, reverb=0.4)
    s.bus("bells", gain=0.5, reverb=0.6, hp=600)
    s.bus("bass", gain=0.8, lp=1500)
    s.bus("drums", gain=0.75, reverb=0.3)
    P = ["Dsus2", "D", "Gmaj7", "A", "Bm", "G", "Em7", "A", "D", "G/D", "Bm", "Gmaj9", "Em9", "A7sus4", "D", "D"]
    mel = phrase("A4/.5 D5/.5 E5/.5 F#5/2.5 r/.5 A5/.5 F#5/.5 E5/.5 D5/2 B4/.5 D5/.5 E5/.5 G5/2.5 "
                 "F#5/1 E5/1 C#5/1 A4/1 A4/.5 D5/.5 E5/.5 F#5/1.5 A5/1 B5/1.5 A5/.5 G5/1 F#5/1 "
                 "E5/1 F#5/.5 G5/.5 B5/1 A5/1 G5/.5 F#5/.5 E5/1 A5/2 "
                 "B5/2 A5/2 G5/1 A5/1 B5/2 E5/1 F#5/1 G5/1 B5/1 A5/2 C#6/2 D6/4 D6/4", 14)
    s.add("drums", ms.drum_taiko(1.0, 55, 2.0), 0)
    s.add("drums", ms.cymbal(0.7, 4.0), 0)
    for bar, sym in enumerate(P):
        s.chord_notes("strings", ms.inst_strings, voicing(sym, N("D4"), N("A5")), bar, 0, 4, 0.5 if bar < 2 else 0.6)
        s.chord_notes("choir", ms.inst_choir, voicing(sym, N("A3"), N("F#5")), bar, 0, 4, 0.35 if bar < 8 else 0.5)
        s.note("bass", ms.inst_bass, bass_of(sym), bar, 0, 4, 0.6, kind="sub")
        s.chord_notes("piano", ms.inst_piano, voicing(sym, N("D3"), N("A4")), bar, 0, 2, 0.35, strum=0.04)
        if 2 <= bar < 14:
            s.hit("drums", "taiko", lambda: ms.drum_taiko(0.8, 60, 1.2), bar, 0, 0.5)
            s.hit("drums", "snare_soft", lambda: ms.drum_snare(0.6, 200, 0.6, 0.3), bar, 2, 0.3)
    # compassos 1-2: impacto e respiro; o motivo entra no 3º e resolve no 16º
    s.melody("lead", ms.inst_brass, mel, 2, vel=0.62, bright=1.1)
    s.melody("bells", ms.inst_bell, mel, 2, transpose=12, vel=0.18, ratio=3.0, decay=1.6)
    s.melody("bells", ms.inst_glass, "D6/2 F#6/2 A6/4 D7/8", 13, vel=0.35)
    s.add("drums", ms.cymbal(0.6, 5.0), s.time(14))
    return s.render(reverb_seconds=3.2, reverb_damp=8000), {"loop": False}


def derrota():
    """Ré menor -> Ré maior, 72 bpm. Melancolia breve que convida a tentar de novo."""
    s = Song("derrota", 72, 6, seed=1010, loop=False, tail=5.0)
    s.bus("piano", gain=0.85, reverb=0.5)
    s.bus("pad", gain=0.6, reverb=0.6, hp=150, lp=2500)
    s.bus("bells", gain=0.4, reverb=0.7, hp=600)
    P = ["Dm", "Bbmaj7", "Gm6", "A7sus4", "Bbmaj7", "D"]
    mel = phrase("A4/.5 D5/.5 E5/.5 F5/2.5 r/.5 F5/.5 D5/.5 C5/.5 Bb4/2 A4/1 G4/1 E4/2 D4/2 C#4/2 F4/2 G4/2 F#4/4", 6)
    for bar, sym in enumerate(P):
        s.chord_notes("pad", ms.inst_pad, voicing(sym, N("D3"), N("A4")), bar, 0, 4, 0.4, bright=1200, attack=1.0)
        s.chord_notes("piano", ms.inst_piano, voicing(sym, N("D3"), N("F4")), bar, 0, 4, 0.3, strum=0.06)
    s.melody("piano", ms.inst_piano, mel, 0, vel=0.5)
    s.melody("bells", ms.inst_glass, "A5/16 F#6/8", 0, vel=0.2)
    return s.render(reverb_seconds=3.5, reverb_damp=5000), {"loop": False}


TRACKS = {"menu": menu, "ar": ar, "agua": agua, "floresta": floresta, "gelo": gelo, "terra": terra,
          "fogo": fogo, "grootspectro": boss, "vitoria": vitoria, "derrota": derrota}
