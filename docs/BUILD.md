# Build — Morph Adventure (C99 + raylib 5.5)

Responsável: pessoa 7. Contrato de código: [`CONTRACTS.md`](CONTRACTS.md). Testes: [`TESTING.md`](TESTING.md).

> **Integração local:** os módulos 2–6 e `src/core/composition.c` estão disponíveis.
> Configure com `-DMORPH_MODULES_LINKED=ON` para gerar `morph-adventure`.
> O alvo `morph-core-shell` continua sendo apenas diagnóstico. Veja [INTEGRATION.md](INTEGRATION.md).

## O que o build gera

| Alvo | O que é | Precisa de raylib |
|---|---|---|
| `morph_core` | Biblioteca estática com `src/core/game.c` e `viewport.c` | Não |
| `morph-core-shell` | Janela de diagnóstico real (`src/main.c`) | Sim |
| `morph-adventure` | Jogo integrado. Só existe com `-DMORPH_MODULES_LINKED=ON` | Sim |
| `core-check`, `core-rules`, ... | Testes sem janela (CTest) | Não |

Todo o código do projeto compila em **C99** com `-Wall -Wextra -Wpedantic -Werror` (GCC/Clang) ou `/W4 /WX` (MSVC). A raylib não recebe essas flags.

## raylib 5.5 fixada

O arquivo `cmake/MorphRaylib.cmake` usa sempre a raylib **5.5** e confere a versão no `raylib.h` usado. Uma versão diferente interrompe a configuração. Há três formas de obter a raylib (`-DMORPH_RAYLIB_PROVIDER=...`):

| Modo | Como funciona | Quando usar |
|---|---|---|
| `FETCH` (padrão) | Na primeira configuração, baixa o código oficial com Git (tag 5.5, commit `c1ab645ca298a2801097931d1079b10ff7eb9df8`) e compila junto | Caso normal, com internet |
| `FETCH` offline | `-DFETCHCONTENT_SOURCE_DIR_RAYLIB=<pasta com o código 5.5>` | Sem internet, com o código já baixado |
| `PREBUILT` | `-DMORPH_RAYLIB_DIR=<pasta>` com `include/` e `lib/` do pacote oficial (ex.: `raylib-5.5_win64_mingw-w64`) | Mesmo pacote binário que a pessoa 1 usou |
| `SYSTEM` | `find_package(raylib 5.5 EXACT)` | raylib 5.5 já instalada no sistema |

Nenhum modo instala ferramentas no sistema. Se algo faltar, a configuração para com uma mensagem explicando o que fazer.

> O CMake da própria raylib também ativa C++. O compilador C++ precisa existir (no MSYS2 ele vem junto com o `gcc`; no Visual Studio, com a carga "Desenvolvimento para desktop com C++"). O nosso código continua sendo só C99.

## Windows (alvo principal)

### Opção A — MSYS2 UCRT64 (mesma base da pessoa 1)

Instale o [MSYS2](https://www.msys2.org/) e, **no terminal "MSYS2 UCRT64"**, instale as ferramentas uma vez:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja git
```

Na raiz do repositório:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```
```bash
cmake --build build
```
```bash
ctest --test-dir build --output-on-failure
```
```bash
./build/morph-core-shell.exe
```

Com o pacote binário oficial em vez do download (como a pessoa 1 fez):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR=C:/libs/raylib-5.5_win64_mingw-w64
```

No MinGW, os executáveis são ligados com `-static -static-libgcc`, para não dependerem de DLLs do MSYS2 quando abertos em outra pasta.

### Opção B — Visual Studio 2022 (não verificado)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```
```powershell
cmake --build build --config Release
```
```powershell
ctest --test-dir build -C Release --output-on-failure
```

O MSVC não tem um modo "C99 estrito"; ele compila o C99 do projeto como C11/C17. Esse caminho **ainda não foi executado** por ninguém da equipe.

## Linux (verificado no ambiente da pessoa 7)

Dependências (Ubuntu/Debian): `build-essential cmake git libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev`.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
```bash
cmake --build build --parallel
```
```bash
ctest --test-dir build --output-on-failure
```
```bash
./build/morph-core-shell
```

macOS: não verificado.

## Opções úteis

| Opção | Padrão | Efeito |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug`, `Release`, `RelWithDebInfo`. Os testes rodam em todos, inclusive com `NDEBUG` |
| `MORPH_BUILD_GAME` | `ON` | `OFF` compila só núcleo e testes, sem raylib |
| `MORPH_BUILD_TESTS` | `ON` | Testes CTest |
| `MORPH_WARNINGS_AS_ERRORS` | `ON` | Warnings do projeto viram erro |
| `MORPH_SANITIZE` | vazio | GCC/Clang: `address,undefined` |
| `MORPH_MODULES_LINKED` | `OFF` | Gera `morph-adventure`. Exige `src/core/composition.c` e `.c` em cada pasta de módulo; senão a configuração falha (sem módulos vazios) |

Não é permitido configurar o build dentro da pasta do código (`cmake -S . -B .`).

## Janela de diagnóstico

```bash
./build/morph-core-shell --smoke-frames 5 --capture core-shell.png
```

Abre a janela, desenha 5 quadros, salva a captura na pasta atual e imprime `core-shell: initialized=1 ready=0 missing=0x3f frames=5`. `ready=0` e `missing=0x3f` são o esperado enquanto as pessoas 2–6 não entregam.

## Pacote para distribuir

| Sistema | Comando | Situação |
|---|---|---|
| Linux | `scripts/package/package.sh` | Executado: compila, testa, gera o ZIP, extrai numa pasta temporária limpa e abre a janela a partir dela |
| Windows | `.\scripts\package\package.ps1` (opções `-Generator`, `-RaylibDir`) | Escrito, **ainda não executado em Windows** |

O ZIP fica em `<build>/package/MorphAdventure-<versão>-<sistema>-<arquitetura>.zip` e contém os executáveis e `LEIA-ME.txt`. O HTML de referência (`MorphAdventure.html`) continua no repositório e não é substituído.
