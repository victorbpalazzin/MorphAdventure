# Testes — Morph Adventure

Responsável: pessoa 7. Como compilar: [`BUILD.md`](BUILD.md).

## Como rodar

```bash
ctest --test-dir build --output-on-failure
```

No Visual Studio (multi-config), acrescente `-C Release` ou `-C Debug`. Para filtrar por grupo, use `-L core`, `-L contract`, `-L gui` ou `-L module`.

## Regras para todos os testes

1. **Use `MORPH_CHECK`** de `tests/support/morph_check.h`, nunca `assert()`. Com `NDEBUG` (Release), o `assert()` some, e a chamada dentro dele também. O teste `tests-no-assert` falha se encontrar `assert(` ou `<assert.h>` em `tests/`.
2. **Teste as funções reais.** Não copie código de produção para dentro do teste. Para módulos, use `morph_add_module_test` (em `tests/CMakeLists.txt`), que compila os `.c` de produção junto com o teste.
3. **Simulações só nos testes e com nome claro.** O `core_check.c` usa módulos simulados para testar o coordenador; isso está escrito no próprio arquivo. Simulações nunca entram no executável.
4. **Sementes fixas.** Tudo que for aleatório (obstáculos, itens) precisa aceitar uma semente para o teste ser reproduzível.
5. **Diga o tipo de evidência:** compilação, teste automático, estado preparado ou partida real jogada. São coisas diferentes.

## Testes existentes

| Teste | Label | O que verifica |
|---|---|---|
| `core-check` | core | Coordenador da pessoa 1 com módulos simulados: viewport, ciclo de vida e falha parcial no init, relógio fixo em 30/60/120/144 Hz, impulso pendente e consumido uma vez, pausa e perda de foco, dano/escudo, score com x2 sem efeito retroativo, campanha de 6 mundos com vidas preservadas, portal pausável, sem fade sobre a vitória, erros de contrato. Movido de `docs/handoffs/pessoa-1/validation/` |
| `core-rules` | core | Funções reais sem simulação: metas 4/8/12/16/20/24, bateria proporcional após cada coleta e 100% só na meta, fila de 128 eventos em ordem com overflow explícito, nomes de modo, viewport em 10 tamanhos de janela (proporção, centralização, ida e volta de coordenadas, barras rejeitadas) e janela de diagnóstico sem módulos que não simula nada |
| `check-macro-ndebug` | contract | Compilado sempre com `NDEBUG`; **precisa falhar** (`WILL_FAIL`). Prova que `MORPH_CHECK` continua ativo em Release |
| `tests-no-assert` | contract | Nenhum `assert()` em `tests/` |
| `morph-headers-standalone` | (build) | Cada cabeçalho de `src/core/` compila sozinho em C99, duas vezes (include guard). Falha na compilação, não no CTest |
| `audio-cues` | core | Diretor de áudio puro: um efeito por evento, intervalos mínimos, melodia da coleta, plano musical por tela, camadas do chefe, megarraio, perigos e ambiente |
| `audio-backend-run` | module;audio | raylib real: preferências, modo silencioso, carregamento de 47 efeitos e 12 músicas, fades, pausa, portal, camadas, vitória/derrota, mudo, arquivos ausentes (SKIP das partes com dispositivo se não houver placa de som) |
| `audio-flow` | gui;audio | Jogo completo com módulos reais: cada evento real gera exatamente o efeito esperado; trilhas na ordem dos seis mundos e da batalha. Aceita `--tempo-real` para gravar a mixagem final |
| `core-shell-smoke` | gui | Janela raylib real: 5 quadros, saída `initialized=1 ready=0 missing=0x3f frames=5` e captura PNG não vazia. No Linux sem monitor usa `xvfb-run`; sem ele, fica desabilitado |

## Checklist compartilhada (etapas 1 e 2)

Cada responsável entrega os testes do próprio módulo em `tests/<modulo>/` (com um `CMakeLists.txt` que usa `morph_add_module_test`). A pessoa 7 revisa, integra e roda em Debug e Release. ☐ = pendente.

| # | Cenário | Dono do módulo | Situação |
|---|---|---|---|
| 1 | Física: impulso -6.4, gravidade +0.34, queda máx. 7.2, dano -4.5 e 75 passos de invulnerabilidade | Pessoa 2 | ☐ aguarda `src/player/` |
| 2 | `CollisionCircleRect`/`CollisionCirclePolygon`: interior, arestas, tangente (distância estritamente menor que o raio) | Pessoa 2 | ☐ aguarda `src/player/collision.c` |
| 3 | `reset_world`: posição (180,245), raios 23/18, escala 1 | Pessoa 2 | ☐ |
| 4 | `config` válido para os mundos 0..5 e igual à tabela do contrato | Pessoa 4 | ☐ aguarda `src/obstacles/` |
| 5 | Geração com semente fixa; margem entre envelopes móveis; trajetória possível com a física real, **incluindo perigos** | Pessoas 4 e 2 | ☐ |
| 6 | `spawn=false` não cria obstáculos; os existentes saem; `take_passed` paga uma vez só | Pessoa 4 | ☐ |
| 7 | Energia: meta por mundo, IDs únicos, item perdido reaparece, sem coleta duplicada | Pessoa 5 | ☐ aguarda `src/items/` |
| 8 | Boosts: escudo sem duração e sem acumular, ímã 5 s, slow 4 s (0,68), x2 7 s, máx. 2, terceiro substitui o mais antigo, timers parados na pausa/portal | Pessoa 5 | ☐ |
| 9 | Portal: sinais ENTERED → BLACKOUT → FADE_DONE uma vez cada; vitória só depois do Fogo | Pessoa 6 | ☐ aguarda `src/effects/` |
| 10 | UI: clique em botão não vira impulso; RETRY/RESTART corretos | Pessoa 6 | ☐ aguarda `src/ui/` |
| 11 | Portal bloqueado com bateria **ou** percurso incompletos | Pessoa 1 (coordenador) | ✅ `core-check` (estado preparado) |
| 12 | Vidas preservadas entre mundos; derrota; nova partida com score zero | Pessoa 1 | ✅ `core-check` (estado preparado) |
| 13 | Pausa em gameplay, portal e transição sem salto de tempo | Pessoa 1 | ✅ `core-check` (estado preparado) |
| 14 | Score: obstáculo +10, fragmento +35, portal +100, x2 sem efeito retroativo | Pessoa 1 | ✅ `core-check` |
| 15 | Mesmo número de passos físicos com taxas de quadro diferentes | Pessoa 1 | ✅ `core-check` (30/60/120/144 Hz) |
| 16 | Redimensionamento e conversão de clique | Pessoa 1 | ✅ `core-rules` + `core-check` |
| 17 | Ar completo jogado de verdade; campanha completa jogada de verdade | Todos | ☐ aguarda integração |
| 18 | FPS/tempo de quadro no executável real, com hardware identificado | Pessoa 7 | ☐ aguarda o jogo integrado |

## Relatório de execução

Ambiente: container Linux x86_64 (Ubuntu 24.04), GCC 13.3.0, Clang 18.1.3, MinGW-w64 GCC 13 (win32), CMake 3.28.3, Ninja, raylib 5.5 compilada do código-fonte. Janela via Xvfb com Mesa llvmpipe (OpenGL por software, **sem placa de vídeo real**). Data: 16/11/2026.

| Verificação | Resultado |
|---|---|
| Configuração com download da raylib 5.5 (FETCH) e versão conferida no cabeçalho | ✅ aprovado |
| Build Release (`-O3 -DNDEBUG`), GCC, `-Werror` | ✅ aprovado, sem warnings do projeto |
| CTest Release (GCC): 5 testes | ✅ 5/5 |
| CTest Debug (GCC): 5 testes | ✅ 5/5 |
| CTest Debug com `-fsanitize=address,undefined` (núcleo, testes e janela) | ✅ 5/5 |
| CTest Release com Clang | ✅ 5/5 |
| Valgrind `--leak-check=full` em `core-check` e `core-rules` | ✅ sem erros nem vazamentos |
| `MORPH_BUILD_GAME=OFF` (sem raylib): 4 testes | ✅ 4/4 |
| Mensagens de erro: módulos ligados sem `composition.c`, PREBUILT sem pasta, build dentro do código | ✅ falham com mensagem explicativa |
| Janela real: 5 quadros, captura inspecionada visualmente (texto legível, barras proporcionais, 6 módulos PENDENTE) | ✅ aprovado (Xvfb/llvmpipe) |
| `scripts/package/package.sh`: ZIP gerado, extraído em pasta temporária limpa e aberto de lá | ✅ aprovado (Linux x86_64) |
| Cross-compile para Windows x64 (MinGW-w64 GCC no Linux) | ✅ compila sem warnings; `morph-core-shell.exe` depende só de DLLs do Windows (GDI32, KERNEL32, msvcrt, SHELL32, USER32, WINMM); ZIP gerado |
| Executar os `.exe` e os testes **no Windows** | ⛔ não executado (sem Windows neste ambiente) |
| `scripts/package/package.ps1` | ⛔ não executado |
| Visual Studio / MSVC | ⛔ não executado |
| macOS | ⛔ não executado |
| FPS / tempo de quadro | ⛔ não medido: só existe a janela de diagnóstico, e o ambiente não tem GPU real |
| Testes de módulos (pessoas 2–6), Ar jogável e campanha | ⛔ bloqueado: módulos ainda não entregues |

### Defeitos encontrados

| # | Onde | Descrição | Severidade | Encaminhado para |
|---|---|---|---|---|
| D1 | `src/core/viewport.c` (`ViewportToLogical`) | Arredondamento de float na borda direita: com a janela padrão 1100×700, a posição de tela `x = 1100` (logo depois do último pixel) vira `x lógico = 959.99997` e conta como **dentro** da área de jogo. Na prática afeta só um ponto exatamente na borda | Baixa | Pessoa 1 |
