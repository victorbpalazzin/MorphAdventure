"""Exportação: limitador final, normalização de loudness (EBU R128), OGG/WAV e métricas."""
import json
import os
import re
import subprocess
import tempfile
import numpy as np
from scipy.io import wavfile
import morphsynth as ms


def write_wav(path, x, mono=False):
    x = np.asarray(x, dtype=float)
    if mono and x.ndim == 2:
        x = x.mean(axis=1)
    data = np.clip(x, -1, 1)
    # dither TPDF leve antes de quantizar para 16 bits
    data = data + (np.random.default_rng(1).random(data.shape) - np.random.default_rng(2).random(data.shape)) / 32768
    wavfile.write(path, ms.SR, (np.clip(data, -1, 1) * 32767).astype(np.int16))


def measure(path):
    """Loudness integrada (LUFS), true peak (dBTP) e duração via ffmpeg."""
    r = subprocess.run(["ffmpeg", "-hide_banner", "-nostats", "-i", path, "-af",
                        "ebur128=peak=true", "-f", "null", "-"], capture_output=True, text=True)
    err = r.stderr
    lufs = re.findall(r"I:\s+(-?[\d.]+|-inf) LUFS", err)
    peak = re.findall(r"Peak:\s+(-?[\d.]+|-inf) dBFS", err)
    dur = re.findall(r"Duration: (\d+):(\d+):([\d.]+)", err)
    seconds = None
    if dur:
        h, m, s = dur[0]
        seconds = int(h) * 3600 + int(m) * 60 + float(s)
    to_f = lambda v: float("-inf") if v == "-inf" else float(v)
    return {"lufs": to_f(lufs[-1]) if lufs else None, "true_peak": to_f(peak[-1]) if peak else None, "seconds": seconds}


def loop_seam_db(x):
    """Diferença entre o fim e o começo do loop (salto na emenda), em dB relativo ao RMS."""
    a = x[-64:]
    b = x[:64]
    jump = np.abs(a[-1] - b[0]).max()
    slope = np.abs(np.diff(x[:2048], axis=0)).mean()
    return float(20 * np.log10(max(jump, 1e-9) / max(slope * 4, 1e-9)))


def _loudness(x):
    with tempfile.TemporaryDirectory() as tmp:
        wav = os.path.join(tmp, "m.wav")
        write_wav(wav, x)
        err = subprocess.run(["ffmpeg", "-hide_banner", "-nostats", "-i", wav, "-af",
                              "loudnorm=print_format=json", "-f", "null", "-"], capture_output=True, text=True).stderr
        stats = json.loads(err[err.rindex("{"):err.rindex("}") + 1])
    return float(stats["input_i"]), float(stats["input_tp"])


def common_gain(total, lufs=-16.0, ceiling_db=-1.5):
    """Ganho (dB) que leva a soma das camadas a `lufs`, sem ultrapassar o teto de pico."""
    i, tp = _loudness(total)
    return min(lufs - i, ceiling_db - tp)


def export_music(x, path, lufs=-16.0, true_peak=-1.5, quality=5, loop=True, fixed_gain_db=None):
    """Normaliza em duas passagens com loudnorm (linear) e codifica em Vorbis.

    fixed_gain_db: ganho comum já calculado (camadas sincronizadas não podem ser
    normalizadas uma a uma, senão a soma muda de equilíbrio e pode estourar)."""
    if fixed_gain_db is not None:
        y = x * ms.db(fixed_gain_db)
        with tempfile.TemporaryDirectory() as tmp:
            norm = os.path.join(tmp, "norm.wav")
            write_wav(norm, y)
            subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", norm, "-c:a", "libvorbis",
                            "-q:a", str(quality), path], check=True)
        info = measure(path)
        info["gain_db"] = round(fixed_gain_db, 2)
        if loop:
            info["seam_db"] = round(loop_seam_db(y), 1)
        return info
    x = ms.limiter(x, -0.8)
    with tempfile.TemporaryDirectory() as tmp:
        wav = os.path.join(tmp, "in.wav")
        write_wav(wav, x)
        first = subprocess.run(["ffmpeg", "-hide_banner", "-nostats", "-i", wav, "-af",
                                f"loudnorm=I={lufs}:TP={true_peak}:LRA=11:print_format=json", "-f", "null", "-"],
                               capture_output=True, text=True).stderr
        stats = json.loads(first[first.rindex("{"):first.rindex("}") + 1])
        gain = lufs - float(stats["input_i"])
        tp_room = true_peak - float(stats["input_tp"])
        gain = min(gain, tp_room)  # ganho linear, nunca acima do teto de pico
        y = x * ms.db(gain)
        norm = os.path.join(tmp, "norm.wav")
        write_wav(norm, y)
        subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", norm, "-c:a", "libvorbis",
                        "-q:a", str(quality), path], check=True)
    info = measure(path)
    info["gain_db"] = round(gain, 2)
    if loop:
        info["seam_db"] = round(loop_seam_db(y), 1)
    return info


def trim_silence(x, floor_db=-50.0, keep=0.03):
    """Corta a cauda abaixo de floor_db em relação ao pico (reverb quase inaudível)."""
    level = np.abs(x).max(axis=1) if x.ndim == 2 else np.abs(x)
    peak = level.max()
    if peak <= 0:
        return x
    loud = np.nonzero(level > peak * ms.db(floor_db))[0]
    end = min(len(x), loud[-1] + ms.samples(keep)) if len(loud) else len(x)
    return x[:end]


def export_sfx(x, path, peak_db=-3.0, mono=True):
    x = np.asarray(x, dtype=float)
    x = ms.normalize(x, ms.db(peak_db))
    x = trim_silence(x)
    n = len(x)
    fade_len = min(n, max(ms.samples(0.006), n // 12))
    tail = np.linspace(1, 0, fade_len)
    if x.ndim == 2:
        x[n - fade_len:] *= tail[:, None]
    else:
        x[n - fade_len:] *= tail
    seconds = n / ms.SR
    if seconds > 1.5:  # efeitos longos: Vorbis de alta qualidade (raylib decodifica no LoadSound)
        path = os.path.splitext(path)[0] + ".ogg"
        with tempfile.TemporaryDirectory() as tmp:
            wav = os.path.join(tmp, "s.wav")
            write_wav(wav, x, mono=mono)
            subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", wav, "-c:a", "libvorbis",
                            "-q:a", "6", path], check=True)
    else:
        write_wav(path, x, mono=mono)
    return {"peak_db": peak_db, "seconds": round(seconds, 3), "file": os.path.basename(path)}


def spectrogram(path, png, width=1200, height=360):
    subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", path, "-lavfi",
                    f"showspectrumpic=s={width}x{height}:legend=1:scale=log:fscale=log", png], check=True)
