"""Efeitos exclusivos da versão educativa (desafios de lógica e segunda chance).

Princípios: informar sem assustar. O acerto é caloroso e satisfatório; o erro é
um gesto suave e descendente (nunca uma buzina); o cronômetro só "fala" nos
últimos segundos, com um tique discreto. Mesma paleta do jogo (Ré maior).
"""
import numpy as np
import morphsynth as ms


def build(g):
    F, place, space, sparkle, collect, t_axis = g["F"], g["place"], g["space"], g["sparkle"], g["collect"], g["t_axis"]

    def notes(seq, inst, gap, vel=0.55, total=1.6, pan_spread=0.3, **kw):
        out = np.zeros((ms.samples(total), 2))
        for k, nm in enumerate(seq):
            pan = pan_spread * (k / max(1, len(seq) - 1) * 2 - 1)
            place(out, ms.stereo(inst(F(nm), 0.3, vel, **kw), pan), k * gap)
        return out

    def quiz_open():
        """Pergunta aparece: duas notas que pedem atenção + leve swell de pad (sem urgência)."""
        out = notes(("A5", "D6"), ms.inst_bell, 0.11, 0.55, 1.8, ratio=3.0, decay=1.4)
        pad = sum(ms.inst_pad(F(n), 0.6, 0.35, bright=3000, attack=0.25, release=0.7) for n in ("D4", "A4", "F#5"))
        place(out, pad * 0.7, 0.05)
        n, t = t_axis(0.25)
        place(out, ms.biquad(ms.noise(n), "bp", 3500, 0.8) * np.sin(np.pi * t / 0.25) ** 2 * 0.15, 0)
        return space(out, 1.4, 0.3), -6.0

    def quiz_select():
        """Troca de alternativa: toque curtíssimo de mallet."""
        return space(ms.inst_mallet(F("E6"), 0.1, 0.4, wood=False)[: ms.samples(0.5)], 0.5, 0.15), -15.0

    def quiz_correct():
        """Acerto: arpejo maior ascendente + acorde que floresce + brilho."""
        out = notes(("D5", "F#5", "A5", "D6", "F#6"), ms.inst_bell, 0.065, 0.6, 2.4, ratio=2.0, decay=1.4, index=1.0)
        chord = sum(ms.inst_pad(F(n), 0.7, 0.5, bright=5000, attack=0.04, release=0.9, detune=12) for n in ("D5", "A5", "F#6"))
        place(out, chord, 0.3)
        place(out, ms.stereo(sparkle(1.0, 45, 6000, 12000, 81) * 0.45), 0.3)
        return space(out, 1.8, 0.35), -4.0

    def quiz_wrong():
        """Erro: dois tons suaves descendo (sem dissonância agressiva), timbre aveludado."""
        out = notes(("A4", "F4"), ms.inst_ep, 0.18, 0.5, 1.8, bright=0.4)
        n, t = t_axis(0.6)
        place(out, ms.biquad(ms.noise(n, "pink"), "lp", 900) * np.exp(-t / 0.2) * 0.08, 0.15)
        return space(out, 1.4, 0.3, 5000), -9.0

    def quiz_tick():
        """Últimos segundos (3, 2, 1): tique de relógio de madeira, discreto."""
        out = ms.drum_wood(0.7, 1300)[: ms.samples(0.12)] + 0.4 * ms.inst_glass(F("A6"), 0.05, 0.3)[: ms.samples(0.12)]
        return space(out, 0.4, 0.12), -12.0

    def quiz_timeout():
        """Tempo esgotado: sino abafado duplo + queda suave (informa, sem alarme)."""
        out = notes(("E5", "E5"), ms.inst_bell, 0.22, 0.5, 2.0, ratio=1.41, decay=1.2, index=0.8)
        pad = sum(ms.inst_pad(F(n) * 1.0, 0.8, 0.3, bright=1200, attack=0.1, release=0.8) for n in ("D4", "A4"))
        place(out, pad, 0.35)
        return space(out, 1.6, 0.3, 4500), -8.0

    def quiz_new():
        """Nova pergunta: página virando + nota leve."""
        n, t = t_axis(0.3)
        page = ms.sweep(ms.noise(n), "bp", 2000 + 4000 * t / 0.3, q=1.2) * np.sin(np.pi * t / 0.3) ** 2 * 0.4
        out = np.zeros((ms.samples(1.2), 2))
        place(out, ms.stereo(page), 0)
        place(out, ms.stereo(ms.inst_bell(F("D6"), 0.2, 0.45, ratio=3.0, decay=1.0)), 0.18)
        return space(out, 1.0, 0.25), -10.0

    def quiz_unlock():
        """Desafio de portal concluído: escala mágica até a oitava + brilho do portal."""
        out = notes(("D5", "E5", "F#5", "A5", "B5", "D6"), ms.inst_glass, 0.06, 0.55, 2.6)
        place(out, ms.stereo(sparkle(1.4, 50, 5000, 12000, 91) * 0.5), 0.3)
        chord = sum(ms.inst_strings(F(n), 0.7, 0.4, attack=0.15, release=0.9) for n in ("D4", "A4", "D5", "F#5"))
        place(out, chord, 0.35)
        return space(out, 2.0, 0.35), -4.0

    def heartbeat(out, at, vel=0.6):
        n, t = t_axis(0.25)
        beat = np.sin(2 * np.pi * np.cumsum(65 + 55 * np.exp(-t / 0.03)) / ms.SR) * np.exp(-t / 0.07) * vel
        place(out, beat, at)
        place(out, beat * 0.75, at + 0.2)

    def revive_open():
        """Segunda chance: batimento + swell esperançoso (tensão gentil, não ameaça)."""
        out = np.zeros((ms.samples(2.4), 2))
        heartbeat(out, 0.0, 0.7)
        heartbeat(out, 0.75, 0.55)
        pad = sum(ms.inst_pad(F(n), 1.2, 0.45, bright=2500, attack=0.8, release=0.8) for n in ("D4", "G4", "B4"))
        place(out, pad, 0.2)
        place(out, notes(("A5", "D6", "E6"), ms.inst_bell, 0.14, 0.45, 1.6, ratio=3.0, decay=1.4), 0.55)
        return space(out, 1.8, 0.3), -4.0

    def revive_success():
        """Ressurreição: acorde maior que se abre + batimento forte + faíscas."""
        out = np.zeros((ms.samples(2.6), 2))
        heartbeat(out, 0.0, 0.8)
        chord = sum(ms.inst_strings(F(n), 0.9, 0.5, attack=0.08, release=1.0) for n in ("D4", "A4", "D5", "F#5", "A5"))
        place(out, chord, 0.1)
        place(out, notes(("D6", "F#6", "A6", "D7"), ms.inst_glass, 0.07, 0.5, 2.0), 0.15)
        place(out, ms.stereo(sparkle(1.2, 50, 6000, 12000, 101) * 0.5), 0.15)
        return space(out, 2.0, 0.35), -3.0

    def revive_fail():
        """Segunda chance encerrada: descida suave e acolhedora (prepara a tela de derrota)."""
        out = notes(("D5", "A4", "F4", "D4"), ms.inst_ep, 0.16, 0.5, 2.4, bright=0.4)
        pad = sum(ms.inst_pad(F(n), 1.0, 0.3, bright=1000, attack=0.3, release=1.0) for n in ("D3", "A3", "F4"))
        place(out, pad, 0.4)
        return space(out, 2.0, 0.35, 4500), -6.0

    return {
        "letter": lambda: collect("D6", letter=True, seed=12),
        "quiz_open": quiz_open, "quiz_select": quiz_select, "quiz_correct": quiz_correct, "quiz_wrong": quiz_wrong,
        "quiz_tick": quiz_tick, "quiz_timeout": quiz_timeout, "quiz_new": quiz_new, "quiz_unlock": quiz_unlock,
        "revive_open": revive_open, "revive_success": revive_success, "revive_fail": revive_fail,
    }
