# Telas narrativas — 21/11/2026

## Escopo

Abertura e vitória com seis personagens, conforme escolha explícita do usuário.
Composição vetorial original do projeto, sem imagens geradas ou texturas externas.
A referência fornecida de Grootspectro e sua adaptação já existente foram preservadas:
tronco retorcido, raízes, garras, galhos, folhas escuras e olhos/fissuras violetas.

Abertura: luar, floresta escura em camadas, névoa violeta, boss gigantesco,
heróis agrupados com olhos assustados, sobrancelhas de preocupação e suor.
Vitória: mesmo enquadramento e posições do grupo, céu quente, vegetação restaurada,
boss tombado com olhos apagados, heróis sorrindo/saltando e partículas coloridas.
A área de leitura e botões fica separada da ilustração; resolução lógica 960×540.

## Arquivos

- src/ui/narrative.c (novo): paisagens e composição das duas telas, textos completos.
- src/ui/ui.c, ui_draw.c, ui_internal.h: hitboxes, botões e relógio de apresentação.
- src/player/player.h, player_draw.c: retratos reutilizando a arte de gameplay,
  com expressões exclusivas das cenas; sem mudar o personagem jogável.
- src/effects/boss_visual.h, boss_visual.c: retratos do mesmo boss, em pé/tombado,
  preservando o render de combate pela função existente.
- src/core/game.c: avanço somente de apresentação em INTRO/WON, com foco e limite
  de delta; não atualiza gameplay nem permite vitória antecipada.
- tests/ui/ui_rules.c, ui_lab.c: botões, relógio, imutabilidade de estado e capturas.
- docs/CONTRACTS.md: APIs e comportamento documentados.

## Validação

Comandos na pasta work/ui-work, com ferramentas UCRT64/CMake/Ninja no PATH:

    cmake --build build-boss --parallel 6
    ctest --test-dir build-boss --output-on-failure

Build integrado com raylib 5.5, C99, Release, avisos tratados como erro.
Capturas reais: build-boss/tests/ui/captures/01-inicio.png e 09-vitoria.png,
além das janelas 480×270, 800×600 e 1280×720. O teste integrado percorre portais,
verifica boss obrigatório e três megarraios, com preparação explícita de energia;
a luta automatizada separada usa coleta e voo reais.

## Limitações

Arte vetorial adaptada ao estilo existente; não é uma pintura raster definitiva
nem cópia fotográfica da referência. Não há novas texturas, fontes ou dependências
para distribuir/liberar. Letras, perguntas e ressurreição por desafio lógico não
existem neste checkout; não foram implementadas nem declaradas testadas.
Nenhum push, merge ou release autorizado/realizado.

## Resultados observados

Suíte completa: 17/17 passaram em 38,09 s. Após os ajustes finais de brilho,
dica de controle e remoção do layout narrativo antigo, ui-rules/ui-flow/boss-rules
passaram novamente (3/3, 3,77 s). Executável principal abriu e encerrou com código 0
no smoke de cinco quadros. Captura real da abertura: build-boss/narrativa-jogo.png.
Revisadas visualmente abertura, vitória e versão 480×270; também foram produzidas
capturas 800×600 e 1280×720. A interface permanece dentro das áreas lógicas.
Combate automatizado real: 3240 passos, três vidas, 420 pontos e GAME_WON.

Os testes verificaram clique em Iniciar Aventura, comandos de Jogar Novamente
e Menu Principal, animação de apresentação sem alterar GameState, pureza do desenho,
pausa, transições dos seis portais e vitória apenas depois dos três disparos no boss.
Logs: build-boss/narrative-tests-full.log e build-boss/Testing/Temporary/LastTest.log.
Executável final: build-boss/horizontal/morph-adventure.exe.
Branch local confirmada: feature/grootspectro. Nenhum commit/push/merge nesta entrega.
