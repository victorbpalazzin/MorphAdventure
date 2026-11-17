# Pessoa 2 — personagens, física e colisões

## Entrega e referência

Branch local `migration/player`, a partir da `main` em `05536ef` (build da pessoa 7).
Sem push, PR ou merge. O HTML e os arquivos dos demais responsáveis foram preservados.

Referência lida: todo o JavaScript de `MorphAdventure.html`, especialmente `flap`, `tick`,
`damage`, `drawPlayer`, `collisionRectCircle`, `circlePolygon` e `tickPortal`.
SHA-256 da cópia CRLF: `3AF7952C02BCC1950664726373ABA2C84ADA87BE63B9B306C71406EF801557AF`.
Interfaces consumidas do contrato versão 1, raylib 5.5.

- Etapa 1 do módulo: passarinho, física, feedback de dano e primitivas de colisão implementados e testados isoladamente. O percurso Ar integrado ainda depende dos outros módulos.
- Trabalho independente da etapa 2: seis desenhos vetoriais animados, força ambiental, anel de escudo/ímã e controle de pose pelo portal implementados.
- Validação parcial da etapa 3: testes do módulo e inspeção de duas fases de animação dos seis desenhos, efeitos e hitboxes. Não há aceite de campanha completa, balanceamento ou integração final.

## Arquivos

- `src/player/player.h/.c`: factory `PlayerModuleCreate`, callbacks e estado privado.
- `src/player/player_internal.h`: estado privado compartilhado apenas com o desenho.
- `src/player/player_draw.c`: arte vetorial sem texturas/fontes externas, sem alocação por frame.
- `src/player/collision.h/.c`: círculo-retângulo e círculo-polígono, independentes de raylib.
- `tests/player/`: física, colisões, laboratório visual e extensão CMake já descoberta pelo build da pessoa 7. Não alterei o CMake compartilhado nem testes existentes.

## Integração

Pessoa 1: incluir `player/player.h` e usar `m.player = PlayerModuleCreate()` em sua composição.
Os `.c` diretamente em `src/player/` já são descobertos por `MorphModules.cmake`.
O núcleo só habilita a campanha quando TODOS os módulos reais estiverem presentes.

A factory usa uma única instância privada estática, como permitido pelo contrato. Chamadas
repetidas da factory referem-se à mesma instância; não usar para dois jogos simultâneos.
`init` e `shutdown` não requerem contexto gráfico; `draw` requer janela/render target da raylib.
Nenhum callback altera vidas, score, mundo ou qualquer campo de `GameState`.

`step` consome exclusivamente `input.impulse_pressed`, filtrado pelo core. Não lê teclado,
mouse ou toque. `normal_steps` vale 1 no loop fixo. `world_steps`, `world_scale` e frequência
de renderização não afetam física nem invulnerabilidade. Força ambiental é somada à gravidade
ANTES da integração e do limite superior da velocidade, exatamente como no HTML.

- Inicial: `(180,245)`, velocidade zero, raio visual 23, colisão 18, escala 1.
- Impulso -6.4; gravidade +0.34; máximo de queda 7.2; atualização semi-implícita por passo.
- Se `y < 42`, fixa `y=42` e `vy=1`. Não sinaliza dano no teto.
- Se `y > 487`, fixa `y=487` e sinaliza `touching_floor`; não desconta vida.
- `apply_hit`: velocidade -4.5, invulnerabilidade 75 passos e piscar em blocos de sete passos. O core decide quando chamá-lo, inclusive para escudo absorvido.
- `step` ignora modos fora de PLAY e pose de portal ativa. Pausa não avança física/timers.

Pessoa 6: `PortalPose.rotation` é em **radianos** (acordo local documentado; nenhum tipo
compartilhado foi alterado). Com `overrides_player=true`, posição/escala/rotação são aplicadas
exatamente e física é suspensa. `reset_world` limpa o controle do portal. Ao liberar a pose,
escala volta a 1 e inclinação volta à calculada pela velocidade; não se altera a posição.
O reset no blackout continua sob controle da pessoa 1.

Pessoa 5: desenho consulta `ItemsView.shield` e `boosts[0..boost_count)` para o ímã. Não mantém
timers de boosts próprios; quando ambos estão presentes, anel do escudo tem prioridade,
como no HTML. Asas e pulsação usam `GameState.simulation_steps`, que congela durante pausa.
Desenho não modifica estado nem chama BeginDrawing/EndDrawing.

Pessoa 4: consumir `CollisionCircleRect` e `CollisionCirclePolygon` com a geometria real e
`PlayerView.collision_radius` (18, já descontado do raio visual). Não subtrair cinco novamente.
Tangência EXTERNA não colide (`distância < raio`, sem epsilon), como no HTML. Interior de
polígono colide. Ambos os sentidos dos vértices, concavidade e vértices repetidos são aceitos.
Coordenadas inválidas, raio não positivo, retângulo não positivo, ponteiro nulo ou polígono
com menos de três vértices retornam false. Polígonos devem ser simples; auto-interseções
não são suportadas. Cálculos geométricos usam double internamente para reduzir overflow.

## Validação executada

Windows x64, GCC 16.2.0 UCRT64, CMake 3.31.6, Ninja 1.12.1, raylib 5.5 PREBUILT oficial.
Build Release com `NDEBUG` e `-Wall -Wextra -Wpedantic -Werror`.

```powershell
cmake -S . -B build-player -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR=C:/libs/raylib-5.5_win64_mingw-w64
cmake --build build-player --parallel
ctest --test-dir build-player --output-on-failure
./build-player/tests/player/player-preview.exe --atlas player-atlas.png
```

Substituir o diretório raylib pelo local do pacote. O build normal com FETCH continua
disponível; não foi necessário baixar outra raylib para estes testes.

Resultado: **7/7 testes CTest passaram**, incluindo os cinco anteriores. Novos testes:

- `player-collisions`: tangências de aresta/canto, contato interno, concavidade, vértices
  repetidos, entradas inválidas e comparação retângulo/polígono em grade de 51×51 pontos,
  em ambos os sentidos de vértices.
- `player-physics`: valores analíticos da trajetória; impulso não repetido; teto/chão;
  queda máxima; força positiva/negativa; slow não altera física; hit/75 passos; pausa;
  pose e reset de portal; GameState imutável; trajetória final idêntica para 240 passos
  sob agendas de renderização de 30/60/120/144 Hz. Essas agendas simulam frequências;
  não são medição de FPS real do monitor.

Janela real: `player-preview --atlas` abriu com raylib/OpenGL no Windows, gerou captura e
terminou com código 0. A captura mostra duas fases de asa de cada personagem, escudo,
ímã e piscada de dano com depuração. O próprio laboratório verifica que desenhar não
altera `PlayerView`. O smoke test do executável de diagnóstico original também passou.

O laboratório sem `--atlas` permite voar com Espaço/W/cima/clique, pausar com P, mudar
personagem com 1–6, reiniciar com R e ligar hitbox com H. É uma ferramenta isolada,
sem obstáculos, coleta ou campanha. A depuração começa desativada (`init`); sua API
`PlayerSetDebugHitbox` só altera desenho. Verde indica raio real 18, azul raio visual 23;
os raios de colisão não mudam com a escala decorativa de portal.

## Limitações e próximos passos

- Falta a composição da pessoa 1 com cenários, obstáculos, itens, interface e efeitos reais.
  Não foram simulados módulos em produção para esconder essas dependências.
- Nenhuma partida completa foi jogada. Ainda falta comparar os personagens sobre os fundos
  reais e medir a sensação de voo com obstáculos/perigos finais.
- O desenho preserva vértices, paletas, proporções e fórmulas de animação do HTML. Elipses
  são tesselladas; a sombra borrada Canvas (`shadowBlur=6`) não foi reproduzida. A prévia
  usa MSAA, cuja disponibilidade depende do contexto da janela. Não prometo igualdade de pixels.
- Colisões cobrem polígonos simples, não polígonos auto-intersectantes. Falta exercitar
  a geometria concreta que a pessoa 4 entregar.
- Sem testes em Linux/macOS/MSVC, sanitizers ou medição de desempenho nesta entrega.
- Com `MORPH_BUILD_GAME=OFF`, os testes de colisão continuam disponíveis. Física e prévia
  são adicionadas quando existe `morph::raylib`, pois usam a factory real com seu desenho;
  não foi criado um desenho falso para os testes sem raylib.
- A pendência antiga da borda de mouse em `src/core/viewport.c` pertence à pessoa 1 e
  permanece fora desta entrega.

Pessoa 7: manter os casos em `tests/player/`; acrescentar geometria de obstáculos reais
e testes de campanha quando as dependências chegarem. Pessoa 1: conectar a factory e
validar primeiro Ar jogável antes de declarar etapas 1/2/3 integradas.
