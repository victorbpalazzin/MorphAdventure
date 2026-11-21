# Grootspectro — integração local

Branch `feature/grootspectro`, base `e9b25b9`. Sem publicação ou merge.
O README tinha alterações anteriores à tarefa e será preservado.

## Decisão de contrato (antes da implementação)

Contrato v2: acrescentar `GAME_BOSS`, `GameInput.attack_pressed` e estado de combate
ao GameState. Não mudar assinaturas das seis factories. Somente core coordena modos,
vidas e pontos. Helpers puros do boss recebem apenas seu estado; UI/effects leem-no.
Todos os alvos devem ser recompilados juntos; tabelas v1 são rejeitadas.

Novo `src/boss/` contém estado e lógica sem raylib. Arte procedural fica em effects;
energia de combate em items; HUD em ui. Não há texturas ou recursos externos novos.
Referência visual: imagem enviada `Grootspectro, Titã Espectral da Floresta.png`:
tronco retorcido, garras ramificadas, olhos/boca/fissuras violetas, folhas vermelhas
e névoa espectral. Adaptar para vetores do jogo, sem usar o fundo da foto como sprite.

O sexto portal conserva sua animação e recompensa. No blackout, o core reinicia
somente a pose do player, limpa obstáculos/efeitos e inicia a apresentação do boss,
sem reset de vidas, pontuação ou boosts. Cinco portais anteriores inalterados.
Introdução e derrota suspendem física/dano; combate mantém física normal, escudo,
invulnerabilidade e chão. Pausa congela tudo. Slow escala projéteis/coletáveis uma vez;
física, duração de boosts e animações do raio usam passos normais.

Meta inicial configurável: 4 fragmentos por raio. Namespace próprio, um coletável
por vez, reposição sem limite de tentativas. Três raios completos, um registro de
dano por disparo. Coleta paga 35 pelo Reward central e respeita x2.
Ataques saem da boca, avisam a direção e não perseguem o jogador após o disparo.
Durante coleta, reservar uma faixa horizontal para que energia obrigatória não
seja atravessada pelos projéteis. Fases variam frequência, velocidade e quantidade.

## Validação

Etapas A, B e C implementadas; etapa D com compilação, testes e inspeção gráfica
local concluídas. Balanceamento humano e outros sistemas permanecem não validados.

Ambiente: Windows x64, GCC 16.2 UCRT64, C99 Release, raylib 5.5 PREBUILT,
CMake 3.31.6, Ninja. Warnings do projeto tratados como erros.
A primeira tentativa em build/ encontrou o executável antigo em uso pelo Windows;
nenhum processo do usuário foi fechado. A entrega compilada usa build-boss/.

Comandos executados na raiz work/ui-work (CMake, Ninja e UCRT64 no PATH):

    cmake -S . -B build-boss -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_MODULES_LINKED=ON -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR=C:/Users/fabia/Documents/Codex/2026-11-10/files-mentioned-by-the-user-voo/work/deps/raylib-5.5/raylib-5.5_win64_mingw-w64
    cmake --build build-boss --parallel 6
    ctest --test-dir build-boss --output-on-failure
    ./build-boss/morph-adventure.exe --smoke-frames 5

Resultado final: **17/17 testes passaram**, 30,82 s. Executável integrado abriu,
desenhou 5 frames e encerrou com código 0: ready=1, missing=0x00.

- boss-rules: reposição de mais de dez fragmentos perdidos, carga configurável,
  limite e percentuais, sem dupla coleta, avisos sem colisão, colisões reais dos
  projéteis, velocidades/frequências das três fases, exatamente três impactos,
  recusa de disparo incompleto/duplicado, esvaziamento e derrota adiada.
  Comparação de deslocamento em 68 passos normais versus 100 passos a 0,68.
- core-check: seis portais com fixtures declaradas; somente o Fogo inicia boss;
  vida persistente, pausa/foco, escudo/invulnerabilidade, derrota interrompe coleta,
  x2 na recompensa de combate, bateria convencional independente, J enfileirado,
  três impactos antes da vitória, retry/menu limpam o boss.
- ui-flow: todos os módulos reais, seis portais (energia/distância preparadas),
  capturas das fases/raios, vitória, início e janelas em três resoluções; desenho
  não muda o estado e recursos dos mundos são liberados.
- Teste adicional dentro de ui-flow: arena inicial preparada, depois voo por
  impulsos, projéteis, coleta e três ataques pelo core real. Nenhum teleporte,
  reposição artificial de vidas ou preenchimento artificial de bateria durante
  esse combate. Terminou em WON, 3 vidas, 420 pontos (12 coletas). Este controlador
  automatizado comprova uma rota amostrada, não o equilíbrio de todas as partidas.
- Testes anteriores de física, colisões, obstáculos, itens e cenários passaram.

Capturas finais inspecionadas: troncos/garras/raízes e fissuras violetas adaptados
para vetores; fragmento multicolorido distinto de orbe violeta; raio em camadas;
menu, vitória e HUD sem sobreposição das instruções de combate com os menus.
A fonte padrão suporta os acentos usados; a seta não suportada foi substituída
por texto. Nenhum asset, heap ou textura novo foi introduzido pelo boss.

## Arquivos entregues

Criados: src/boss/boss.h e boss.c; src/items/combat_energy.h e combat_energy.c;
src/effects/boss_visual.h e boss_visual.c; tests/boss/CMakeLists.txt e boss_rules.c;
este handoff.

Alterados: src/core/game_types.h, game.h e game.c; src/main.c;
src/player/player.c; src/worlds/worlds.c; src/effects/effects.c;
src/ui/ui.c e ui_draw.c; CMakeLists.txt e cmake/MorphModules.cmake;
tests/CMakeLists.txt; tests/core/core_check.c e core_rules_test.c;
tests/ui/CMakeLists.txt e ui_lab.c; docs/CONTRACTS.md; README.md.
O README preserva a reformulação anterior e acrescenta controles/regras do boss.

## Limites e integração

Sem push, merge ou nova Release. Alterações locais na branch feature/grootspectro.
O HTML original foi consultado para preservar portais, vidas, física e tempos;
não foi alterado. A imagem recebida orienta a arte vetorial e não é carregada em
runtime. Sem teste humano completo, sanitizers nesta entrega, Linux/macOS ou
promessa de fidelidade fotográfica. Textos narrativos foram condensados para a
área lógica 960×540, preservando a história e as mensagens principais.
O personagem paira durante carga/disparo, sem dano de chão ou projétil; o restante
do combate mantém a física original. Coletáveis ameaçados têm corredor protegido;
esta é uma escolha explícita de acessibilidade/balanceamento inicial.

Para jogar esta versão local, execute build-boss/morph-adventure.exe.
A configuração de energia do boss fica em MORPH_COMBAT_ENERGY_DEFAULT.
Não há dependência pendente de outro módulo para compilar esta batalha.
O rebalanceamento posterior está em docs/handoffs/rebalanceamento.md, na mesma branch local. Incluir ambos ao preparar uma futura publicação autorizada.
