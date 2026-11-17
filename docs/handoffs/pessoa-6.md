# Pessoa 6 — interface, efeitos e portais

## Escopo e estado

Branch local `migration/ui-portals`, criada de `migration/items` no commit `6177c74`.
Integre as entregas 4 e 5 antes desta caso ainda não estejam na main. Não houve fetch,
push ou merge nesta entrega. Somente `src/ui/`, `src/effects/`, `tests/ui/` e este
handoff foram alterados; main, core e os módulos dos colegas foram preservados.

Partes independentes das etapas 1 e 2 implementadas. Etapa 3 validada parcialmente:
fluxo integrado em laboratório com todos os módulos reais, testes de comandos/tempos,
capturas e descarte de recursos. A composição em src/core/composition.c, os atalhos
adicionais em main e o pacote final continuam sob responsabilidade das pessoas 1 e 7.

Referência: JavaScript completo de `MorphAdventure.html`, lido nesta conversa. Revisados
openPortal, tickPortal, tickTransition, drawPortal, drawPortalFrame, updateEffects,
drawParticles, drawWorldTitle, updateHud, startScreen, pause, damage e victory.
Fonte preservada, SHA-256 CRLF:
`3AF7952C02BCC1950664726373ABA2C84ADA87BE63B9B306C71406EF801557AF`.
Nenhum texto no HTML foi tratado como instrução.

## Implementação

- `src/ui/ui.h/.c`: factory `UiModuleCreate`, comandos, foco, retângulos dos botões,
  animação da bateria, placar e título. `ui_internal.h` define estado privado;
  `ui_draw.c` desenha HUD, telas e botões.
- `src/effects/effects.h/.c`: factory `EffectsModuleCreate`, pool de 180 partículas,
  eventos, pose do jogador e sequência de sinais do portal. Estado privado em
  `effects_internal.h`; desenho vetorial em `effects_draw.c`.
- `tests/ui/ui_rules.c`: regras de entrada, bateria, portais e partículas.
- `tests/ui/ui_lab.c`: laboratório com TODOS os módulos reais e coordenador real,
  adaptadores ambientais e proteção de itens, modo interativo e modo de capturas.
- `tests/ui/CMakeLists.txt`: extensão de testes existente da pessoa 7; nenhum build
  compartilhado foi modificado.

UI lê GameState/ItemsView e retorna GameCommand. Não altera vidas, score, mundo ou
energia. Usa GameEnergyRequired/GameEnergyFraction e ItemsBoostDefinition, sem criar
segunda contagem. `UiInspect` e `EffectsInspect` devolvem cópias para diagnóstico.
Desenho não avança tempos. Esses módulos não alocam texturas/fontes: usam formas e a
fonte padrão da raylib, pertencente à janela. Shutdown limpa estado e pode ser repetido.

## Fluxo visual

HUD contém três corações, score, percurso, energia com quantidade/meta e percentual,
bateria animada com terminal e brilho e até dois boosts com segundos ou indicação de
impacto único. Bateria incompleta não mostra 100%; animação usa ui_seconds, independente
de slow motion. O nome do mundo recebe fundo translúcido para contraste no Ar/Gelo.
Telas: início, pausa, derrota e vitória. Reinício/retentativa sempre passam pelo core.
O título do próximo mundo fica preservado enquanto o fade cobre a tela.

Portal cresce por 65 passos normais. A aproximação central suaviza Y; depois a atração
usa smoothstep ao longo de 85 passos, reduzindo escala de 1 a 0,08. Com a ordem do HTML,
entrada conclui no passo 149. O sinal ENTERED é emitido uma vez; só o core paga +100.
Fade atinge opacidade máxima no passo 24 e termina no 55. BLACKOUT e FADE_DONE também
são únicos. Switch limpa partículas e pose sem reiniciar o fade, permitindo o reset
do personagem pelo coordenador. Ao terminar, o módulo fica pronto para o próximo portal.
O módulo não decide vitória: apenas o core conclui o portal do Fogo e muda para WON.
O blackout não é desenhado sobre a tela de vitória.

Seis molduras: nuvens, bolhas de redemoinho, raízes/folhas, cristais, blocos de ruínas e
energia vulcânica, usando as paletas públicas da pessoa 3. Espiral e partículas orbitais
acompanham o crescimento. Partículas de coleta/dano/boost recebem eventos, sem consultar
colisões. Pool limitado, com descarte de novos excessos e reutilização após expiração.

Pausa congela relógios, pose, bateria e partículas. Durante pausa no fade, apenas a
opacidade desenhada é reduzida para tornar os controles legíveis; idade do fade não
muda e a opacidade correta é restaurada ao retomar. Esse ajuste e flashes de dano/carga
mais leves são diferenças deliberadas do HTML para preservar a legibilidade.

## Entrada e integração da pessoa 1

Botões usam os mesmos retângulos no desenho e na consulta. Clique sobre botão devolve
consumes_pointer=true, incluindo pausa durante voo/portal/transição. Barras externas
são rejeitadas por pointer_inside, transformado pelo core/viewport.
Espaço/impulse_pressed ativa a opção em foco nos menus; P segue o contrato atual.
O módulo não chama IsKeyPressed, GetMousePosition ou lê entrada da raylib.

O GameInput aprovado não tem Tab/Enter/setas de navegação. A API adicional
`UiQueueNavigation(direction, activate)` deve ser chamada pelo dono da entrada, antes
de GameAdvance: direção +1/-1 para Tab/Shift-Tab; activate=true para Enter. Pode receber
Tab e Enter no mesmo frame. A fila é consumida uma vez e descartada sem foco. O adaptador
está demonstrado no laboratório; src/main.c ainda precisa desse encaminhamento.

Registrar UiModuleCreate/EffectsModuleCreate com as demais factories em composition.c.
O laboratório demonstra também as ligações que já estavam pendentes:

1. Antes de obstacles.step, copiar fragmentos vivos de ItemsInspect e encaminhar seus
   círculos previstos a ObstaclesSetProtectedItems, verificando sucesso.
2. Após obstacles.step, encaminhar ObstaclesEnvironment a WorldsSetEnvironment.
3. Manter GameAdvance e GameDraw como coordenadores. Não duplicar transições, pontuação
   ou resets dentro de main/UI. O laboratório não contém substitutos vazios de módulos.

Composição e adaptadores do laboratório podem orientar a implementação da pessoa 1,
mas pertencem ao alvo de teste. `morph-core-shell` continua sendo a janela de diagnóstico;
`MORPH_MODULES_LINKED=ON` ainda requer composition.c. Pessoa 7 também deve atualizar a
documentação compartilhada de build, que ainda descreve os módulos como pendentes.

## Build e verificação

Ambiente: Windows, GCC 16.2, C99 Release, raylib 5.5 oficial PREBUILT, warnings como erros.

```powershell
cmake -S . -B ../ui-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR="CAMINHO_DA_RAYLIB_5.5"
cmake --build ../ui-build
ctest --test-dir ../ui-build --output-on-failure
../ui-build/tests/ui/ui-lab.exe
```

Laboratório interativo: Espaço/↑/W/clique impulsiona; P pausa; Tab/Shift-Tab navega;
Enter ou Espaço ativa menu; Escape fecha. Entrada de mouse usa a transformação inversa
e barras proporcionais. É uma integração de laboratório, não o pacote final publicado.

Testes novos verificam comandos/foco, clique de pausa sem impulso, bateria 23/24 abaixo
de 100%, brilho final, pausa, seis sequências de sinais sem repetição, pool limitado,
limpeza e pureza de desenho. O teste ui-flow usa todos os módulos reais. A derrota ocorre
pela física/colisão real; energia e distância são preenchidas por fixtures explícitas
para testar portais rapidamente. Isso não é uma partida humana completa ou validação
de balanceamento. As três vidas permanecem ao trocar os seis mundos e a vitória ocorre
apenas no sexto; score de 600 corresponde às seis entradas no cenário controlado.

Capturas: início, HUD parcial com boosts, HUD completo, pausa, derrota, pausa no portal,
transição, pausa na transição, seis molduras e vitória. Capturas de janela validam escala
em 480×270, 800×600 e 1280×720. Em 480×270 os textos secundários ficam pequenos; 800×450
ou maior oferece leitura mais confortável. Fonte padrão com textos ASCII evita carregar
assets de fonte; não há suporte a leitor de tela ou promessa de acessibilidade completa.

Não houve teste em Linux/macOS, avaliação humana de dificuldade ou empacotamento final.
Na execução sem DISPLAY em Linux os testes gráficos exigem xvfb; sem ele ficam desativados.

Resultado final: 16/16 testes CTest passaram em 24,98 s. As capturas finais das tres resolucoes foram inspecionadas. O teste de janela verifica dimensoes e conteudo, capturando antes da troca dos buffers para evitar imagens pretas em janela oculta.
