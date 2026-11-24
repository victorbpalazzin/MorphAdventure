"""Sequenciador, teoria musical e mixagem por barramentos para as trilhas."""
import numpy as np
import morphsynth as ms

MODES = {
    "major": [0, 2, 4, 5, 7, 9, 11], "lydian": [0, 2, 4, 6, 7, 9, 11],
    "mixolydian": [0, 2, 4, 5, 7, 9, 10], "dorian": [0, 2, 3, 5, 7, 9, 10],
    "minor": [0, 2, 3, 5, 7, 8, 10], "harmonic": [0, 2, 3, 5, 7, 8, 11],
}
QUALITIES = {
    "": [0, 4, 7], "m": [0, 3, 7], "maj7": [0, 4, 7, 11], "m7": [0, 3, 7, 10], "7": [0, 4, 7, 10],
    "maj9": [0, 4, 7, 11, 14], "m9": [0, 3, 7, 10, 14], "add9": [0, 4, 7, 14], "madd9": [0, 3, 7, 14],
    "sus2": [0, 2, 7], "sus4": [0, 5, 7], "7sus4": [0, 5, 7, 10], "6": [0, 4, 7, 9], "69": [0, 4, 7, 9, 14],
    "m11": [0, 3, 7, 10, 14, 17], "maj7#11": [0, 4, 7, 11, 18], "13": [0, 4, 10, 14, 21], "9": [0, 4, 7, 10, 14],
    "7b9": [0, 4, 7, 10, 13], "m6": [0, 3, 7, 9], "dim": [0, 3, 6], "m7b5": [0, 3, 6, 10], "5": [0, 7, 12],
}


def chord(symbol):
    """'F#m9' ou 'D/F#' -> (raiz MIDI na oitava 3, intervalos, baixo MIDI)."""
    bass = None
    if "/" in symbol:
        symbol, b = symbol.split("/")
        bass = ms.note(b + "2")
    root = symbol[0]
    i = 1
    if i < len(symbol) and symbol[i] in "#b":
        root += symbol[i]
        i += 1
    quality = symbol[i:]
    r = ms.note(root + "3")
    if bass is None:
        bass = r - 12
    while bass > ms.note("E2") + 7:
        bass -= 12
    return r, QUALITIES[quality], bass


def voicing(symbol, low=ms.note("G3"), high=ms.note("E5"), spread=True):
    """Distribui as notas do acorde entre low e high (voicing aberto e suave)."""
    r, iv, _ = chord(symbol)
    notes = []
    for k in iv:
        n = r + k
        while n < low:
            n += 12
        while n > high:
            n -= 12
        notes.append(n)
    notes = sorted(set(notes))
    if spread and len(notes) >= 3:
        notes[1] += 12 if notes[1] + 12 <= high + 5 else 0
        notes = sorted(set(notes))
    return notes


def bass_of(symbol, octave_shift=0):
    return chord(symbol)[2] + 12 * octave_shift


def parse_melody(text):
    """'A4/.5 D5/.5 r/1' -> [(midi|None, beats)]."""
    out = []
    for tok in text.split():
        name, dur = tok.split("/")
        out.append((None if name == "r" else ms.note(name), float(dur)))
    return out


class Bus:
    def __init__(self, song, name, gain=1.0, pan=0.0, reverb=0.2, delay=0.0, delay_beats=0.75,
                 chorus=0.0, hp=None, lp=None, duck=0.0, width=0.0, drive=0.0):
        self.song, self.name = song, name
        self.buf = np.zeros((song.total, 2))
        self.gain, self.pan, self.reverb, self.delay = gain, pan, reverb, delay
        self.delay_beats, self.chorus, self.hp, self.lp = delay_beats, chorus, hp, lp
        self.duck, self.width, self.drive = duck, width, drive


class Song:
    def __init__(self, name, bpm, bars, beats_per_bar=4, tail=7.0, seed=1, loop=True):
        self.name, self.bpm, self.bars, self.bpb = name, bpm, bars, beats_per_bar
        self.spb = 60.0 / bpm
        self.length = bars * beats_per_bar * self.spb
        self.loop = loop
        self.total = ms.samples(self.length + tail)
        self.buses = {}
        self.kicks = []
        self.rng = np.random.default_rng(seed)
        self.cache = {}

    def bus(self, name, **kw):
        if name not in self.buses:
            self.buses[name] = Bus(self, name, **kw)
        return self.buses[name]

    def time(self, bar, beat=0.0):
        return (bar * self.bpb + beat) * self.spb

    def human(self, amount=0.006):
        return self.rng.normal(0, amount)

    def note(self, bus, inst, midi, bar, beat, beats, vel=0.7, pan=0.0, humanize=True, **kw):
        if midi is None:
            return
        t = self.time(bar, beat) + (self.human() if humanize else 0)
        v = vel * (1 + (self.rng.normal(0, 0.06) if humanize else 0))
        audio = ms.tail_fade(inst(float(ms.hz(midi)), beats * self.spb, max(0.05, min(1.0, v)), **kw))
        ms.mix_into(self.buses[bus].buf, ms.stereo(audio, pan) if audio.ndim == 1 else audio, ms.samples(max(0, t)))

    def chord_notes(self, bus, inst, notes, bar, beat, beats, vel=0.6, strum=0.0, **kw):
        for i, n in enumerate(notes):
            pan = (i - (len(notes) - 1) / 2) * 0.25
            self.note(bus, inst, n, bar, beat + i * strum, beats - i * strum, vel, pan=pan, **kw)

    def melody(self, bus, inst, text, bar, beat=0.0, vel=0.75, transpose=0, legato=1.0, accent=None, **kw):
        pos = bar * self.bpb + beat
        for m, d in parse_melody(text):
            if m is not None:
                # frase respira: notas longas um pouco mais fortes, curtas mais leves
                v = vel * (1.0 + 0.08 * min(d, 2) - 0.04) * (1.1 if accent and (pos % self.bpb) in accent else 1.0)
                self.note(bus, inst, m + transpose, 0, pos, d * legato, v, **kw)
            pos += d
        return pos

    def hit(self, bus, key, factory, bar, beat, vel=1.0, pan=0.0, variants=3):
        """Percussão com variações pré-renderizadas (evita repetição idêntica)."""
        k = (key, int(self.rng.integers(variants)))
        if k not in self.cache:
            self.cache[k] = ms.tail_fade(factory())
        t = self.time(bar, beat) + self.human(0.004)
        v = vel * (1 + self.rng.normal(0, 0.07))
        ms.mix_into(self.buses[bus].buf, ms.stereo(self.cache[k] * v, pan), ms.samples(max(0, t)))
        if key.startswith("kick"):
            self.kicks.append(self.time(bar, beat))

    def add(self, bus, audio, seconds):
        ms.mix_into(self.buses[bus].buf, ms.stereo(ms.tail_fade(audio)), ms.samples(seconds))

    # ------------------------------------------------------------ mixagem
    def render(self, reverb_seconds=2.5, reverb_damp=6500, shimmer=0.0, predelay=0.02, master=True):
        dry = np.zeros((self.total, 2))
        send_r = np.zeros((self.total, 2))
        send_d = {}
        for b in self.buses.values():
            x = b.buf
            if b.hp:
                x = ms.biquad(x, "hp", b.hp, 0.7)
            if b.lp:
                x = ms.biquad(x, "lp", b.lp, 0.7)
            if b.chorus > 0:
                x = ms.chorus(x, mix=b.chorus)
            if b.drive > 0:
                x = ms.saturate(x, b.drive)
            if b.width > 0:
                mid = x.mean(axis=1, keepdims=True)
                x = mid + (x - mid) * (1 + b.width)
            if b.duck > 0 and self.kicks:
                x = ms.sidechain(x, self.spb, sorted(set(round(k, 4) for k in self.kicks)), depth=b.duck)
            if b.pan:
                g = np.array([min(1, 1 - b.pan), min(1, 1 + b.pan)])
                x = x * g
            x = x * b.gain
            dry += x
            if b.reverb > 0:
                send_r += x * b.reverb
            if b.delay > 0:
                send_d.setdefault(b.delay_beats, np.zeros((self.total, 2)))
                send_d[b.delay_beats] += x * b.delay
        out = dry
        if np.any(send_r):
            wet = ms.reverb(send_r, reverb_seconds, predelay, reverb_damp, shimmer=shimmer)
            out = out + wet[: self.total]
        for beats, s in send_d.items():
            wet = ms.delay(s, beats * self.spb, feedback=0.38, damp=4500)
            out = out + wet[: self.total] * 0.8
        if master:
            out = master_chain(out)
        return self.finish(out)

    def finish(self, out):
        n = ms.samples(self.length)
        if self.loop:
            # dobra a cauda (reverb/delay) sobre o início: loop sem emenda audível
            tail = out[n:]
            body = out[:n].copy()
            k = min(len(tail), n)
            body[:k] += tail[:k]
            return body
        fade_tail = ms.fade(out, 0, 1.5)
        return fade_tail


def master_chain(x, drive=1.05):
    x = ms.biquad(x, "hp", 28, 0.7)
    x = ms.compressor(x, threshold_db=-16, ratio=2.0, attack=0.02, release=0.25)
    x = ms.normalize(x, 0.9)
    x = ms.saturate(x, drive) * 0.95
    return x


def section(start, count):
    return range(start, start + count)
