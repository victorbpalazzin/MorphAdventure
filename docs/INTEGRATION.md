# Integração local — Windows / MSYS2 UCRT64

A mensagem `modulos_ligados=OFF` corresponde à opção CMake `MORPH_MODULES_LINKED`.
Ela continua OFF por padrão para permitir o alvo de diagnóstico. O cache local em
`build/` foi configurado com ON; não é necessário mudar o padrão no CMakeLists.txt.

`src/core/composition.c` conecta as factories reais de player, worlds, obstacles,
items, ui e effects. Também encaminha proteção dos fragmentos ao módulo de obstáculos
e estado ambiental ao cenário. Uma falha no encaminhamento da proteção invalida a
consulta de passagem, detectada pelo coordenador como erro, sem continuar silenciosamente.
O main integrado encaminha Tab/Shift-Tab e Enter à UI; Espaço, W, seta para cima, P e
clique seguem o fluxo existente. Nenhuma regra de vidas/pontos foi duplicada.

No terminal UCRT64, dentro deste checkout:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_MODULES_LINKED=ON -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR="C:/caminho/raylib-5.5_win64_mingw-w64"
cmake --build build
ctest --test-dir build --output-on-failure
./build/morph-adventure.exe
```

Nesta máquina a dependência usada está em:
`C:/Users/fabia/Documents/Codex/2026-11-10/files-mentioned-by-the-user-voo/work/deps/raylib-5.5/raylib-5.5_win64_mingw-w64`.
Com o cache já configurado, basta executar os comandos de build, testes e jogo.

Se não houver pacote PREBUILT, o provedor FETCH fixado na raylib 5.5 pode ser usado,
mas requer internet e compiladores instalados. Não misture bibliotecas MinGW/MSVC
ou ambientes MINGW64/UCRT64 no mesmo build. Caso um build preexistente use outro
gerador ou compilador, crie outra pasta de build para preservar o anterior.

O executável correto é `morph-adventure.exe`; `morph-core-shell.exe` mostra a tela de
diagnóstico por projeto, mesmo quando a integração está habilitada.

O teste `ui-flow` agora usa a MESMA MorphCreateModules do executável principal,
substituindo a antiga composição duplicada do laboratório. Ele testa os seis portais,
pausa, derrota, reinício e recursos com módulos reais. Preenche energia/distância por
fixtures para testar transições rapidamente; isso não representa uma partida humana
completa nem garante balanceamento. Demais testes cobrem física, colisões, geração,
energia e boosts. Nenhum teste vazio substitui módulo de produção.

Trabalho mantido localmente na branch `integration/local`, sem publicação ou merge.

Validacao local: compilacao Release com GCC 16.2 UCRT64 e raylib 5.5 PREBUILT concluida; 16/16 testes passaram em 30,45 s. O executavel morph-adventure foi aberto por cinco quadros e confirmou initialized=1 ready=1 missing=0x00, encerrando com codigo 0. Captura: build/integration-smoke.png; log CTest: build/Testing/Temporary/LastTest.log. Isso confirma inicializacao e testes automatizados, sem substituir uma partida humana completa.
