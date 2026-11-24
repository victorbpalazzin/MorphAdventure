"""Morph Adventure - biblioteca de síntese e mixagem offline (numpy + scipy).

Gera todo o áudio original do jogo a partir de código: instrumentos sintetizados
(subtrativo, FM, modal, Karplus-Strong, formantes), efeitos (reverb por
convolução, delay, chorus, saturação, sidechain), sequenciamento musical e
masterização. Nenhuma amostra externa é usada; todo o resultado é original.
"""
import math
import numpy as np
from scipy import signal
from scipy.ndimage import maximum_filter1d

SR = 44100
RNG = np.random.default_rng(20261124)

# ---------------------------------------------------------------- utilidades

def hz(midi):
    return 440.0 * 2.0 ** ((np.asarray(midi, dtype=float) - 69.0) / 12.0)


NOTE_INDEX = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}


def note(name):
    """'C#4' -> número MIDI."""
    name = name.strip()
    base = NOTE_INDEX[name[0].upper()]
    i = 1
    while i < len(name) and name[i] in "#b":
        base += 1 if name[i] == "#" else -1
        i += 1
    octave = int(name[i:])
    return 12 * (octave + 1) + base


def db(x):
    return 10.0 ** (x / 20.0)


def samples(seconds):
    return max(1, int(round(seconds * SR)))


def silence(seconds, channels=2):
    return np.zeros((samples(seconds), channels)) if channels == 2 else np.zeros(samples(seconds))


def stereo(x, pan=0.0, width=0.0):
    """Mono -> estéreo com pan de potência constante. pan -1..1."""
    if x.ndim == 2:
        return x
    angle = (pan + 1) * math.pi / 4
    out = np.stack([x * math.cos(angle), x * math.sin(angle)], axis=1) * math.sqrt(2)
    if width > 0:  # pequeno atraso Haas para abrir a imagem
        d = int(0.011 * SR * width)
        if d > 0:
            out[d:, 1] = 0.5 * out[d:, 1] + 0.5 * out[:-d, 1]
    return out


def mix_into(buf, x, start):
    """Soma x (mono ou estéreo) em buf (estéreo) a partir da amostra start."""
    if start >= len(buf):
        return
    if x.ndim == 1:
        x = stereo(x)
    if start < 0:
        x = x[-start:]
        start = 0
    end = min(len(buf), start + len(x))
    buf[start:end] += x[: end - start]


def tail_fade(x, seconds=0.02):
    """Garante final suave de qualquer nota/efeito (sem corte abrupto audível)."""
    k = min(len(x), samples(seconds))
    if k < 2:
        return x
    x = np.array(x, dtype=float, copy=True)
    ramp = np.linspace(1, 0, k)
    x[len(x) - k:] = (x[len(x) - k:].T * ramp).T
    return x


def fade(x, fade_in=0.0, fade_out=0.0):
    x = x.copy()
    n = len(x)
    if fade_in > 0:
        k = min(n, samples(fade_in))
        ramp = np.linspace(0, 1, k) ** 2
        x[:k] = (x[:k].T * ramp).T
    if fade_out > 0:
        k = min(n, samples(fade_out))
        ramp = np.linspace(1, 0, k) ** 2
        x[n - k:] = (x[n - k:].T * ramp).T
    return x

# ---------------------------------------------------------------- envelopes

def adsr(n, a=0.01, d=0.1, s=0.7, r=0.2, hold=None, curve=3.0):
    """Envelope ADSR com segmentos exponenciais. hold = duração da nota (s)."""
    hold = n / SR if hold is None else hold
    env = np.zeros(n)
    na, nd = samples(a), samples(d)
    nh = min(n, samples(hold))
    t = np.arange(n)
    # ataque
    k = min(na, n)
    env[:k] = 1 - np.exp(-curve * np.linspace(0, 1, k)) if k > 1 else 1
    env[:k] /= max(env[k - 1], 1e-9)
    # decaimento até sustain
    if nh > na:
        m = np.arange(nh - na)
        env[na:nh] = s + (1 - s) * np.exp(-m / max(1.0, nd / curve))
    level = env[nh - 1] if nh > 0 else 0
    if n > nh:
        m = np.arange(n - nh)
        env[nh:] = level * np.exp(-m / max(1.0, samples(r) / 4.6))
    return env


def exp_decay(n, seconds, start=1.0):
    return start * np.exp(-np.arange(n) / max(1.0, seconds * SR / 6.9))

# ---------------------------------------------------------------- osciladores

def phase_of(freq, n, phase0=0.0):
    f = np.broadcast_to(np.asarray(freq, dtype=float), (n,))
    return (phase0 + np.cumsum(f) / SR) % 1.0, f / SR


def saw(freq, n, phase0=None):
    phase0 = RNG.random() if phase0 is None else phase0
    p, dt = phase_of(freq, n, phase0)
    y = 2 * p - 1
    dt = np.maximum(dt, 1e-9)
    m = p < dt
    t = p[m] / dt[m]
    y[m] -= t + t - t * t - 1
    m = p > 1 - dt
    t = (p[m] - 1) / dt[m]
    y[m] -= t * t + t + t + 1
    return y


def square(freq, n, width=0.5, phase0=None):
    phase0 = RNG.random() if phase0 is None else phase0
    a = saw(freq, n, phase0)
    b = saw(freq, n, (phase0 + width) % 1.0)
    return 0.5 * (a - b)


def sine(freq, n, phase0=0.0):
    p, _ = phase_of(freq, n, phase0)
    return np.sin(2 * np.pi * p)


def tri(freq, n, phase0=0.0):
    p, _ = phase_of(freq, n, phase0)
    return 1 - 4 * np.abs(p - 0.5)


def noise(n, color="white"):
    x = RNG.standard_normal(n)
    if color == "pink":
        b, a = [0.049922035, -0.095993537, 0.050612699, -0.004408786], [1, -2.494956002, 2.017265875, -0.522189400]
        x = signal.lfilter(b, a, x) * 3.5
    elif color == "brown":
        x = np.cumsum(x)
        x = signal.lfilter([1, -1], [1, -0.995], x) * 0.06
    return x


def supersaw(freq, n, voices=7, detune_cents=18.0, stereo_out=True):
    left = np.zeros(n)
    right = np.zeros(n)
    for i in range(voices):
        spread = (i - (voices - 1) / 2) / max(1, (voices - 1) / 2)
        cents = spread * detune_cents
        v = saw(np.asarray(freq) * 2 ** (cents / 1200), n)
        pan = spread * 0.8
        left += v * math.cos((pan + 1) * math.pi / 4)
        right += v * math.sin((pan + 1) * math.pi / 4)
    s = np.stack([left, right], axis=1) / math.sqrt(voices)
    return s if stereo_out else s.mean(axis=1)

# ---------------------------------------------------------------- filtros

def _sos(kind, f, q=0.707, gain_db=0.0):
    f = float(np.clip(f, 10, SR * 0.45))
    w = 2 * math.pi * f / SR
    cw, sw = math.cos(w), math.sin(w)
    alpha = sw / (2 * q)
    A = 10 ** (gain_db / 40)
    if kind == "lp":
        b = [(1 - cw) / 2, 1 - cw, (1 - cw) / 2]; a = [1 + alpha, -2 * cw, 1 - alpha]
    elif kind == "hp":
        b = [(1 + cw) / 2, -(1 + cw), (1 + cw) / 2]; a = [1 + alpha, -2 * cw, 1 - alpha]
    elif kind == "bp":
        b = [alpha, 0, -alpha]; a = [1 + alpha, -2 * cw, 1 - alpha]
    elif kind == "peak":
        b = [1 + alpha * A, -2 * cw, 1 - alpha * A]; a = [1 + alpha / A, -2 * cw, 1 - alpha / A]
    elif kind == "lowshelf":
        sq = 2 * math.sqrt(A) * alpha
        b = [A * ((A + 1) - (A - 1) * cw + sq), 2 * A * ((A - 1) - (A + 1) * cw), A * ((A + 1) - (A - 1) * cw - sq)]
        a = [(A + 1) + (A - 1) * cw + sq, -2 * ((A - 1) + (A + 1) * cw), (A + 1) + (A - 1) * cw - sq]
    elif kind == "highshelf":
        sq = 2 * math.sqrt(A) * alpha
        b = [A * ((A + 1) + (A - 1) * cw + sq), -2 * A * ((A - 1) + (A + 1) * cw), A * ((A + 1) + (A - 1) * cw - sq)]
        a = [(A + 1) - (A - 1) * cw + sq, 2 * ((A - 1) - (A + 1) * cw), (A + 1) - (A - 1) * cw - sq]
    else:
        raise ValueError(kind)
    b = np.array(b) / a[0]; a = np.array(a) / a[0]
    return np.concatenate([b, a])[None, :]


def biquad(x, kind, f, q=0.707, gain_db=0.0):
    sos = _sos(kind, f, q, gain_db)
    return signal.sosfilt(sos, x, axis=0)


def sweep(x, kind, cutoff, q=0.707, block=256):
    """Filtro com frequência variando no tempo (processado em blocos, estado contínuo)."""
    cutoff = np.broadcast_to(np.asarray(cutoff, dtype=float), (len(x),))
    out = np.zeros_like(x)
    zi = None
    for s in range(0, len(x), block):
        e = min(len(x), s + block)
        sos = _sos(kind, cutoff[s:e].mean(), q)
        if zi is None:
            zi = np.zeros((1, 2) + x.shape[1:])
        out[s:e], zi = signal.sosfilt(sos, x[s:e], axis=0, zi=zi)
    return out


def onepole_lp(x, f):
    a = math.exp(-2 * math.pi * f / SR)
    return signal.lfilter([1 - a], [1, -a], x, axis=0)


def formant(x, vowel="a", q=9.0):
    table = {
        "a": [(800, 0), (1150, -5), (2900, -12)],
        "o": [(450, 0), (800, -8), (2830, -18)],
        "u": [(325, 0), (700, -12), (2530, -24)],
        "e": [(400, 0), (2000, -10), (2550, -14)],
        "i": [(300, 0), (2300, -10), (3000, -14)],
    }
    out = 0
    for f, g in table[vowel]:
        out = out + biquad(x, "bp", f, q) * db(g)
    return out * 3.0

# ---------------------------------------------------------------- dinâmica e cor

def saturate(x, drive=1.0):
    return np.tanh(x * drive) / math.tanh(drive) if drive > 0 else x


def normalize(x, peak=0.9):
    m = np.max(np.abs(x))
    return x * (peak / m) if m > 0 else x


def compressor(x, threshold_db=-18, ratio=3.0, attack=0.01, release=0.15, makeup_db=0.0):
    mono = np.abs(x).max(axis=1) if x.ndim == 2 else np.abs(x)
    a_att = math.exp(-1 / (attack * SR)); a_rel = math.exp(-1 / (release * SR))
    # seguidor de envelope de pico com ataque/relaxamento distintos (vetorizado em blocos)
    env = signal.lfilter([1 - a_rel], [1, -a_rel], mono)
    env = np.maximum(env, signal.lfilter([1 - a_att], [1, -a_att], mono))
    level = 20 * np.log10(np.maximum(env, 1e-6))
    over = np.maximum(0, level - threshold_db)
    gain = db(-over * (1 - 1 / ratio) + makeup_db)
    return (x.T * gain).T if x.ndim == 2 else x * gain


def limiter(x, ceiling_db=-1.0, lookahead=0.005, release=0.08):
    ceil = db(ceiling_db)
    peak = np.abs(x).max(axis=1) if x.ndim == 2 else np.abs(x)
    la = samples(lookahead)
    peak = maximum_filter1d(peak, size=2 * la + 1)
    need = np.minimum(1.0, ceil / np.maximum(peak, 1e-9))
    a = math.exp(-1 / (release * SR))
    # ganho sobe devagar (release) e desce imediatamente
    g = np.empty_like(need)
    cur = 1.0
    # versão vetorizada aproximada: suavização + mínimo instantâneo
    smooth = signal.lfilter([1 - a], [1, -a], need, zi=[need[0] * a])[0]
    g = np.minimum(need, smooth)
    g = np.minimum(g, 1.0)
    out = (x.T * g).T if x.ndim == 2 else x * g
    return np.clip(out, -ceil, ceil)


def sidechain(x, beat_seconds, starts, depth=0.6, release=0.22):
    """Ducking rítmico (bombeamento) disparado em starts (segundos)."""
    n = len(x)
    g = np.ones(n)
    curve_len = samples(release * 2.5)
    curve = 1 - depth * np.exp(-np.arange(curve_len) / (release * SR / 3))
    for s in starts:
        i = samples(s)
        if i >= n:
            continue
        e = min(n, i + curve_len)
        g[i:e] = np.minimum(g[i:e], curve[: e - i])
    return (x.T * g).T if x.ndim == 2 else x * g

# ---------------------------------------------------------------- espaço

_IR_CACHE = {}


def impulse_response(seconds=2.5, predelay=0.015, damp=6000, bright=1.0, seed=7):
    key = (round(seconds, 3), predelay, damp, bright, seed)
    if key in _IR_CACHE:
        return _IR_CACHE[key]
    rng = np.random.default_rng(seed)
    n = samples(seconds * 1.3)
    t = np.arange(n) / SR
    ir = np.zeros((n, 2))
    for ch in range(2):
        nz = rng.standard_normal(n)
        low = biquad(nz, "lp", 600)
        mid = biquad(biquad(nz, "hp", 600), "lp", 4000)
        high = biquad(nz, "hp", 4000) * bright
        # graves decaem um pouco mais devagar, agudos bem mais rápido (ar)
        e_low = np.exp(-6.9 * t / (seconds * 1.15))
        e_mid = np.exp(-6.9 * t / seconds)
        e_high = np.exp(-6.9 * t / (seconds * 0.45))
        ir[:, ch] = low * e_low + mid * e_mid + high * e_high
        ir[:, ch] = onepole_lp(ir[:, ch], damp)
    # reflexões iniciais esparsas
    for k in range(14):
        d = samples(0.004 + rng.random() * 0.06)
        g = 0.55 * (1 - k / 14) * (1 if rng.random() > 0.5 else -1)
        ir[d, k % 2] += g
    pre = samples(predelay)
    ir = np.vstack([np.zeros((pre, 2)), ir])
    # fade-in curto evita ataque duro do ruído
    ir[: samples(0.004) + pre] *= np.linspace(0, 1, samples(0.004) + pre)[:, None]
    ir /= np.sqrt(np.sum(ir ** 2) / 2)
    _IR_CACHE[key] = ir
    return ir


def reverb(x, seconds=2.5, predelay=0.02, damp=6000, bright=1.0, shimmer=0.0):
    x = stereo(x)
    ir = impulse_response(seconds, predelay, damp, bright)
    wet = np.stack([signal.fftconvolve(x[:, c], ir[:, c]) for c in range(2)], axis=1)
    if shimmer > 0:  # oitava acima realimentada na cauda (efeito cristalino)
        up = octave_up(x)
        wet2 = np.stack([signal.fftconvolve(up[:, c], ir[:, c]) for c in range(2)], axis=1)
        wet[: len(wet2)] += shimmer * wet2
    return wet * 0.35


def octave_up(x):
    """Pitch-shift aproximado de oitava por grãos sobrepostos (para shimmer)."""
    x = stereo(x)
    grain = samples(0.06)
    hop = grain // 2
    win = np.hanning(grain)
    out = np.zeros_like(x)
    for s in range(0, len(x) - grain, hop):
        g = x[s:s + grain]
        g2 = g[::2]
        g2 = np.vstack([g2, g2])[:grain]
        out[s:s + grain] += g2 * win[:, None]
    return biquad(out, "hp", 900) * 0.5


def delay(x, time, feedback=0.35, mix=1.0, pingpong=True, damp=5000, taps=8):
    x = stereo(x)
    d = samples(time)
    out = np.zeros((len(x) + d * taps, 2))
    cur = x.copy()
    gain = 1.0
    for k in range(1, taps + 1):
        gain *= feedback
        if gain < 0.01:
            break
        cur = onepole_lp(cur, damp)
        tap = cur * gain
        if pingpong and k % 2 == 1:
            tap = tap[:, ::-1]
        out[k * d:k * d + len(x)] += tap
    return out * mix


def chorus(x, depth_ms=6.0, rate=0.6, mix=0.5):
    x = stereo(x)
    n = len(x)
    t = np.arange(n) / SR
    base = samples(0.012)
    out = x.copy() * (1 - mix * 0.5)
    for ch, ph in ((0, 0.0), (1, math.pi / 2)):
        dly = base + depth_ms / 1000 * SR * 0.5 * (1 + np.sin(2 * np.pi * rate * t + ph))
        idx = np.arange(n) - dly
        out[:, ch] += mix * np.interp(idx, np.arange(n), x[:, ch], left=0)
    return out


def tremolo(x, rate, depth=0.5):
    n = len(x)
    lfo = 1 - depth * 0.5 * (1 + np.sin(2 * np.pi * rate * np.arange(n) / SR))
    return (x.T * lfo).T if x.ndim == 2 else x * lfo

# ---------------------------------------------------------------- instrumentos
# Todos retornam mono (ou estéreo) com cauda de release incluída.

def inst_pad(f, dur, vel=0.7, bright=2400, attack=0.8, release=1.6, detune=14, voices=5, vowel=None):
    n = samples(dur + release)
    x = supersaw(f, n, voices, detune)
    lfo = 1 + 0.25 * np.sin(2 * np.pi * 0.18 * np.arange(n) / SR + RNG.random() * 6)
    x = sweep(x, "lp", bright * lfo, q=0.6)
    if vowel:
        x = formant(x, vowel) * 0.6 + x * 0.4
    env = adsr(n, attack, 0.5, 0.85, release, hold=dur)
    return (x.T * env).T * vel * 0.35


def inst_strings(f, dur, vel=0.7, attack=0.35, release=0.6, bright=3200):
    n = samples(dur + release)
    vib = 1 + 0.0035 * np.sin(2 * np.pi * 5.3 * np.arange(n) / SR + RNG.random() * 6) * np.clip(np.arange(n) / (0.4 * SR), 0, 1)
    x = supersaw(f * vib, n, 5, 9)
    x = biquad(x, "lp", bright, 0.7)
    x = biquad(x, "peak", 1200, 1.0, 3)
    env = adsr(n, attack, 0.3, 0.9, release, hold=dur)
    return (x.T * env).T * vel * 0.3


def inst_spiccato(f, dur, vel=0.7):
    dur = min(dur, 0.22)
    n = samples(dur + 0.15)
    x = supersaw(f, n, 3, 7)
    x = biquad(x, "lp", 2600 + 2000 * vel, 0.8)
    env = adsr(n, 0.004, 0.08, 0.35, 0.09, hold=dur)
    bow = noise(n) * exp_decay(n, 0.03) * 0.15
    return ((x.T * env).T + stereo(biquad(bow, "bp", 3000, 2))) * vel * 0.4


def inst_brass(f, dur, vel=0.8, bright=1.0, release=0.25):
    n = samples(dur + release)
    scoop = f * (1 - 0.03 * np.exp(-np.arange(n) / (0.04 * SR)))
    x = supersaw(scoop, n, 3, 6)
    fenv = 300 + (900 + 2600 * vel * bright) * adsr(n, 0.06, 0.4, 0.55, release, hold=dur)
    x = sweep(x, "lp", fenv, q=0.9)
    x = saturate(x, 1.4)
    env = adsr(n, 0.03, 0.2, 0.85, release, hold=dur)
    return (x.T * env).T * vel * 0.35


def inst_lead(f, dur, vel=0.8, glide_from=None, bright=3800, breath=0.0, vibrato=0.006, kind="saw"):
    n = samples(dur + 0.35)
    t = np.arange(n) / SR
    freq = np.full(n, float(f))
    if glide_from:
        g = samples(0.06)
        freq[:g] = np.geomspace(glide_from, f, g)
    vib = 1 + vibrato * np.sin(2 * np.pi * 5.5 * t) * np.clip((t - 0.25) / 0.3, 0, 1)
    if kind == "saw":
        x = 0.6 * saw(freq * vib, n) + 0.4 * saw(freq * vib * 1.003, n)
    elif kind == "square":
        x = square(freq * vib, n, 0.42)
    else:
        x = sine(freq * vib, n) + 0.18 * sine(2 * freq * vib, n) + 0.06 * sine(3 * freq * vib, n)
    x = biquad(x, "lp", bright, 0.8)
    if breath > 0:
        b = biquad(noise(n), "bp", min(9000, f * 3), 1.2) * breath
        x = x + b * adsr(n, 0.05, 0.2, 0.5, 0.2, hold=dur)
    env = adsr(n, 0.02, 0.15, 0.85, 0.25, hold=dur)
    return x * env * vel * 0.3


def inst_flute(f, dur, vel=0.7):
    return inst_lead(f, dur, vel, kind="sine", breath=0.5, bright=6000, vibrato=0.008)


def inst_ep(f, dur, vel=0.7, bright=1.0):
    """Piano elétrico FM (tine + corpo)."""
    n = samples(dur + 1.4)
    t = np.arange(n) / SR
    idx = (1.2 + 2.5 * vel * bright) * np.exp(-t / 0.35)
    mod = np.sin(2 * np.pi * f * 1.0 * t) * idx
    body = np.sin(2 * np.pi * f * t + mod)
    tine = np.sin(2 * np.pi * f * 7.0 * t) * np.exp(-t / 0.04) * 0.25 * vel
    env = adsr(n, 0.002, 1.8, 0.15, 0.4, hold=dur, curve=4)
    return (body + tine) * env * vel * 0.35


def inst_piano(f, dur, vel=0.7):
    n = samples(min(dur, 4.0) + 1.8)
    t = np.arange(n) / SR
    x = np.zeros(n)
    B = 0.0004
    for k in range(1, 9):
        fk = f * k * math.sqrt(1 + B * k * k)
        if fk > 16000:
            break
        amp = (1 / k ** 1.2) * (1 if k < 4 else 0.7)
        dec = 2.8 / (1 + 0.35 * k) * (440 / max(f, 110)) ** 0.25
        x += amp * np.sin(2 * np.pi * fk * t + RNG.random()) * np.exp(-t / dec)
    hammer = biquad(noise(n), "bp", min(6000, f * 4), 1.5) * np.exp(-t / 0.006) * 0.25
    env = adsr(n, 0.001, 3.0, 1.0, 0.35, hold=dur)
    return (x + hammer) * env * vel * 0.32


def inst_bell(f, dur, vel=0.7, ratio=3.5, decay=2.5, index=2.0):
    """Sino/vidro FM inarmônico."""
    n = samples(decay * 2.2 + 0.1)
    t = np.arange(n) / SR
    mod = np.sin(2 * np.pi * f * ratio * t) * index * np.exp(-t / (decay * 0.3))
    x = np.sin(2 * np.pi * f * t + mod) * np.exp(-t / (decay / 2.3))
    x += 0.3 * np.sin(2 * np.pi * f * 2.0 * t) * np.exp(-t / (decay / 5))
    x[: samples(0.002)] *= np.linspace(0, 1, samples(0.002))
    return x * vel * 0.3


def inst_glass(f, dur, vel=0.7):
    n = samples(dur + 5.5)
    t = np.arange(n) / SR
    x = np.zeros(n)
    for ratio, amp, d in ((1, 1, 1.1), (2.76, 0.4, 0.65), (5.4, 0.2, 0.33), (8.93, 0.08, 0.16)):
        x += amp * np.sin(2 * np.pi * f * ratio * t) * np.exp(-t / d)
    x[: samples(0.003)] *= np.linspace(0, 1, samples(0.003))
    return x * vel * 0.22


def inst_mallet(f, dur, vel=0.7, wood=True):
    """Marimba/kalimba por síntese modal."""
    modes = ((1, 1.0, 0.55), (3.93, 0.25, 0.12), (9.54, 0.08, 0.04)) if wood else ((1, 1.0, 0.9), (5.95, 0.3, 0.25), (16.4, 0.1, 0.05))
    n = samples(5.5 * max(d for _, _, d in modes))
    t = np.arange(n) / SR
    x = np.zeros(n)
    for ratio, amp, d in modes:
        if f * ratio < 18000:
            x += amp * np.sin(2 * np.pi * f * ratio * t) * np.exp(-t / d)
    click = biquad(noise(samples(0.004)), "bp", 3000, 1) * 0.3
    x[: len(click)] += click
    return x * vel * 0.4


def inst_pluck(f, dur, vel=0.7, bright=0.7, decay=0.996, body=True):
    """Karplus-Strong (corda dedilhada) via filtro IIR, afinado com precisão.

    O laço de atraso tem período inteiro + 0,5 amostra (filtro de média); o
    resultado é reamostrado para a frequência exata, sem desafinar as notas agudas.
    """
    length = max(dur, 0.2) + 1.2
    period = SR / f
    N = max(2, int(period - 0.5))
    actual = N + 0.5
    ratio = actual / period            # >1: o laço soa grave demais; acelera na leitura
    n = samples(length * ratio + 0.01)
    burst = noise(N + 2) * exp_decay(N + 2, 0.01)
    burst = onepole_lp(burst, 1500 + 9000 * bright)
    excite = np.zeros(n)
    excite[: len(burst)] = burst
    a = np.zeros(N + 2)
    a[0] = 1
    a[N] = -decay * 0.5
    a[N + 1] = -decay * 0.5
    y = signal.lfilter([1], a, excite)
    m = samples(length)
    y = np.interp(np.arange(m) * ratio, np.arange(n), y)
    if body:
        y = y + 0.4 * biquad(y, "peak", 220, 1.2, 6)
    rel = adsr(m, 0.001, 0.1, 1.0, 0.15, hold=max(dur, 0.2) + 0.6)
    return y * rel * vel * 0.5


def inst_bass(f, dur, vel=0.8, kind="sub", bright=600):
    n = samples(dur + 0.2)
    t = np.arange(n) / SR
    if kind == "sub":
        x = np.sin(2 * np.pi * f * t) + 0.15 * np.sin(2 * np.pi * 2 * f * t)
        x = saturate(x, 1.3)
    elif kind == "reese":
        x = saw(f, n) + saw(f * 1.012, n) + 0.8 * np.sin(2 * np.pi * f * t)
        x = sweep(x, "lp", bright * (1 + 0.6 * np.sin(2 * np.pi * 0.25 * t)), q=1.1)
        x = saturate(x * 0.6, 2.0)
    else:  # pluck bass
        x = saw(f, n)
        x = sweep(x, "lp", 200 + bright * 2 * np.exp(-t / 0.08), q=1.2)
        x = x + 0.6 * np.sin(2 * np.pi * f * t)
    env = adsr(n, 0.005, 0.2, 0.85, 0.08, hold=dur)
    return x * env * vel * 0.45


def inst_choir(f, dur, vel=0.7, vowel="a"):
    n = samples(dur + 1.2)
    t = np.arange(n) / SR
    vib = 1 + 0.004 * np.sin(2 * np.pi * 4.8 * t + RNG.random() * 6)
    src = supersaw(f * vib, n, 5, 12)
    src = biquad(src, "lp", 5000)
    x = formant(src, vowel, q=7.0)
    env = adsr(n, 0.6, 0.5, 0.9, 1.2, hold=dur)
    return (x.T * env).T * vel * 0.25

# ---------------------------------------------------------------- percussão

def drum_kick(vel=1.0, tone=52, punch=1.0, decay=0.35, click=1.0):
    n = samples(decay + 0.1)
    t = np.arange(n) / SR
    f = tone * (1 + 3.5 * punch * np.exp(-t / 0.03))
    x = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t / (decay / 3))
    c = biquad(noise(n), "hp", 2500) * np.exp(-t / 0.003) * 0.35 * click
    return saturate(x + c, 1.2) * vel * 0.9


def drum_snare(vel=1.0, tone=190, snap=1.0, decay=0.22):
    n = samples(decay + 0.1)
    t = np.arange(n) / SR
    body = np.sin(2 * np.pi * tone * t) * np.exp(-t / 0.05)
    body += 0.5 * np.sin(2 * np.pi * tone * 1.6 * t) * np.exp(-t / 0.03)
    nz = biquad(biquad(noise(n), "hp", 1200), "lp", 9000) * np.exp(-t / (decay / 3)) * snap
    return (0.6 * body + nz) * vel * 0.55


def drum_clap(vel=1.0):
    n = samples(0.35)
    t = np.arange(n) / SR
    env = np.zeros(n)
    for k, o in enumerate((0, 0.009, 0.018, 0.027)):
        i = samples(o)
        env[i:] += np.exp(-(t[: n - i]) / (0.008 if k < 3 else 0.08))
    x = biquad(noise(n), "bp", 1400, 1.2) * env
    return x * vel * 0.5


def drum_hat(vel=0.6, open_=False, tone=8000):
    n = samples(0.45 if open_ else 0.08)
    t = np.arange(n) / SR
    metal = sum(square(tone * r / 8000 * 330, n, 0.5) for r in (1.0, 1.342, 1.2312, 1.6532, 1.9523, 2.1523))
    x = biquad(metal * 0.3 + noise(n) * 0.7, "hp", tone)
    x *= np.exp(-t / (0.12 if open_ else 0.018))
    return x * vel * 0.25


def drum_shaker(vel=0.5, length=0.09):
    n = samples(length + 0.03)
    t = np.arange(n) / SR
    env = np.sin(np.pi * np.clip(t / length, 0, 1)) ** 2
    x = biquad(noise(n), "bp", 6500, 1.5) * env
    return x * vel * 0.3


def drum_tom(vel=0.9, tone=110, decay=0.5):
    n = samples(decay + 0.1)
    t = np.arange(n) / SR
    f = tone * (1 + 0.6 * np.exp(-t / 0.05))
    x = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t / (decay / 3))
    x += biquad(noise(n), "bp", tone * 3, 2) * np.exp(-t / 0.02) * 0.3
    return x * vel * 0.6


def drum_taiko(vel=1.0, tone=62, decay=1.0):
    n = samples(decay + 0.2)
    t = np.arange(n) / SR
    f = tone * (1 + 0.4 * np.exp(-t / 0.06))
    x = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t / (decay / 3.5))
    x += 0.4 * np.sin(2 * np.pi * np.cumsum(f * 1.58) / SR) * np.exp(-t / (decay / 6))
    skin = biquad(noise(n), "bp", 700, 1.0) * np.exp(-t / 0.04) * 0.5
    return saturate(x + skin, 1.3) * vel * 0.8


def drum_wood(vel=0.6, tone=900):
    """Batida de madeira: transiente de ruído + modos inarmônicos curtos."""
    n = samples(0.12)
    t = np.arange(n) / SR
    x = 0.6 * np.sin(2 * np.pi * tone * t) * np.exp(-t / 0.012)
    x += 0.35 * np.sin(2 * np.pi * tone * 2.71 * t) * np.exp(-t / 0.006)
    x += 0.25 * np.sin(2 * np.pi * tone * 0.53 * t) * np.exp(-t / 0.02)
    x += biquad(noise(n), "bp", tone * 2.2, 1.4) * np.exp(-t / 0.004) * 0.9
    return x * vel * 0.3


def drum_frame(vel=0.7, tone=150):
    n = samples(0.5)
    t = np.arange(n) / SR
    x = np.sin(2 * np.pi * tone * t) * np.exp(-t / 0.12)
    x += biquad(noise(n), "bp", 900, 0.9) * np.exp(-t / 0.03) * 0.6
    return x * vel * 0.5


def cymbal(vel=0.7, length=2.5, reverse=False):
    n = samples(length)
    t = np.arange(n) / SR
    x = biquad(noise(n), "hp", 5000) + 0.3 * biquad(noise(n), "bp", 3200, 3)
    x *= np.exp(-t / (length / 4))
    if reverse:
        x = x[::-1]
    return x * vel * 0.22


def riser(length=4.0, vel=0.6, start=300, end=6000):
    n = samples(length)
    t = np.arange(n) / SR
    cut = start * (end / start) ** (t / length)
    x = sweep(noise(n), "bp", cut, q=2.0)
    x *= (t / length) ** 2
    return x * vel * 0.8

# ---------------------------------------------------------------- texturas

def texture_wind(seconds, vel=0.3, center=700, seed=3):
    rng = np.random.default_rng(seed)
    n = samples(seconds)
    t = np.arange(n) / SR
    lfo = center * (1 + 0.5 * np.sin(2 * np.pi * 0.07 * t + rng.random() * 6) + 0.25 * np.sin(2 * np.pi * 0.19 * t))
    x = sweep(noise(n, "pink"), "bp", lfo, q=1.8)
    amp = 0.6 + 0.4 * np.sin(2 * np.pi * 0.05 * t + 1.0)
    return stereo(x * amp, width=1.0) * vel


def texture_water(seconds, vel=0.3, bubbles=0.4, seed=5):
    rng = np.random.default_rng(seed)
    n = samples(seconds)
    t = np.arange(n) / SR
    bed = sweep(noise(n, "brown"), "lp", 350 * (1 + 0.3 * np.sin(2 * np.pi * 0.11 * t)), q=0.7)
    out = stereo(bed * 0.8, width=1.0)
    count = int(seconds * 1.4 * bubbles)
    for _ in range(count):
        dur = 0.03 + rng.random() * 0.06
        m = samples(dur)
        tt = np.arange(m) / SR
        f0 = 500 + rng.random() * 1400
        b = np.sin(2 * np.pi * np.cumsum(f0 * (1 + 2.2 * tt / dur)) / SR) * np.sin(np.pi * tt / dur) ** 2
        mix_into(out, stereo(b * (0.15 + 0.2 * rng.random()), rng.uniform(-0.8, 0.8)), samples(rng.random() * seconds))
    return out * vel


def texture_forest(seconds, vel=0.3, seed=11):
    rng = np.random.default_rng(seed)
    n = samples(seconds)
    out = stereo(biquad(noise(n, "pink"), "bp", 2500, 0.5) * 0.12, width=1.0)
    t = 0.5
    while t < seconds - 1:
        f = 2600 + rng.random() * 1800
        for k in range(rng.integers(2, 5)):  # pequenos trinados
            m = samples(0.05 + rng.random() * 0.05)
            tt = np.arange(m) / SR
            sweep_f = f * (1 + 0.35 * np.sin(np.pi * tt / tt[-1]) * (1 if k % 2 else -1))
            ch = np.sin(2 * np.pi * np.cumsum(sweep_f) / SR) * np.sin(np.pi * tt / tt[-1]) ** 2
            mix_into(out, stereo(ch * 0.12, rng.uniform(-0.9, 0.9)), samples(t + k * 0.08))
        t += 2.5 + rng.random() * 5
    return out * vel


def texture_crackle(seconds, vel=0.3, density=25, seed=13):
    rng = np.random.default_rng(seed)
    n = samples(seconds)
    x = np.zeros(n)
    count = int(seconds * density)
    pos = rng.integers(0, n, count)
    x[pos] = rng.standard_normal(count) * rng.random(count) ** 3
    x = biquad(x, "hp", 1200)
    rumble = biquad(noise(n, "brown"), "lp", 120) * 0.6
    return stereo(x + rumble, width=1.0) * vel


def texture_sand(seconds, vel=0.3, seed=17):
    rng = np.random.default_rng(seed)
    n = samples(seconds)
    t = np.arange(n) / SR
    grains = biquad(noise(n) * (rng.random(n) > 0.985), "hp", 3500)
    gust = 0.5 + 0.5 * np.sin(2 * np.pi * 0.09 * t) ** 2
    bed = sweep(noise(n, "pink"), "bp", 1500 * (1 + 0.4 * np.sin(2 * np.pi * 0.13 * t)), q=1.0)
    return stereo((grains * 0.6 + bed * 0.5) * gust, width=1.0) * vel


def texture_ice(seconds, vel=0.3, seed=19):
    rng = np.random.default_rng(seed)
    out = silence(seconds)
    t = 0.2
    while t < seconds - 0.5:
        f = 3000 + rng.random() * 4000
        mix_into(out, stereo(inst_glass(f, 0.1, 0.25 + 0.3 * rng.random()), rng.uniform(-0.9, 0.9)), samples(t))
        t += 0.3 + rng.random() * 1.6
    return out * vel
