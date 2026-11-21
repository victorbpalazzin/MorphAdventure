# Correção dos projéteis e perigos horizontais — 19/11/2026

Causas verificadas: BossStep descartava tiros quando energy.present, x<620 e
|y-energia.y|<64. Perigos dos mundos dependiam de anchor_id, eram removidos por
sobreposição com colunas/itens e por age>=360. Nenhum desses filtros será usado
para apagar projéteis já lançados. Regras de término: corpo completamente fora
à esquerda; reset/troca de modo; megarraio dissipa tiros do boss explicitamente.

Contrato v3: BossProjectile ganha posição anterior e marca de contato consumido;
BossState ganha controle da sequência. BossRegisterContact é chamado pelo core
somente após impacto válido (inclusive escudo). Factories mantêm assinaturas.
Colisão varrida reutilizável fica em player/collision; boss e obstáculos usam-na.
ObstaclesHazard troca âncora por id próprio, origem vertical e posição anterior.
Configuração de velocidade, intervalo, aviso, oscilação e limite simultâneo por mundo.

Geração avalia envelopes dos totens previstos na chegada ao jogador; pode adiar
o lançamento, mas nunca apagar um perigo em voo para abrir passagem. Itens usam
as consultas existentes e continuam sendo repostos. Perguntas/letras não foram
encontradas nesta cópia de src/; não afirmar validação de recursos ausentes.
Sem push, merge, commit ou publicação. Grootspectro e rebalanceamento preservados.


## Implementação local

- Boss: disparos ajustados para trajetórias horizontais nas alturas anunciadas, 1/2/3
  por fase. Fase 3 usa sequência de 18 passos entre tiros; velocidades e intervalos
  centralizados em src/boss/boss.c. Cada disparo pode causar um único impacto válido.
  Alturas 185–420, separação de 110 px: projéteis não passam atrás do HUD do boss.
- Seis perigos: velocidades próprias 6,2–9,2 px/passo (±4%), intervalos 300–200,
  avisos 75–62, oscilações limitadas a 2–6 px. Os parâmetros estão em
  ObstaclesHazardSettings, src/obstacles/obstacles.c, e na tabela do contrato v3.
- Totens permanecem intactos. Na geração, prevê-se a região de encontro com o
  jogador, usando envelopes dos totens; uma faixa livre de ao menos 52 px é
  reservada. Geração inadequada é adiada. Nada remove um perigo lançado para
  proteger item, totem ou personagem.
- Colisão por segmento evita perder contato quando as posições finais não se
  sobrepõem. Escudo, dano e invulnerabilidade continuam no coordenador.
- Efeitos existentes preservados, gelo orientado na horizontal com contorno de
  geada; perigos desenhados depois dos totens. Nenhum asset externo adicionado.

## Arquivos desta correção

CMakeLists.txt; README.md; docs/CONTRACTS.md; este handoff;
src/core/game.c e game_types.h; src/boss/boss.c e boss.h;
src/player/collision.c e collision.h; src/effects/boss_visual.c;
src/obstacles/obstacles.c, obstacles.h, obstacles_internal.h e obstacles_draw.c;
tests/core/core_check.c e core_rules_test.c; tests/player/collision_test.c;
tests/boss/boss_rules.c; tests/obstacles/obstacles_test.c e obstacles_preview.c;
tests/ui/ui_lab.c.

O worktree já continha a implementação do boss e o rebalanceamento sem commit.
Essas alterações anteriores foram preservadas. A lista de git status inclui ambas
as entregas e não deve ser interpretada como arquivos alterados só nesta correção.

## Validação e comandos

Build local: Ninja, GCC/UCRT64, Release C99, raylib 5.5 PREBUILT, módulos reais ON,
-Wall -Wextra -Wpedantic -Werror. O executável anterior estava bloqueado pelo
Windows; os binários novos ficam em build-boss/horizontal para não sobrescrevê-lo.

No terminal já configurado com CMake/Ninja/GCC no PATH, dentro do worktree:

    cmake -S . -B build-boss -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH="<worktree>/build-boss/horizontal"
    cmake --build build-boss --parallel 6
    ctest --test-dir build-boss --output-on-failure

O cache existente fixa MORPH_MODULES_LINKED=ON, MORPH_RAYLIB_PROVIDER=PREBUILT e
MORPH_RAYLIB_DIR=../deps/raylib-5.5/raylib-5.5_win64_mingw-w64 (caminho absoluto no cache).

Casos acrescentados: existência até sair totalmente à esquerda, passagem por x180,
energia presente sem apagar tiro, contato consumido, varredura rápida, alturas
visíveis e separadas, perigos sem qualquer totem, travessia de coluna sólida,
proteção de item sem apagar perigo, pausa/slow e progressão de parâmetros.
Rotas: 12 percursos (sementes 20/21 × seis mundos), física de referência, sem dano
permitido, incluindo duração-base +300 passos. Geração adicional: 72 cenários,
5000 passos cada. São amostras, não garantia de todas as combinações.

O primeiro teste integrado revelou derrota do piloto automático antigo na fase 3.
O piloto de teste foi substituído por previsão de trajetórias usando somente
impulso/J; não se alteraram vidas, dano ou energia para forçar vitória. A arena
inicial é preparada explicitamente, e a luta usa módulos reais até o resultado.
Capturas reais por raylib/OpenGL estão em build-boss/tests/obstacles/captures e
build-boss/tests/ui/captures. O laboratório de cenários usa pose controlada apenas
para capturar arte; não é evidência de uma partida humana completa.

## Limitações

Balanceamento humano ainda precisa de avaliação. Não foi feita campanha inteira
manualmente. Perguntas de lógica e coleta de letras não existem neste src; não
foram inventadas nem declaradas validadas. Fragmentos, boosts e portais existentes
foram mantidos. Sem push, merge ou release: incluir boss e estas correções quando
o usuário autorizar a publicação futura.


## Resultado final observado

19/11/2026: build concluído; 17/17 testes passaram em 37,37 s. Todas as 12
rotas amostradas mantiveram trajetórias sobreviventes. Combate integrado completo:
3240 passos, três fases, três vidas restantes, 420 pontos, GAME_WON. Não houve
reposição artificial de vidas/energia durante essa luta. O piloto é automatizado,
não uma validação humana do balanceamento. Executável principal iniciado com
--smoke-frames 5 encerrou com código 0. Revisadas capturas reais dos seis perigos
e do tiro do boss já à esquerda do personagem. Logs preservados em
build-boss/validation-horizontal/ e capturas nos diretórios acima.

Executar a versão nova, a partir do worktree no PowerShell do VS Code:

    .\build-boss\horizontal\morph-adventure.exe

A configuração local de saída separada está no cache, não exige alteração no
CMake dos colegas. Em outro build, o executável segue o destino normal configurado.
