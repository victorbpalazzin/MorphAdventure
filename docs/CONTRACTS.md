# Contrato de integração — Morph Adventure / C99 / raylib 5.5

**Versão 6. Grootspectro, perigos horizontais e modo educativo (opcional) integrados localmente.** Os seis módulos reais estão conectados. O núcleo coordena a campanha e a batalha final. Os cabeçalhos de src/core/ e as extensões ao final deste documento são a fonte de verdade; as notas das entregas iniciais abaixo registram a origem da arquitetura.

## Referência e escopo

- Base Git: `75fcb07` de `origin/main`, com `MorphAdventure.html` e README já enviados pelo grupo. A branch local desta entrega é `migration/core`.
- HTML preservado: `MorphAdventure.html`. SHA-256 observado: `3AF7952C02BCC1950664726373ABA2C84ADA87BE63B9B306C71406EF801557AF` (bytes da cópia local; conversão CRLF/LF por Git pode mudar esse hash).
- Alvo inicial: desktop Windows x64; C99; **raylib 5.5** fixada. Não usar “latest” nem versões diferentes entre colegas. A pessoa 7 pode preparar CMake com uma dependência oficial fixada e download na primeira configuração.
- Não há migração de arte/jogabilidade neste núcleo. O executável sem módulos mostra uma tela de diagnóstico explícita; não tem módulos vazios disfarçados de jogo.
- Consultadas as APIs oficiais de [raylib](https://www.raylib.com/cheatsheet/cheatsheet.html) e o [release 5.5](https://github.com/raysan5/raylib/releases/tag/5.5). A validação compilou contra os cabeçalhos reais de 5.5.

## Propriedade e limites

| Pessoa | Arquivos | Responsabilidade |
|---|---|---|
| 1 | `src/main.c`, `src/core/`, este contrato | Entrada, tempo, estados, vidas, pontos, coordenação e composição |
| 2 | `src/player/` | Física, personagens, hit feedback e primitivas de colisão |
| 3 | `src/worlds/`, `assets/worlds/` se usado | Cenários, parallax, recursos visuais decorativos |
| 4 | `src/obstacles/` | Configurações dos seis mundos, obstáculos, geometria, perigos e passagem segura |
| 5 | `src/items/` | Fragmentos, boosts, energia e seus timers |
| 6 | `src/ui/`, `src/effects/` | Interface, partículas, portais e fade |
| 7 | `CMakeLists.txt`, `cmake/`, `tests/`, scripts de pacote | Build, regressão e distribuição |

O estado central é mutável apenas pelo coordenador. Callbacks recebem `const GameState *`; nunca remover `const`, guardar o endereço da entrada temporária nem editar vidas/pontos/mundo por conta própria. Cada módulo mantém seu estado privado através de `void *context`. Não copiar tipos do contrato para outros cabeçalhos. Mudanças incompatíveis exigem acordo e incremento de `MORPH_CONTRACT_VERSION`.

Meta dos mundos: **GameEnergyRequired(world)** consulta a tabela, de 0 a 5: **4/6/8/10/12/16**. Incremento de 2 até Terra e de 4 no Fogo, conforme sequência solicitada. A capacidade alocada é um teto, não a meta.

Limites de interface: 6 mundos, capacidade para 24 fragmentos, 2 boosts ativos, 128 eventos por passo. Capacidade excedida da fila causa erro explícito; não descartar recompensa silenciosamente. Pessoa 4 e pessoa 5 devem limitar suas próprias listas, reciclar objetos fora da tela e registrar os limites de entidades no handoff.

## Arquivos do núcleo e como conectar módulos

- `game_types.h`: tipos compartilhados, enums e constantes; não depende da raylib.
- `module_api.h`: todas as assinaturas de callbacks e `GameModules`; fonte de verdade das APIs.
- `game.h`/`game.c`: coordenador e validação de contrato.
- `viewport.h`/`viewport.c`: escala e conversão de entrada, testáveis sem janela.
- `main.c`: adaptador raylib; coleta entrada, renderiza em textura lógica 960×540, mantém proporções.

Cada responsável cria o próprio `.h/.c` com a seguinte factory pública, incluindo `core/module_api.h`:

```c
/* src/player/player.h */
PlayerApi PlayerModuleCreate(void);
/* src/worlds/worlds.h */
WorldsApi WorldsModuleCreate(void);
/* src/obstacles/obstacles.h */
ObstaclesApi ObstaclesModuleCreate(void);
/* src/items/items.h */
ItemsApi ItemsModuleCreate(void);
/* src/ui/ui.h */
UiApi UiModuleCreate(void);
/* src/effects/effects.h */
EffectsApi EffectsModuleCreate(void);
```

A factory fornece uma tabela completa de callbacks e contexto com vida útil maior que `Game`. Não desenhar, carregar textura nem abrir janela na factory. `life.init(context)` aloca/carrega os recursos depois de `InitWindow`; `life.shutdown` libera após a última renderização, antes de `CloseWindow`. Uma factory pode usar uma estrutura privada estática para a única instância de jogo; testes que exigirem múltiplas instâncias devem receber contextos independentes.

Pessoa 1 escreverá `src/core/composition.c` quando as entregas reais estiverem disponíveis:

```c
GameModules MorphCreateModules(void) {
    GameModules m = {0};
    m.contract_version = MORPH_CONTRACT_VERSION;
    m.player = PlayerModuleCreate();
    m.worlds = WorldsModuleCreate();
    m.obstacles = ObstaclesModuleCreate();
    m.items = ItemsModuleCreate();
    m.ui = UiModuleCreate();
    m.effects = EffectsModuleCreate();
    return m;
}
```

O build integrado habilitará `MORPH_MODULES_LINKED` e compilará essa composição e os módulos. Sem essa macro, a janela informa todos os módulos pendentes e não permite começar uma falsa partida. Não criar implementações vazias para fazer `ready=true`. Composição parcial não é campanha integrada; cada colega pode testar o próprio módulo num executável isolado preparado com a pessoa 7.

## Ciclo de vida e falhas

`GameInit` recebe as tabelas por valor. Verifica versão e todas as funções obrigatórias. Se faltam módulos, retorna `true` para a janela de diagnóstico, com `ready=false` e bits em `missing_modules`. Só inicializa módulos quando todas as tabelas estão completas. Inicializa na ordem player/worlds/obstacles/items/ui/effects. Libera na ordem inversa.

`init` que falha deve desfazer sua própria inicialização parcial; o core libera apenas os módulos anteriores que terminaram com sucesso. `GameShutdown` pode ser chamado de novo sem dupla liberação.

Pessoa 4 fornece `config(context, world)` para todos os índices 0..5, mesmo que a etapa 1 só tenha o Ar desenhado. O ponteiro retornado é estável até o shutdown. Configuração inválida, fila excedida ou estado impossível dos itens levam a `GAME_ERROR`, com mensagem visível; não progredir silenciosamente.

## Tempo, entrada e coordenadas

- Simulação fixa: `MORPH_STEP_SECONDS = 1/60`. `FrameContext.normal_steps = 1`; use valores por passo do HTML para física.
- `normal_seconds = ui_seconds = 1/60`. Boosts duram segundos normais. `world_steps = 1 × world_scale`; `world_seconds = normal_seconds × world_scale`.
- Escala normal é 1; slow motion é 0,68. O core atualiza timers dos itens antes de consultar a escala, e aplica a escala uma vez. Física do personagem usa `normal_steps`, não `world_steps`.
- O core integra `speed`, `scroll` e `world_steps`. `distance/minimum_distance` representa a fração coletada da bateria, sincronizada antes do ritmo e após a coleta; não exige tempo ou distância adicional. Obstáculos/itens movem suas posições com `frame.scroll_delta`, sem acrescentar de novo ao deslocamento global. Cenários consultam `state.scroll`.
- `frame.pace` é o fator de ritmo calculado uma vez pelo coordenador. Pessoa 4 usa `config.obstacle_interval / (0.98 + (frame.pace - 1) × 0.55)` no spawn e não recalcula outro relógio de ritmo.
- Trava longa: o acumulador limita o atraso recebido a 0,1 s e executa no máximo cinco passos por frame; excesso de atraso é descartado para evitar teleporte/espiral de atraso. Isso não é promessa de manter tempo de parede quando o computador não acompanha.
- Perder foco pausa gameplay, portal ou transição; voltar exige P/Continuar. Nenhum tempo acumulado é reaplicado. `ui.input` continua disponível na pausa, mas callbacks de atualização não.
- Impulso vem de borda de Espaço/↑/W, ou clique/toque inicial na área lógica, e permanece pendente até o próximo passo. É consumido uma vez, mesmo com vários passos no mesmo frame.
- `ui.input` retorna `UiDecision`: um comando e se consumiu o clique. Tecla/comando de início/pausa/reinício não executa física naquele frame. Um clique em botão não vira impulso.
- `GameInput.pointer` já está em pixels lógicos. Consulte `pointer_inside`; barras pretas e bordas externas não são cliques de jogo. Não aplicar conversão de escala outra vez.
- O protótipo HTML usa repetição de eventos do navegador; a base C usa acionamento por borda deliberadamente para não ligar a frequência de impulso ao sistema operacional. Uma política de repetição futura deve ser acordada e testada, não adicionada isoladamente.

## Ordem de um passo de gameplay

1. Ações da UI/teclado e perda de foco são resolvidas antes do acumulador.
2. Atualizar timers de boosts no tempo normal, consultar escala e multiplicador.
3. Sincronizar percurso com a fração de energia e atualizar ritmo, velocidade, deslocamento contínuo e relógio do mundo.
4. Atualizar obstáculos/ambiente com a posição anterior do jogador, geometria móvel e permissão de spawn.
5. Atualizar física com input pendente e força ambiental; ler `PlayerView` atualizado.
6. Drenar obstáculos ultrapassados e aplicar pontos uma única vez.
7. Se chão ou perigo toca jogador, processar um impacto: respeitar invulnerabilidade → consumir escudo → aplicar knockback/invulnerabilidade → descontar vida apenas sem escudo. A derrota interrompe o passo antes de coleta/portal.
8. Coletar fragmentos e boosts; processar fila de eventos em ordem.
9. Reconsultar energia e avaliar portal. Atualizar efeitos, cenário e UI sem duplicar regras.

`take_passed` esvazia o contador toda vez que é chamado; o core não sabe os IDs de obstáculos. `collect_step` atualiza IDs/energia no módulo items e enfileira exatamente um `EVENT_FRAGMENT_COLLECTED` por coleta válida. `event.multiplier` registra 1 ou 2 no instante da recompensa: um boost pego depois no mesmo passo não dobra retroativamente uma coleta anterior. O core aplica os valores 10/35/100; módulos não recebem pontos arbitrários.

## Responsabilidade detalhada dos módulos

### Player / pessoa 2

`step` recebe força ambiental; aplica física e reduz invulnerabilidade em tempo normal. `view` retorna estado sem alterá-lo. `touching_floor` sinaliza contato danoso ao coordenador. `apply_hit` aplica velocidade -4,5 e 75 passos de invulnerabilidade, inclusive quando escudo absorve. `set_portal_pose` só aplica posição/escala/rotação se `overrides_player=true`. Quando o portal libera o controle, preserve a posição inicial do novo mundo e retome escala 1.

`reset_world` coloca o personagem em (180,245), velocidade 0, escala 1, raio visual 23 e raio de colisão 18. `draw` consulta boosts para escudo/ímã, sem manter timers duplicados.

Primitivas a implementar em `src/player/collision.h/.c`:

```c
bool CollisionCircleRect(GameVec2 center, float radius, GameRect rect);
bool CollisionCirclePolygon(GameVec2 center, float radius,
                            const GameVec2 *vertices, size_t count);
```

`GameRect` usa largura/altura positivas; polígonos são simples, na ordem das arestas, com pelo menos três pontos, em coordenadas lógicas. Teste arestas/interior e documente contato tangente. A implementação HTML usa distância estritamente menor que o raio.

### Worlds / pessoa 3

`step` mantém animação decorativa; `draw` só desenha, usando `state.scroll` para parallax. Recursos privados são criados no init/reset e liberados corretamente. Não criar obstáculos ou causar dano. Paletas compartilhadas podem ser expostas por cabeçalho próprio após acordo com pessoa 4/6, sem alterar o core.

### Obstacles / pessoa 4

Tabela da versão atual:

| Mundo | speed | gap | interval | motion | frequency | duration_steps | event_interval | force |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Ar | 4.2 | 210 | 78.2 | 12 | .019 | 1500 | 440 | .065 |
| Água | 4.6 | 202 | 73.1 | 16 | .021 | 2100 | 410 | .080 |
| Floresta | 5.0 | 194 | 68 | 20 | .023 | 2700 | 380 | 0 |
| Gelo | 5.4 | 186 | 62.9 | 24 | .025 | 3300 | 350 | .090 |
| Terra | 5.8 | 178 | 57.8 | 28 | .027 | 3900 | 320 | .105 |
| Fogo | 6.2 | 170 | 52.7 | 32 | .029 | 4500 | 290 | 0 |

`step(..., spawn)` só gera obstáculos/eventos especiais quando `spawn=true`. Entidades existentes continuam saindo. No modo PORTAL, apenas limpeza visual/deslocamento: cessar perigos, sem novo dano. Use `frame.scroll_delta` inclusive nesse modo (já reduzido a 35% pelo core).

`touches_player` verifica perigos já ativos, nunca os que ainda estão em aviso. Deve ser consulta sem consumir vidas/escudos. `path_clear` confirma que não há mais obstáculo perigoso à frente/atravessando a hitbox; o modo portal assume trajeto seguro. `passages` oferece consultas sem mutação com contexto válido por todo o `collect_step`, sem retornar ponteiros para variáveis de pilha. As formas desenhadas e colisões devem coincidir.

### Items / pessoa 5

`view` é consulta pura. Quantidade não pode exceder `GameEnergyRequired(world)`; multiplicador deve ser exatamente 1 ou 2; escala deve ser 1 ou 0,68. `boosts[]` usa até dois elementos, com tipo/segundos restantes (escudo usa valor negativo para indicar carga sem cronômetro). `shield` acompanha a carga real.

`step_timers` usa segundos normais. `collect_step` usa deslocamento/tempo de mundo para geração, movimento e magnetismo conforme a referência, mantém IDs únicos e reposição de perdidos. Só emite `EVENT_FRAGMENT_COLLECTED` e `EVENT_BOOST_COLLECTED`; o core emite outros eventos de feedback. Use `GameEnqueueEvent`; propague `overflow` se falhar. `consume_shield` retorna true uma única vez e remove o efeito. Nenhum callback altera vidas/pontos/mundo.

`reset_world` limpa energia/itens/boosts. Portal e transição congelam timers dos boosts como no HTML; x2 presente ao entrar no portal vale para +100. Regras: escudo uma colisão, ímã 7 s, slow 4 s, x2 7 s; máximo dois; novo terceiro substitui o ativo mais antigo; renovar tipo existente não duplica efeitos.

### UI e Effects / pessoa 6

UI transforma cliques em comandos; ações são retornadas ao core. Para derrotado/vencedor, “Tentar/Jogar novamente” usa RETRY; reinício global usa RESTART e volta ao início. `step` anima bateria/score/HUD, `draw` não modifica estado.

Effects recebe eventos de feedback e atualiza partículas com limite de capacidade. `begin_portal` começa a animação; `step_portal` controla seu relógio e retorna sinais na sequência:

1. `PORTAL_SIGNAL_ENTERED` quando o personagem entra: o core paga +100 com x2 e muda PORTAL → TRANSITION.
2. `PORTAL_SIGNAL_BLACKOUT` no instante opaco: se mundo 5, core inicia GAME_BOSS com as vidas e boosts atuais. Caso contrário, incrementa mundo uma vez e reseta player/worlds/obstacles/items/ui, **preservando vidas e score**.
3. `switch_world_under_fade` limpa partículas/atualiza paleta preservando a animação de fade; não reiniciar o timer do portal. Em seguida, `portal_pose` deve liberar o controle do jogador (`overrides_player=false`).
4. `PORTAL_SIGNAL_FADE_DONE` ao revelar o novo mundo: core muda TRANSITION → PLAY.

Retorne cada sinal uma vez; repetições de ENTERED/BLACKOUT são protegidas pelo core, mas a pessoa 6 não deve depender disso para avançar timers. `life.reset_world` faz reset completo apenas no início de uma partida/mundo fora do fade preservado. Pausar suspende todos esses callbacks de tempo e retoma o modo exato.

Ordem de desenho: cenário → obstáculos → itens → portal → personagem → partículas → HUD/telas → camada de transição. A camada final de fade cobre HUD na troca de mundo. Em `GAME_WON`, o core não chama `draw_transition`: o blackout final não pode esconder a tela de vitória, pois seu relógio já parou. Nas demais fases, inclusive pausa durante transição, o callback continua disponível. Todo desenho acontece dentro do render target lógico iniciado por `main.c`; os módulos não chamam `BeginDrawing`/`EndDrawing` nem abrem uma segunda janela.

## Máquina de estados

```text
INTRO --START--> PLAY
PLAY --vidas=0--> LOST --RETRY--> PLAY (nova partida)
PLAY --bateria completa + caminho livre--> PORTAL
PORTAL --ENTERED--> TRANSITION
TRANSITION --BLACKOUT--> próximo mundo, ainda em TRANSITION
TRANSITION --FADE_DONE--> PLAY
TRANSITION --BLACKOUT no Fogo--> BOSS (introdução → combate → derrota do boss)
BOSS --terceiro raio + animação de derrota concluída--> WON --RETRY--> PLAY (nova partida)
PLAY/PORTAL/TRANSITION/BOSS --P ou perda de foco--> PAUSED
PAUSED --P ou RESUME--> modo anterior, sem salto de tempo
qualquer modo normal --RESTART--> INTRO (nova partida)
violação de contrato --> ERROR (diagnóstico; requer correção do módulo)
```

`NewRun` é interno: restaura três vidas, zera score e inicia no Ar. `BeginWorld` é interno: preserva vidas/score e reseta dados de fase. Nenhum módulo deve chamar resets de outros módulos.

## O que continua pendente

- Factories e implementações reais das pessoas 2–6; registro em composition.c pela pessoa 1 quando recebidos.
- Build CMake/dependência automática e testes integrados pela pessoa 7.
- Arte/paridade visual, jogabilidade completa, som se futuramente solicitado, balanceamento humano e distribuição final.
- Teste real em outros sistemas e celulares. A base desta entrega não promete suporte nativo mobile.

Para a pessoa 7, os comandos exatos já executados, o teste puro C e o status desta entrega estão em `docs/handoffs/pessoa-1.md`.

## Extensão v2: Grootspectro

Implementação central documentada em docs/handoffs/grootspectro.md. Os seis módulos
reais já estão conectados por composition.c quando MORPH_MODULES_LINKED=ON.
Os trechos anteriores de entregas pendentes são contexto histórico da versão 1.

GameInput.attack_pressed é uma borda de J, enfileirada até um único passo fixo,
limpa em pausa/foco/reset. GameState.boss contém dados limitados (24 projéteis,
um coletável) sob controle do core. Nenhuma assinatura de factory mudou. Recompilar
juntos todos os módulos: MORPH_CONTRACT_VERSION=5 impede misturar tabelas antigas.

BossBegin/Step/Fire/Touches são helpers puros em boss/. CombatEnergyReset/Step/Fraction
pertencem a items/ e mantêm somente a energia de combate. A bateria convencional
não é reiniciada nem usada para atacar. MORPH_COMBAT_ENERGY_DEFAULT=4 configura cada
carga; há três segmentos. Coleta paga 35 através de Reward, com x2 vigente.

No blackout do Fogo, reset apenas de pose, obstáculos e efeitos. Não repor vidas,
nem pontuação, boosts ou energia de progressão. Introdução dura 180 passos normais;
física/dano suspensos. Combate usa física normal, dano/invulnerabilidade/escudo centrais;
boosts expiram em tempo normal. Projéteis e coletáveis usam world_steps já escalados.
O cenário desacelera suavemente. Pausa/foco congelam todos os relógios.

Raio: carga visual 35 passos, impacto único, término no passo 85; jogador paira
durante o disparo e projéteis são dissipados. A bateria esvazia ao terminar, não
permite segundo disparo durante a animação. Terceiro impacto inicia 210 passos de
desintegração, sem ataques; só então GAME_WON. Retry/restart apagam todo estado do boss.
Desenho de effects/UI é somente leitura, sem alocação dinâmica ou assets adicionais.

## Rebalanceamento dos percursos

Intervalos multiplicados por 0,85, mantendo velocidades, aberturas e ritmo central.
Perigos: redemoinho, água-viva, massa de cipós espinhosos, estilhaço de gelo,
rocha e bola de fogo. O comportamento horizontal da extensão v3 abaixo substitui
as antigas âncoras, queda vertical e remoções para proteger itens/passagens.
Totens adjacentes preservam o envelope comum mínimo de 80 px.

Reposição de fragmentos dos mundos: primeiro após 180 passos de mundo; intervalo
seguinte limitado a 180 passos (mínimo 125), para que diminuir a meta não aumente
a espera. O slow afeta esse relógio uma única vez; não altera a bateria do boss.

## Extensão v3: trajetórias horizontais e contato consumido

BossProjectile inclui previous e spent. BossRegisterContact é chamado pelo core
após Damage aceitar o impacto, inclusive absorção por escudo. Invulnerabilidade
rejeita dano; o tiro continua visível. Colisão usa CollisionSweptCircle, compartilhada
com obstacles, para testar o segmento do passo contra o círculo do jogador.
BossState inclui burst_remaining/burst_index/burst_timer para a sequência da fase 3.
Nenhuma assinatura de factory foi alterada. Recompilar todos os módulos juntos.

Tiros do boss mantêm a altura anunciada por 65 passos de mundo. Velocidades-base
3,8/4,6/5,4 px/passo, intervalos-base 205/155/115 passos, respectivamente 1/2/3 tiros;
na fase 3 os tiros são separados por 18 passos. Variação nas fases 2/3: até +0,4.
Alturas ficam entre 185 e 420 px, separadas por 110 px, abaixo do HUD.
BossFire dissipa tiros explicitamente; fora disso, remoção somente com x+raio<0
ou reset/fim da batalha. Energia presente nunca apaga tiros.

ObstaclesHazard usa id próprio, previous, base_y e speed; não possui âncora.
ObstaclesHazardSettings(world) oferece velocidade, intervalo, oscilação, aviso e
limite simultâneo. Todos usam world_steps uma única vez; física permanece normal.
Primeiro lançamento após 180 passos, com tentativas adiadas em 20 quando não há
espaço. Geração considera envelopes dos totens previstos na chegada ao jogador,
reserva ao menos 52 px no lado livre e evita item protegido na origem. Após lançado,
o perigo cruza totens/itens e não é apagado por eles, por idade ou pela posição do
jogador. Somente saída completa à esquerda ou reset/transição o remove.
As consultas de coleta verificam terreno/totens; perigos móveis não invalidam itens.

| Mundo | px/passo | intervalo | aviso | oscilação px | máximo |
|---|---:|---:|---:|---:|---:|
| Ar | 5,58 | 510 | 75 | 4 | 2 |
| Água | 6,12 | 510 | 72 | 6 | 2 |
| Floresta | 6,66 | 510 | 70 | 5 | 2 |
| Gelo | 7,20 | 510 | 68 | 2 | 2 |
| Terra | 7,74 | 510 | 65 | 4 | 3 |
| Fogo | 8,28 | 510 | 62 | 6 | 3 |

Velocidade dos perigos varia ±4% por lançamento. Parâmetros em obstáculos.c.
A verificação de rotas amostradas não prova que toda combinação futura é evitável;
balanceamento humano permanece necessário. Ver handoffs/projeteis-horizontais.md.

## Percurso ligado à bateria

O percurso acompanha exatamente GameEnergyFraction: 0% sem coleta, progresso proporcional
a cada fragmento e 100% na coleta do último. Não existe espera por distância mínima.
Novos obstáculos param após completar a bateria; os já lançados continuam até liberar
o caminho, preservando a entrada segura no portal. O cenário segue scroll normalmente.
A bateria de combate do boss permanece independente.

## Extensão v4: vida extra e ímã

BOOST_LIFE é um coletável instantâneo, fora dos slots e da seleção aleatória de
boosts. Aparece após metade da bateria nos mundos 2, 4 e 6; posição usa a consulta
de passagem segura. Se perdido/inseguro, pode reaparecer (tentativas a cada 90
passos de mundo). Uma coleta por mundo, sem duração e sem multiplicação por x2.
Items emite EVENT_BOOST_COLLECTED; apenas o core incrementa vidas e rejeita duplicatas
com life_awarded, reiniciado em BeginWorld. Os efeitos reutilizam a cor do item.
O HUD mostra até seis corações, o máximo natural por partida. O bônus é opcional:
não bloqueia o portal se não for coletado. Ímã dura 7 segundos normais, congela na
pausa e renova para 7 s; mantém as regras existentes de limite de dois boosts.

## Extensão v5: intangibilidade

BOOST_PHASE participa do mesmo sorteio uniforme dos boosts não ativos, com o
mesmo relógio de geração (360–680 passos iniciais, depois 500–940). A vida extra segue
separada. Dura 7 segundos normais; pausa congela e renovar restaura 7 segundos.
Ocupa um dos dois slots e segue substituição do mais antigo. Damage no core
rejeita impactos de totens, perigos, chão e boss antes de consumir escudo ou
aplicar impulso/invulnerabilidade. Não altera física, coleta ou geometria dos perigos.
PlayerDrawArt usa transparência e halo ciano; HUD usa definição e timer de items.

## Energia preservada ao cruzar perigos horizontais

PassageQuery.is_safe não rejeita itens por sobreposição com perigos móveis.
Estes continuam causando dano ao jogador por touches_player, mas não removem
energia nem benefícios. Só o contato do jogador gera coleta. Saída da tela e
reposicionamento por terreno sólido continuam reoferecendo itens sem contar coleta.
Regressão em items-passages cobre os seis perigos atravessando energia e escudo.

## Sorteio por partida

O executável inicializa ItemsSetSeed com relógio e aleatoriedade da sessão. Reset
combina essa semente com run_id e mundo usando mistura de bits; reiniciar não repete
a sequência anterior. Testes fixam a semente para reproduzir problemas. Boosts têm
fluxo pseudoaleatório separado dos fragmentos. Cada escolha é uniforme entre os
cinco boosts não ativos, sem tipo fixo por mundo e sem ordem predefinida; repetições
são possíveis. Primeiro intervalo 360–680 passos de mundo, seguintes 500–940,
mantendo médias anteriores (520/720). Slow e pausa continuam controlando o relógio.
Altura varia dentro das passagens seguras. Coração continua reservado aos mundos
2/4/6 e não participa desse sorteio.

Ajuste de dificuldade: perigos horizontais dos seis mundos com velocidade-base
reduzida em 10% e intervalo entre lançamentos aumentado em 20%. Preservados
aviso, oscilação, limite simultâneo, variação de ±4% e atraso inicial de 180 passos.

Ajuste adicional: intervalos multiplicados por 1,15 sobre o aumento anterior
de 20%, totalizando 38% acima dos intervalos originais. Velocidades preservadas.

Novo ajuste: mais 30% sobre os intervalos vigentes (fator acumulado 1,794
sobre os originais). Velocidades e atraso inicial preservados.

Padronização anterior: intervalo-base de 540 passos (9 segundos sem slow motion)
entre perigos horizontais em TODOS os seis mundos, substituindo os ajustes
percentuais anteriores. Geração pode ser adiada pela segurança da passagem.

## Cenas narrativas: abertura e vitória

O elenco das cenas contém os seis personagens, conforme confirmação do usuário.
UiNarrativeDraw compõe duas ilustrações vetoriais usando PlayerDrawPortrait e
BossDrawPortrait; não altera estado do jogador/boss nem carrega assets externos.
UiButtons é a fonte única de hitboxes/ações para as telas novas. INTRO oferece
Iniciar Aventura; WON oferece Jogar Novamente e Menu Principal. Pausa/derrota
preservam seus layouts. Títulos, narrativa e pontuação são desenhados fora da arte.
O core chama ui.step em INTRO/WON com tempo de apresentação limitado a 0,1 s por
quadro. Isso não avança simulação, física, itens, vidas ou boss. Sem foco não avança;
pausa permanece congelada. Draw é somente leitura. Vitória ainda exige BOSS_DONE.

Ajuste vigente (modo educativo, aplicado aos dois modos): intervalo-base de 510 passos
(8,5 segundos sem slow motion) entre perigos horizontais em todos os seis mundos.
Velocidades, aviso, oscilação, limite simultâneo e atraso inicial preservados.

## Extensão v6: modo educativo (MORPH_EDUCATIONAL)

MORPH_CONTRACT_VERSION=6: GameState ganhou `edu` (EduState/QuizState), GameMode ganhou
GAME_QUIZ e GameCommand ganhou COMMAND_ANSWER_A..D e COMMAND_CONTINUE, todos no fim
das enumerações. Nenhuma assinatura de callback ou factory mudou. Recompilar tudo junto.

A opção CMake MORPH_EDUCATIONAL (padrão OFF) define MORPH_EDUCATIONAL=0/1 como PUBLIC em
morph_core. GameInit usa esse valor; GameInitMode permite aos testes escolher o modo.
`edu.enabled` fica fixo durante a execução. Com OFF, nenhuma pergunta, cronômetro ou
ressurreição existe: os caminhos novos dependem de `edu.enabled`.

- Letras: apenas representação. Items desenha `ItemsFragmentLetter(state,id)` no lugar
  do raio quando `edu.enabled`; geração, IDs, colisão, ímã, pontuação (35) e metas
  4/6/8/10/12/16 não mudam. A energia de combate do Grootspectro continua igual.
- Portal: no mesmo instante em que o arcade iniciaria PORTAL (bateria completa +
  path_clear), o core abre GAME_QUIZ/QUIZ_PASSAGE. Só COMMAND_CONTINUE após um acerto
  chama begin_portal. Erro ou tempo esgotado abrem outra pergunta (nunca a anterior);
  sem perda de vidas, letras ou pontos. `portal_unlocked` zera em BeginWorld.
- Segunda chance: quando Damage zera as vidas e `revive_used=false`, o core abre
  QUIZ_REVIVE (vidas permanecem 0). Acerto: 1 vida, mesmo mundo/score/letras/boss,
  player.reset_world (180,245) + apply_hit e 180 passos de proteção no core
  (`grace_steps`, Damage rejeita impactos). Erro/tempo esgotado: GAME_LOST após o
  feedback. Uma tentativa por aventura; NewRun restaura a disponibilidade.
- Cronômetro: 12 s de tempo de apresentação (segundos reais do quadro, limitados a
  0,1 s), independente de world_steps/slow motion. O tempo do quadro é aplicado antes
  da entrada do mesmo quadro: resposta no quadro em que o prazo acaba é rejeitada.
  O feedback congela o relógio. Proteções de 0,35 s (entrada) e 0,5 s (continuar)
  evitam que um impulso/clique em curso responda sem querer.
- GAME_QUIZ não executa Step: player, obstáculos, itens, boosts, cenário, portal, boss
  e pontuação ficam congelados. P/perda de foco levam a PAUSED com resume_mode=QUIZ;
  a volta não aplica o tempo acumulado.
- UI: UiQuizButtons/UiQuizDraw (somente leitura). Teclas 1-4/A-D, setas+Enter/Espaço
  e clique; o foco começa vazio para que Espaço de voo não responda. Up/W não respondem.

## Controle (DualShock 4 e outros reconhecidos pela raylib 5.5) — somente versão educativa

Sem mudança de contrato (MORPH_CONTRACT_VERSION continua 6). O controle existe apenas
no build educativo (MORPH_EDUCATIONAL=ON): main.c só lê o controle quando PadSupported()
é verdadeiro. No arcade nada é lido, os textos de ajuda são os originais e
`--gamepad-test` apenas informa que o recurso é da versão educativa. `src/input/gamepad.{h,c}`
(puro, em morph_core) converte um retrato do controle em ações e as aplica ao mesmo
GameInput e à mesma navegação da UI (UiQueueNavigation) usados por teclado e mouse.
main.c lê a raylib apenas pelos botões semânticos (GAMEPAD_BUTTON_RIGHT_FACE_DOWN = X,
RIGHT_FACE_RIGHT = Círculo, RIGHT_FACE_LEFT = Quadrado, MIDDLE_RIGHT = OPTIONS,
LEFT_FACE_* = direcional, eixo LEFT_X/Y), já traduzidos pela base de mapeamentos do GLFW;
nenhum número bruto é presumido. O primeiro controle disponível é lido a cada quadro;
conectar, desconectar ou trocar de controle limpa o estado, e botões já apertados nesse
momento só contam depois de soltos.

| Tela | X | Quadrado | OPTIONS | Círculo | Direcional / analógico |
|---|---|---|---|---|---|
| PLAY / BOSS | impulso | megarraio (só BOSS) | pausa | — | — |
| PORTAL / TRANSITION | — | — | pausa | — | — |
| INTRO / WON / LOST / PAUSED | confirma o foco | — | retoma (PAUSED) | PAUSED: continuar; LOST/WON: voltar ao início | navega |
| QUIZ | confirma a alternativa em foco | — | pausa | — | escolhe a alternativa |

Em menus e perguntas o X nunca vira impulso; o impulso é por borda, então X mantido ao
voltar ao jogo não voa. Na pergunta o foco começa vazio: é preciso escolher pelo
direcional antes de confirmar (mesma proteção do Espaço). Navegação: um passo imediato,
repetição após 0,40 s e depois a cada 0,18 s; analógico com histerese 0,55/0,35. O
cronômetro de 12 s não depende do controle. `morph-adventure --gamepad-test` mostra o
nome do controle e cada ação pressionada para conferir o mapeamento real.

## Áudio (sem mudança de contrato)

`src/audio/audio_cues.c` (puro, em morph_core) decide efeitos e plano musical a partir de
eventos reais e de transições do GameState; `src/audio/audio.c` (raylib) toca. A
composição (`composition.c`) envolve `player.step`, `effects.event` e `obstacles.step`
apenas para ouvir eventos: nenhum callback altera estado do jogo por causa do áudio, e
GameState, GameModules e MORPH_CONTRACT_VERSION não mudaram. Falta de dispositivo ou de
arquivos deixa o jogo em silêncio, nunca em GAME_ERROR. Ver docs/AUDIO.md.
