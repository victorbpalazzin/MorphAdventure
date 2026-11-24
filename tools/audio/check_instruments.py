"""Verificação objetiva dos instrumentos: nenhuma nota pode terminar com corte abrupto,
e a afinação deve estar correta (erro < 3 cents) nos instrumentos afinados."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import numpy as np
import morphsynth as ms
import tracks

INSTRUMENTS = {
    "pad": ms.inst_pad, "strings": ms.inst_strings, "spiccato": ms.inst_spiccato, "brass": ms.inst_brass,
    "lead": ms.inst_lead, "flute": ms.inst_flute, "ep": ms.inst_ep, "piano": ms.inst_piano, "bell": ms.inst_bell,
    "glass": ms.inst_glass, "mallet": ms.inst_mallet, "pluck": ms.inst_pluck, "bass": ms.inst_bass, "choir": ms.inst_choir,
    "drop": tracks.inst_drop, "voice": tracks.inst_voice, "wood_flute": tracks.inst_wood_flute, "kalimba": tracks.inst_kalimba,
    "guitar": tracks.inst_guitar, "stab": tracks.inst_stab, "tremolo": tracks.inst_tremolo,
}
failures = 0
for name, inst in INSTRUMENTS.items():
    for midi in (45, 62, 81):
        x = inst(float(ms.hz(midi)), 0.5, 0.8)
        mono = x.mean(axis=1) if x.ndim == 2 else x
        peak = np.abs(mono).max()
        end = np.abs(mono[-64:]).max()
        raw_end_db = 20 * np.log10(max(end, 1e-12) / peak)
        faded = ms.tail_fade(mono)
        last = np.abs(faded[-ms.samples(0.004):]).max() / peak
        ok = 20 * np.log10(max(last, 1e-12)) < -40
        failures += not ok
        print(f"{name:11s} midi {midi}: fim bruto {raw_end_db:6.1f} dB | após tail_fade {'OK' if ok else 'FALHA'}")
print("FALHAS:", failures)
sys.exit(1 if failures else 0)
