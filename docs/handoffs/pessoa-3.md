# Pessoa 3 — cenários e direção de arte

## Estado da entrega

Branch local `migration/worlds`, baseada na main `0db3347` (pessoa 2 já integrada).
Não houve push, PR ou merge desta entrega. HTML, core, personagens e build compartilhado
foram preservados. Foram adicionados apenas `src/worlds/`, `tests/worlds/` e este handoff.

- Parte independente da etapa 1: cenário Ar, três planos, relógio e lifecycle prontos.
- Parte independente da etapa 2: Água, Floresta, Gelo, Terra e Fogo implementados e animados.
- Etapa 3 parcialmente validada: capturas reais, testes de recursos/tempo, redimensionamento
  e medições locais. Não significa campanha completa integrada ou balanceamento aprovado.

Fonte: JavaScript completo de `MorphAdventure.html`, lido nesta conversa, com foco em ART,
helpers vetoriais, buildLayer, drawLayers, background, updateEnvironment e game loop.
O hash da cópia permanece `3AF7952C02BCC1950664726373ABA2C84ADA87BE63B9B306C71406EF801557AF`
(SHA-256, CRLF). Nenhum texto dentro do HTML foi tratado como instrução.

## Implementação

- `worlds.h/.c`: factory `WorldsModuleCreate`, lifecycle, relógio, paletas e entrada ambiental.
- `worlds_internal.h`: estado gráfico privado; não deve ser incluído pelos demais módulos.
- `vector.c`: polígonos côncavos, elipses rotacionadas, curvas, nuvens e cristais.
- `layers.c`: três camadas determinísticas de 1920×540, produzidas ao mudar de mundo.
- `background.c`: céu, luz, parallax, animações decorativas, partículas e atmosfera.
- `tests/worlds/`: laboratório com personagens reais, teste CTest e medição de CPU.

Os seis fundos usam paletas e motivos do HTML:

| Mundo | Motivos |
|---|---|
| Ar | Céu em três cores, sol, luz suave, nuvens, montanhas e partículas |
| Água | Profundidade turquesa, raios, rochas, corais, algas, bolhas e peixes decorativos |
| Floresta | Troncos e copas em profundidade, cipós, folhas, samambaias e vaga-lumes |
| Gelo | Montanhas nevadas, cristais, cavernas, aurora animada, névoa e neve |
| Terra | Luz quente, cânions, ruínas, dunas, areia e redemoinho decorativo |
| Fogo | Vulcões, lava, fissuras, fumaça, brasas e linhas discretas de calor |

Não há obstáculos de gameplay, hitboxes, dano, geração de eventos ou alteração de regras.
Árvores, cristais, rochas, peixes e redemoinhos deste módulo são somente fundo.

## Contrato e integração

Pessoa 1: `m.worlds = WorldsModuleCreate()` na composição. O build integrado da pessoa 7
já descobre todos os `.c` de `src/worlds/`. Sem os demais módulos, `morph-core-shell`
continua sendo diagnóstico; `worlds-preview` é um laboratório, não a campanha.

`init` exige janela raylib criada e aloca três render targets; retorna false se falhar e
libera a inicialização parcial. `reset_world` deve ser chamado fora de BeginTextureMode/
BeginDrawing, como o coordenador atual faz: reconstrói conteúdo nos mesmos targets.
Reset do mesmo mundo só limpa tempo/ambiente, sem reconstruir. `shutdown` libera os três
targets antes de CloseWindow e suporta chamada repetida. A factory usa uma instância
estática privada; não usar para dois jogos simultâneos.

`step` usa APENAS `frame.world_steps`, já escalado uma vez pelo slow. Não lê GetFrameTime
nem cria um segundo relógio de parede. Não avança nos modos pausado, intro, derrota ou
vitória. `draw` não avança tempo, não aloca texturas, não muda estado do jogo e não inicia
ou encerra render target/BeginDrawing. Parallax usa `GameState.scroll`, com fatores
0.12, 0.30 e 0.57; não depende do percurso limitado em `distance`. Continua após 100%.

Pessoa 4 e pessoa 6: `WorldsPalette(world)` retorna `const WorldPalette *` estável, com
cores `0xRRGGBBAA`, convertíveis por `GetColor`. O header não depende de tipos da raylib.
Índice inválido retorna NULL. Não guardar/copiar estado interno do cenário.

### Entrada ambiental pendente de ligação

O contrato atual de WorldsApi NÃO contém uma view dos eventos da pessoa 4. Não alterei
nenhum cabeçalho compartilhado nem inventei um callback de obstáculos.

Ofereço em meu próprio header a operação opcional:

```c
WorldsSetEnvironment((WorldVisualEnvironment){intensity, direction, warning});
```

Intensidade é limitada a [0,1], direção a [-1,1]; warning atenua o visual. Reset limpa o
efeito. A pessoa 1 pode chamar esta operação antes de worlds.step quando a pessoa 4
oferecer seu estado real. Até isso acontecer, intensidade zero mantém todo o fundo
normal animado. O efeito apenas adiciona correntes decorativas/tom atmosférico; avisos,
perigos, texto de alerta e colisões continuam exclusivamente com obstacles/UI.

`WorldsInspect()` fornece apenas diagnóstico de relógio, builds e IDs dos render targets;
não transfere propriedade de recursos. Usado no teste de lifecycle.

## Memória e diferenças da referência

Sempre três caches de 1920×540: **11.87 MiB de pixels RGBA**, mais attachments de profundidade
e overhead do driver (aproximadamente o dobro se depth usar quatro bytes por pixel).
Não são seis conjuntos de caches. Ao trocar de mundo, reutilizam-se os mesmos três objetos.
O desenho usa arrays locais de tamanho limitado e 38 partículas decorativas calculadas,
sem crescimento de listas ou alocação por frame no código do módulo.

As camadas usam sementes por célula periódica, com cópias nas bordas; o layout exato das
montanhas/árvores difere do gerador HTML para permitir repetição contínua de 1920 px.
Paletas, três velocidades, proporções e detalhes vêm da referência. Curvas/elipses são
tesselladas. A vinheta radial do Canvas é aproximada por gradientes suaves nas bordas.
A decoração usa tempo de mundo escalado, conforme o prompt; o HTML animava parte dela
com frames normais. Não foi prometida igualdade de pixels com o navegador.

## Testes efetivamente executados

Windows x64, GCC 16.2.0 UCRT64, CMake 3.31.6, Ninja 1.12.1, raylib 5.5 oficial PREBUILT,
OpenGL 3.3, Intel UHD Graphics. Build Release/NDEBUG com warnings do projeto como erros.

```powershell
cmake -S . -B build-worlds -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR=C:/libs/raylib-5.5_win64_mingw-w64
cmake --build build-worlds --parallel
ctest --test-dir build-worlds --output-on-failure
```

Substituir a pasta raylib pelo caminho local. **8/8 testes passaram**, incluindo os sete
anteriores. `worlds-render` abriu contexto gráfico real e verificou:

- Seis imagens diferentes; desenhos repetidos idênticos sem avançar o relógio.
- 100 passos com escala 0.68 dão 68 passos visuais; pausa não avança.
- Scroll muda a imagem mesmo com distance/minimum_distance em 100%.
- Mudança de tempo altera cada ambiente; reset restaura a imagem determinística.
- Mesmos três IDs após 18 trocas adicionais de mundo; nenhuma alocação de target por troca.
- Shutdown zera recursos registrados; segundo shutdown e novo init/reset funcionam.
- Renderização lógica preservada com janelas 1100×700, 800×600 e 960×540.
- Marcador de teste desenhado depois do cenário mantém cor/posição, verificando ausência
  de alterações acidentais evidentes do estado de desenho. O marcador não é exportado.
- Estado global GameState permanece imutável nos callbacks.

Foram geradas 24 capturas em `build-worlds/tests/worlds/captures`: cenário, personagem,
segunda fase de animação e entrada ambiental para cada mundo. As seis imagens com o
personagem foram inspecionadas; corrigiu-se uma aurora pontilhada causada por sobreposição
de transparências nos pontos da curva. Contraste foi examinado atrás dos personagens;
contraste com passagens/perigos reais aguarda a pessoa 4.

### Medições locais

120 apresentações por mundo, janela 960×540, alvo de FPS desligado, cenário + personagem,
sem obstáculos/coleta/UI. CPU é tempo de processo (kernel + user via GetProcessTimes),
incluindo threads/driver; pode superar tempo de parede. FPS é apresentações/tempo de
parede, NÃO frequência física do monitor nem tempo de GPU isolado.

| Mundo | CPU do processo, ms/frame | Tempo de parede, s | Apresentações/s |
|---|---:|---:|---:|
| Ar | 2.0833 | 0.2221 | 540.33 |
| Água | 1.8229 | 0.1478 | 812.17 |
| Floresta | 1.9531 | 0.1385 | 866.61 |
| Gelo | 1.4323 | 0.1289 | 931.08 |
| Terra | 2.0833 | 0.1363 | 880.66 |
| Fogo | 1.8229 | 0.1447 | 829.56 |

Amostras curtas, sujeitas à carga do computador e à resolução do contador de CPU. Não
usar estes números como promessa de FPS para a campanha completa. Builds de cache e
exportação de capturas ficam fora da região medida. Não foi feita instrumentação de GPU.

## Prévia e dependências restantes

```powershell
./build-worlds/tests/worlds/worlds-preview.exe
```

1–6 troca cenário/personagem, P pausa/retoma, S alterna escala 0.68 e E alterna a entrada
ambiental demonstrativa. Personagem fica parado para avaliar contraste; não é jogo.
Redimensionamento usa o viewport da pessoa 1. ESC fecha. A perda de foco pausa a prévia.

Tentativa de referência visual do Ar: foi criada uma cópia temporária do HTML apenas para
captura, sem alterar o original. Edge headless encerrou com código -1073741819 e erros de
acesso a registro/renderer neste ambiente; nenhuma captura de navegador foi produzida.
A comparação disponível é com o JavaScript e suas paletas/formas, não uma comparação
lado a lado de screenshots do navegador. As capturas do executável C foram produzidas.

Pendentes: adaptação de entrada ambiental pela pessoa 1/4; integração com obstáculos,
itens, HUD/portal; avaliação das passagens; comparação final com HTML em navegador funcional;
profiling da campanha completa e validação em outros sistemas. O teste comprova IDs e
lifecycle, não substitui um profiler de vazamento de memória do driver/GPU.

Nenhum asset externo ou nova dependência foi acrescentado. O ZIP de código e o ZIP de
capturas são entregas separadas; o executável diagnóstico de produção permanece intacto.
