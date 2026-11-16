# Handoff da pessoa 7 — build, testes e distribuição

## Etapa efetivamente concluída

**Etapa 1, parte da pessoa 7: concluída para o que já existe no repositório.** Build CMake reproduzível com C99 e raylib 5.5 fixada, testes do núcleo sem janela, smoke test da janela de diagnóstico, checklist compartilhada e empacotamento com verificação em pasta limpa (Linux).

Não concluído (depende de outros): testes dos módulos, Ar jogável integrado, etapas 2 e 3. Nada aqui declara essas etapas prontas.

- Branch local: `migration/qa-build`, criada a partir de `origin/main` em `488370f` (merge de `migration/core`).
- **Sem push, sem PR e sem merge.** Aguardando autorização.
- HTML de referência: `MorphAdventure.html`, não alterado. SHA-256 da cópia com fim de linha LF neste clone: `95ff2d08faa020076a26b0a0b1a8089672326491def17ecf1d6e6d23614dd983`. Ele é diferente do hash do contrato (`3AF7952C...`), e isso já era esperado pelo próprio contrato por causa da conversão CRLF/LF do Git. Não li o JavaScript para o meu escopo: build e testes do núcleo não dependem dele. Ele será a referência dos testes da etapa 2.

## Arquivos criados (somente no escopo da pessoa 7)

| Arquivo | Conteúdo |
|---|---|
| `CMakeLists.txt` | Projeto C99; alvos `morph_core`, `morph-core-shell`; testes; pacote |
| `cmake/MorphRaylib.cmake` | raylib 5.5 por FETCH (commit `c1ab645`), FETCH offline, PREBUILT ou SYSTEM; confere a versão no `raylib.h` |
| `cmake/MorphWarnings.cmake` | `-Wall -Wextra -Wpedantic -Werror` só no código do projeto; sanitizers opcionais; runtime estático no MinGW |
| `cmake/MorphModules.cmake` | Alvo `morph-adventure` com `MORPH_MODULES_LINKED`. Falha se faltar `composition.c` ou módulo |
| `cmake/MorphPackage.cmake` | `install` + CPack ZIP |
| `tests/CMakeLists.txt` | Testes CTest, helper `morph_add_module_test`, inclusão automática de `tests/<modulo>/` |
| `tests/core/core_check.c` | Cópia do teste da pessoa 1, sem mudança de lógica (ver "Pedido à pessoa 1") |
| `tests/core/core_rules_test.c` | Novos testes das funções reais do núcleo |
| `tests/support/morph_check.h` | `MORPH_CHECK`, ativo também com `NDEBUG` |
| `tests/support/check_macro_selftest.c`, `no_assert.cmake` | Garantem que os testes valem em Release |
| `tests/smoke/run_shell_smoke.cmake` | Smoke test da janela real |
| `scripts/package/package.sh`, `package.ps1`, `LEIA-ME.txt` | Pacote e verificação em pasta limpa |
| `docs/BUILD.md`, `docs/TESTING.md` | Instruções, checklist e relatório de testes |
| `.gitignore` | Ignora pastas de build e capturas `.png` (exceto em `assets/`). Arquivo na raiz, fora da lista original da pessoa 7: avise se alguém preferir outra regra |

Nenhum arquivo de `src/`, `docs/CONTRACTS.md`, `README.md` ou do handoff da pessoa 1 foi alterado.

## Funções consumidas e oferecidas

- Consumidas (pessoa 1, sem mudança): `GameInit`, `GameShutdown`, `GameAdvance`, `GameDraw`, `GameEnergyRequired`, `GameEnergyFraction`, `GameEnqueueEvent`, `GameModeName`, `ViewportFit`, `ViewportToLogical`; tipos de `game_types.h`/`module_api.h`; saída `core-shell: ...` e opções `--smoke-frames`/`--capture` de `main.c`.
- Oferecidas à equipe:
  - Alvo `morph_core` (ligar testes ao núcleo real).
  - `morph_add_module_test(nome SOURCES <.c de produção> TESTS <.c do teste>)`, para as pessoas 2–6.
  - `MORPH_CHECK` / `MORPH_CHECK_NEAR`.
  - `-DMORPH_MODULES_LINKED=ON` gera `morph-adventure` quando `composition.c` e os módulos existirem.

## Comandos de teste usados

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
```
```bash
cmake --build build-release
```
```bash
ctest --test-dir build-release --output-on-failure
```
```bash
cmake -S . -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DMORPH_SANITIZE=address,undefined
```
```bash
scripts/package/package.sh build-package
```

Também foram feitos: build Debug, build Clang Release, Valgrind nos testes de lógica, build com `MORPH_BUILD_GAME=OFF` e cross-compile para Windows com MinGW-w64.

## Resultados observados

Detalhes e ambiente em [`docs/TESTING.md`](../TESTING.md#relatório-de-execução). Resumo:

- **Compilação:** GCC 13 e Clang 18 (Linux), MinGW-w64 GCC 13 (Windows x64, cross-compile). Nenhum warning do projeto com `-Werror`.
- **Testes automáticos:** 5/5 em Release (`-O3 -DNDEBUG`), Debug, Debug+ASan/UBSan e Clang. Valgrind sem erros.
- **Execução visual real:** a janela de diagnóstico abriu em Xvfb (Mesa llvmpipe, sem GPU) e a captura foi inspecionada. Não é execução em monitor físico nem em Windows.
- **Pacote:** no Linux x86_64, o ZIP foi extraído numa pasta temporária limpa e a janela abriu de lá. O `.exe` Windows cross-compilado depende só de DLLs do sistema, mas **não foi executado**.

## Defeito encaminhado

- **D1 → pessoa 1** (`src/core/viewport.c`): na janela padrão 1100×700, a posição `x = 1100` vira `959.99997` e conta como dentro da área de jogo, por arredondamento de float. Severidade baixa (um ponto exatamente na borda). Sugestão: calcular com `double` ou comparar a posição de tela com `v.x + v.width` antes de dividir. O teste `core-rules` usa 1 px de folga até a correção; quando corrigido, basta tirar a folga.

## Pedido à pessoa 1

`tests/core/core_check.c` é uma cópia de `docs/handoffs/pessoa-1/validation/core_check.c`. Não apaguei o original porque o arquivo é seu. Se concordar, remova a cópia de `docs/` para não haver duas versões. Futuras mudanças devem ir para `tests/core/`.

## Dependências pendentes

- Pessoas 2–6: factories e `.c` em `src/<modulo>/`, mais testes em `tests/<modulo>/` (itens 1–10 da checklist em `docs/TESTING.md`).
- Pessoa 1: `src/core/composition.c` para ligar `MORPH_MODULES_LINKED`.
- Alguém com **Windows**: rodar `.\scripts\package\package.ps1` (MSYS2 UCRT64) e informar o resultado. Só então o Windows pode ser marcado como verificado.

## Limitações

- MSVC, macOS e o script PowerShell não foram executados.
- FPS não foi medido: ainda não há jogo, e o ambiente usado não tem GPU.
- O teste `gui` precisa de monitor ou `xvfb-run` no Linux. Sem eles, fica desabilitado (aparece como "Not Run" no CTest), não aprovado.
- O README da raiz ainda descreve a versão HTML com regras antigas (6 fragmentos fixos, ímã 8 s, slow 7 s, x2 10 s, `index.html`). Não alterei porque não está no meu escopo.
