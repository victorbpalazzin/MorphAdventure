"""Renderiza as trilhas e efeitos de Morph Adventure.

Uso (na raiz do repositório):
    python3 tools/audio/render.py music [nome ...]
    python3 tools/audio/render.py sfx [nome ...]
    python3 tools/audio/render.py all
Gera assets/audio/music/*.ogg, assets/audio/sfx/*.wav e assets/audio/manifest.json.
"""
import json
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "assets", "audio")


def manifest_update(section, name, info):
    path = os.path.join(OUT, "manifest.json")
    data = {}
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
    data.setdefault(section, {})[name] = info
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False, sort_keys=True)


def render_music(names):
    import tracks
    import export
    os.makedirs(os.path.join(OUT, "music"), exist_ok=True)
    for name in names or list(tracks.TRACKS):
        t0 = time.time()
        spec = tracks.TRACKS[name]
        result = spec()
        if isinstance(result, dict):  # trilha em camadas sincronizadas (batalha final)
            for stem, (audio, opts) in result.items():
                info = export.export_music(audio, os.path.join(OUT, "music", stem + ".ogg"), **opts)
                manifest_update("music", stem, info)
                print(f"{stem}: {info}")
        else:
            audio, opts = result if isinstance(result, tuple) else (result, {})
            info = export.export_music(audio, os.path.join(OUT, "music", name + ".ogg"), **opts)
            manifest_update("music", name, info)
            print(f"{name}: {info}  ({time.time() - t0:.0f}s)")


def render_sfx(names):
    import sfx
    import export
    os.makedirs(os.path.join(OUT, "sfx"), exist_ok=True)
    for name in names or list(sfx.SFX):
        audio, peak = sfx.SFX[name]()
        info = export.export_sfx(audio, os.path.join(OUT, "sfx", name + ".wav"), peak_db=peak, mono=audio.ndim == 1)
        manifest_update("sfx", name, info)
        print(f"{name}: {info}")


if __name__ == "__main__":
    what = sys.argv[1] if len(sys.argv) > 1 else "all"
    names = sys.argv[2:]
    if what in ("music", "all"):
        render_music(names if what == "music" else [])
    if what in ("sfx", "all"):
        render_sfx(names if what == "sfx" else [])
