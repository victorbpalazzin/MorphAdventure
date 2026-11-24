#!/usr/bin/env bash
# Gera o pacote ZIP (Linux) e verifica que ele abre fora da pasta de desenvolvimento.
# Uso: scripts/package/package.sh [pasta-de-build]   (padrão: build-package)
# Não instala nada no sistema. Requer: cmake, compilador C, git (1ª configuração),
# headers X11/OpenGL e, sem monitor, xvfb-run para o teste da janela.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD="${1:-build-package}"
case "$BUILD" in /*) ;; *) BUILD="$ROOT/$BUILD" ;; esac

for tool in cmake; do
    command -v "$tool" >/dev/null || { echo "ERRO: '$tool' não encontrado. Veja docs/BUILD.md." >&2; exit 1; }
done

echo "== Configurando (Release) em $BUILD"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release ${MORPH_CMAKE_ARGS:-}
echo "== Compilando"
cmake --build "$BUILD" --parallel
echo "== Testando"
ctest --test-dir "$BUILD" --output-on-failure
echo "== Empacotando"
(cd "$BUILD" && cpack -C Release)

ZIP="$(ls -t "$BUILD"/package/MorphAdventure-*.zip | head -n 1)"
echo "== Pacote: $ZIP"

# Pasta limpa, fora do repositório e do build.
CLEAN="$(mktemp -d "${TMPDIR:-/tmp}/morph-clean-XXXXXX")"
trap 'rm -rf "$CLEAN"' EXIT
cmake -E chdir "$CLEAN" cmake -E tar xf "$ZIP"
PKG_DIR="$(find "$CLEAN" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
echo "== Conteúdo extraído em pasta limpa:"
ls -l "$PKG_DIR"

echo "== Dependências dinâmicas do executável:"
ldd "$PKG_DIR/morph-core-shell" || true

LAUNCHER=""
if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
    if command -v xvfb-run >/dev/null; then
        LAUNCHER="$(command -v xvfb-run)|-a|-s|-screen 0 1280x800x24"
    else
        echo "AVISO: sem monitor e sem xvfb-run; janela NÃO foi executada a partir do pacote." >&2
        exit 2
    fi
fi
echo "== Executando a partir da pasta limpa"
cmake -DEXE="$PKG_DIR/morph-core-shell" -DWORK_DIR="$CLEAN/smoke" "-DLAUNCHER=$LAUNCHER" \
      -P "$ROOT/tests/smoke/run_shell_smoke.cmake"
if [ -x "$PKG_DIR/morph-adventure" ]; then
    echo "== Jogo e áudio a partir da pasta limpa"
    for f in assets/audio/music/menu.ogg assets/audio/music/boss_3.ogg assets/audio/sfx/beam.ogg assets/audio/sfx/collect.ogg CREDITOS.txt; do
        [ -f "$PKG_DIR/$f" ] || { echo "ERRO: pacote sem $f" >&2; exit 1; }
    done
    LOG="$CLEAN/jogo.log"
    ( cd "$CLEAN" && ${LAUNCHER:+xvfb-run -a} "$PKG_DIR/morph-adventure" --smoke-frames 90 > "$LOG" 2>&1 ) || { cat "$LOG"; exit 1; }
    grep -E "AUDIO: Audio:|Audio desativado" "$LOG" || true
    if grep -q "pasta assets/audio nao encontrada" "$LOG"; then echo "ERRO: o jogo extraído não achou o áudio" >&2; exit 1; fi
fi
echo "OK: $ZIP abre fora da pasta de desenvolvimento neste sistema ($(uname -sm))."
