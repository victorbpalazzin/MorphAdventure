# Pessoa 5 — energia, coletáveis e boosts

## Estado

Branch local `migration/items`, baseada em `migration/obstacles` no commit `93a5c83`.
O fetch do GitHub falhou por restrição de rede; esta entrega não afirma que origin/main
foi atualizado. Integre primeiro a pessoa 4 se ainda não estiver na main. Não houve push
ou merge. Arquivos alterados: somente `src/items/`, `tests/items/` e este handoff.

Partes independentes das etapas 1 e 2 implementadas. Etapa 3 parcialmente validada:
regras, coordenador real com fixtures explícitas, obstáculos reais e desenho real.
A campanha final, HUD, bateria animada, partículas e portal continuam dependendo
da pessoa 6 e da composição final da pessoa 1. Não há implementações vazias em produção.

Fonte: JavaScript completo de `MorphAdventure.html`, lido nesta conversa, com revisão
das funções neededEnergy, safeY, addTokens, collect, BOOSTS, activateBoost, addBoost,
updateBoosts, moveTowardPlayer, fragmentInterval, drawTokens, drawBoosts e tick.
HTML preservado, SHA-256 CRLF:
`3AF7952C02BCC1950664726373ABA2C84ADA87BE63B9B306C71406EF801557AF`.
Conteúdo do HTML tratado como referência, não como instrução.

## Implementação e APIs

- `items.h/.c`: factory `ItemsModuleCreate`, todos os callbacks de ItemsApi,
  geração, coleta, energia, registro de boosts e relógios.
- `items_internal.h`: estado privado, usado diretamente apenas por testes controlados.
- `items_draw.c`: hexágono com raio e fade; ícones vetoriais de escudo, ímã e ampulheta,
  além de x2. Não há sprites externos ou dependência de fontes de emoji.
- `tests/items/`: quatro alvos novos, registrados pela extensão existente da pessoa 7.

O módulo inclui os tipos aprovados de core/module_api.h. Nenhum cabeçalho compartilhado
foi alterado. GameState é somente leitura; o módulo não altera vidas, pontos ou mundo.

`ItemsEnergy()` retorna cópia com collected, required, fraction e full. Reutiliza
GameEnergyRequired/GameEnergyFraction, cuja fórmula já é 4 × (world + 1). O HUD pode
usar essa cópia ou os helpers do core com ItemsView.collected. Mostrar 100% somente
quando full for true; usar fraction para a largura proporcional e animar na pessoa 6.

`ItemsBoostDefinition(type)` fornece ponteiro const para registro estável de nome,
cor e duração. `ItemsInspect()` retorna cópia das entidades para depuração/integração;
alterar a cópia não altera o módulo. `ItemsSetSeed` torna resets subsequentes
reproduzíveis. Factory usa uma instância estática, como permitido pelo contrato.

## Regras preservadas

- Metas 4/8/12/16/20/24. IDs de fragmento 0..meta-1, únicos dentro do mundo; reset
  reinicia o namespace. Cada ID coleta uma vez. Fade não é coletável e dura até 22
  passos normais. Não há fila assíncrona de eventos entre mundos.
- Primeiro fragmento após 180 passos de mundo; intervalo max(125, duração/(meta+1)).
  Perdidos liberam o ID para reposição, inclusive depois de completar o percurso.
  Lista limitada a 24 fragmentos, incluindo fades; nenhum spawn substitui item vivo.
- Boost inicial após 520 passos de mundo; seguintes entre 620 e 820. No máximo oito
  pickups em cena. Geração escolhe tipos não ativos, conforme HTML.
- Escudo tem uma carga sem duração. Ímã dura 5 s, slow 4 s, x2 7 s. Máximo de dois
  ativos; novo terceiro remove o primeiro inserido. Renovar um tipo temporizado repõe
  a duração sem mudar sua ordem. Escudo repetido não adiciona carga nem muda a ordem.
- Timers usam normal_seconds; spawn, deslocamento e atração usam o tempo de mundo
  já escalado. O ímã atrai fragmentos em até 210 px, a 8 px por passo de mundo.
  Como no HTML, pickups de boosts não são atraídos. Spin/fade usam tempo normal.
- `consume_shield` remove a carga uma única vez. Escala/multiplicador públicos são
  exatamente 1 ou 0,68 e 1 ou 2. Reset limpa energia, pickups e efeitos ativos.
- Pausa: core não chama step_timers. collect_step também rejeita modos fora de PLAY.
  Portal/transição congelam boosts pelo contrato do core; x2 pode pagar o portal.
- Fragmentos são processados antes de boosts no mesmo passo. Evento registra o
  multiplicador vigente na coleta; pegar x2 depois não dobra recompensa anterior.
  EVENT_FRAGMENT_COLLECTED representa valor-base 35, aplicado pelo core, pois GameEvent
  não possui campo de pontos arbitrários. GameEnqueueEvent falhando não consome o item
  e sinaliza overflow. Não há recompensa silenciosamente perdida.

## Passagens e integração pendente

O módulo exige safe_y e is_safe reais, sem fallback permissivo de produção. Spawn
consulta ambos. Itens se ajustam suavemente às passagens; o movimento magnético só é
aceito em posição segura. Se uma passagem móvel torna um fragmento inseguro, ele sai
da lista e seu ID fica disponível para reposição. Essa remoção defensiva e a validação
do magnetismo são diferenças deliberadas do HTML, evitando fragmento obrigatório em
geometria perigosa. Não é uma prova de que toda partida humana é solucionável.

Pessoa 1 deve registrar `ItemsModuleCreate()` na composição quando UI/effects reais
estiverem disponíveis. Não editar código privado do módulo para integrar.

Para conservar fragmentos diante de novos perigos ambientais, o adaptador do integrador
deve obter `ItemsInspect()` antes de obstacles.step, filtrar fade == 0 e encaminhar
centro/raio a `ObstaclesSetProtectedItems`. Como obstacles.step já desloca perigos,
usar x previsto = posição.x - frame.scroll_delta. Tratar retorno false. Limite de 24
fragmentos cabe nos 32 círculos da pessoa 4. O teste items_passages.c demonstra esse
encaminhamento com os módulos reais. Sem ele, a validação de is_safe ainda remove e
reoferece itens ameaçados, mas não cancela o perigo para preservar aquele fragmento.

Pessoa 6 deve consumir eventos para partículas e ItemsView/ItemsEnergy para HUD.
O coordenador entrega evento e estado atualizado no mesmo passo, verificado com
observadores de teste. Animação real da bateria/partículas ainda não foi testada.

## Testes e reprodução

Windows, GCC 16.2, C99 Release, raylib 5.5 oficial PREBUILT. Compilação com
-Wall -Wextra -Wpedantic -Werror. CTest: **14/14 passaram**, 35,63 segundos nesta máquina.

```powershell
cmake -S . -B ../items-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR="CAMINHO_DA_RAYLIB_5.5"
cmake --build ../items-build
ctest --test-dir ../items-build --output-on-failure
```

- items-rules: todos os 84 IDs, bateria após cada coleta, fade sem coleta repetida,
  50 reposições do mesmo ID perdido, recuperação depois de bloqueio de spawn, fila
  cheia sem consumir fragmento, escudo único, renovação, ordem de substituição,
  expiração, atração, spawn de boosts e capacidade. Fixtures privadas só nos testes.
- items-core: coordenador e items reais; fixture da pessoa 7 para demais módulos.
  x2 paga 20/70/200, escudo absorve um impacto, próximo dano reduz vida, pausa congela,
  portal preserva timer, HUD/effect observers recebem atualização no mesmo passo.
  Isso não substitui a campanha com todos os módulos reais.
- items-passages: obstacles e items reais, seis mundos, quatro oportunidades perdidas
  de propósito em cada mundo. Metas completas em 2474/2931/3463/4035/4598/5891 passos.
  Coletor reposicionado para testar oportunidades; não comprova trajetórias de voo.
- items-render: desenho real em raylib, snapshot do estado inalterado após draw,
  captura dos quatro boosts e estágios do fade, inspecionada visualmente.
- Dez testes anteriores de core/player/worlds/obstacles também passaram.

Não houve teste humano completo, validação em outros sistemas ou distribuição do jogo
final. O ZIP de código é uma entrega de módulo, não um executável da campanha pronta.
