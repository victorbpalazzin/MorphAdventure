# Rebalanceamento local após Grootspectro

Base: trabalho local não commitado em feature/grootspectro. Preservar integralmente
o boss, a narrativa, os testes e o README já modificados. Sem commit, push ou merge.

Decisões antes das mudanças: GameEnergyRequired continua sendo a única fonte da
meta dos mundos, agora 4 + 3 * índice. Capacidade máxima alocada 24 permanece como
limite de armazenamento; não representa a meta. Combate continua com 4 por raio.
Nenhuma assinatura de módulo compartilhado ou versão de contrato muda.
ObstaclesHazard ganha parâmetros próprios de fase/velocidade, somente no módulo
obstacles; snapshot e consumidores recompilados em conjunto. Enum amplia os três
perigos existentes para incluir vento, cipó e rocha. Usar colisores existentes.

Intervalos-base multiplicados por 0,85; velocidades e abertura dos totens mantidas.
Perigos preservam corredor central mínimo inicialmente de 56 px (mais margem),
comparado aos 80 px anteriores, e continuam fora dos totens e círculos de proteção
de itens. O teste de rotas deve verificar as combinações com física real de referência.
Ajustar se não houver rota amostrada; não interpretar amostragem como prova universal.

## Resultado implementado

- Metas centrais 4/7/10/13/16/19; todos os 69 IDs verificados. HUD segue o helper
  existente. Capacidade 24 preservada como limite de armazenamento, sem meta antiga.
- Intervalos-base: 78,2 / 73,1 / 68 / 62,9 / 57,8 / 52,7 passos, fator 0,85 sobre
  92 / 86 / 80 / 74 / 68 / 62. Teste mede também a distância real entre colunas.
  A distância horizontal acompanha velocidade/ritmo e discretização; a redução
  de 15% refere-se ao intervalo-base, não à largura de passagem vertical.
- Ar: redemoinho de vento, espiral animada e oscilação suave; força ambiental existente.
- Água: água-viva com duas frequências verticais e tentáculos animados.
- Floresta: núcleo de cipó enrolado, espinhos e movimento pendular.
- Gelo: estalactite com aviso e queda entre aproximadamente 6,16 e 6,64 px/passo
  de mundo, conforme a variação de velocidade; cessa antes de fechar a rota central.
- Terra: rocha rolante, movimento vertical, rastro de poeira e força ambiental.
- Fogo: bola de fogo com trajetória composta e faíscas decorativas.
- Fase e frequência do movimento variam por semente; geração continua determinística.
  Intervalos ambientais progressivos de 440 a 290 passos foram preservados.
- Avisos duram 85 passos de mundo, sem dano. Movimento durante aviso é contínuo;
  não há salto de posição ao ativar o perigo. Colisores existentes são mantidos:
  polígono no gelo e núcleos circulares visíveis nos demais; halos são decoração.
- Fragmentos: intervalo máximo de 180 passos de mundo (3 s sem slow). Antes,
  dividir a duração pela nova meta menor aumentava a espera para repor perdas;
  o limite evita essa regressão. Metas completas ainda exigem concluir o percurso.
- Vidas, invulnerabilidade, valores de pontuação e boosts não foram alterados.
  A bateria do boss continua com 4 por raio; campanha ainda termina só após derrotá-lo.

## Arquivos desta atualização

Produção: src/core/game.c; src/items/items.c; src/obstacles/obstacles.h,
obstacles.c e obstacles_draw.c. Sem novas dependências ou recursos alocados.

Testes: tests/core/core_check.c e core_rules_test.c; tests/items/items_test.c
e items_passages.c; tests/ui/ui_rules.c; tests/obstacles/obstacles_test.c
e obstacles_preview.c. Documentação: README.md, docs/CONTRACTS.md, este handoff
e nota de continuidade no handoff do boss. Demais alterações locais pertencem
à implementação anterior do Grootspectro e foram preservadas.

## Validação executada

Windows x64 / MSYS2 UCRT64 GCC 16.2 / C99 Release / raylib 5.5 PREBUILT /
CMake 3.31.6 / Ninja. Compilação com warnings como erros. Com ferramentas no PATH:

    cmake --build build-boss --parallel 6
    ctest --test-dir build-boss --output-on-failure

Resultado: **17/17 passaram em 28,13 s**. Depois da extensão do teste de rotas:

    cmake --build build-boss --target obstacles-rules --parallel 6
    ctest --test-dir build-boss -R '^obstacles-rules
 --output-on-failure

Resultado adicional: **1/1 passou em 27,55 s**. Nenhum código de produção mudou
entre essa suíte completa e a extensão da busca; apenas a duração da verificação.

Evidências:
- 5.556 pares de envelopes em 72 combinações semente/mundo, sem fechamento do
  corredor dos totens, sem sobreposição horizontal, com limpeza/recompensa única.
- Cada um dos seis tipos foi observado em aviso e ativo, com colisão e amplitude
  vertical efetiva maior que 12 px. 1.678 amostras em aviso e 1.573 ativas.
- Busca de rotas com física de referência, geometria de produção e nenhum dano:
  duas sementes por mundo, 1.800/2.400/3.000/3.600/4.200/4.800 passos por rota.
  Não usa escudo ou invulnerabilidade para aceitar uma rota.
- Perigos ativos comparados com 68 passos normais versus 100 a 0,68, sem escala
  duplicada. Para ativação discreta da queda, tolerância de no máximo um passo
  de velocidade; demais posições com tolerância menor. Pausa conserva snapshot.
- Coleta dos 69 IDs sem duplicação, bateria parcial abaixo de 100%, 50 reposições
  de um ID perdido e retomada após posição temporariamente insegura.
- Obstáculos/itens reais: quatro perdas propositais em cada mundo; todos concluíram
  suas metas (1994/2157/2683/3750/3740/5351 passos). Coletor reposicionado nesse
  teste verifica disponibilidade de oportunidades, não habilidade de voo.
- Core e UI verificam portais, vidas persistentes, escudo, invulnerabilidade,
  pontuação e pausa. A batalha automatizada real do boss ainda termina em vitória
  com 12 coletas, 3 raios, 3 vidas e 420 pontos, em 2.997 passos.
- Capturas reais dos seis cenários com perigos ativos inspecionadas. Testes gráficos
  também verificam avisos e desenho sem mutação. Nenhum asset externo foi adicionado.

## Limitações e publicação

Não foi realizada uma campanha humana completa nem validação em Linux/macOS.
As rotas são amostras de sementes e velocidades-base, não prova universal para
qualquer combinação de entrada, ritmo e boosts. O balanceamento precisa de teste
humano; não declarar dificuldade equilibrada somente com os testes automatizados.

Para jogar a compilação atual, usar build-boss/morph-adventure.exe. O executável
antigo em build/ não foi substituído. Logs completos e capturas estão também em
outputs/rebalanceamento na raiz do workspace.

**Sem commit, push, merge ou Release.** Ao preparar uma publicação autorizada,
incluir também Grootspectro e seus arquivos novos/testes — lembrete pedido pelo usuário.

