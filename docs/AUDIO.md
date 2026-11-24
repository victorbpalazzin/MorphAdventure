# Áudio do Morph Adventure

Trilha sonora e efeitos originais, criados para o jogo, e o sistema que os toca com
raylib 5.5. Vale para as versões Arcade e Educativa (a educativa acrescenta os sons
dos desafios; ver a seção final).

## Identidade sonora

Todo o jogo gira em torno de um **motivo de Morph**: uma subida de quatro notas
(graus 5-1-2-3 da escala, "olhar para cima") respondida por 5-3-2-1. Ele aparece
no menu e reaparece transformado em cada mundo (modo, timbre, registro e ritmo
mudam), na vitória e em contraponto ao **motivo de Grootspectro** (1-b2-1-#4,
cromático e instável) durante a batalha final. A sensação é de uma jornada
contínua: o mesmo herói atravessando paisagens diferentes.

| Faixa | Tonalidade / andamento | Direção artística |
|---|---|---|
| Menu (`menu.ogg`, 2:00) | Ré maior, 96 bpm | Apresenta o motivo: piano elétrico FM, pads, arpejos dedilhados com delay, sinos, groove leve e cordas na elevação. Expectativa sem exagero. |
| Ar (`ar.ogg`, 1:47) | Sol lídio, 108 bpm | Liberdade e altitude: flauta com sopro canta o motivo com a 4ª aumentada (lídio), harpa em arpejos, pads aéreos largos, percussão orgânica (estalos, shaker, palmas) e vento estilizado (ruído filtrado em movimento, não ruído branco). |
| Água (`agua.ogg`, 1:54) | Mi dórico, 84 bpm | Profundidade e fluidez: piano elétrico líquido (chorus + delay pingue-pongue), sub-grave, "gotas" cristalinas respondendo à melodia, voz sintética etérea cantando o motivo em modo menor, percussão abafada "submersa" e bolhas esparsas integradas ao ambiente. |
| Floresta (`floresta.ogg`, 1:55) | Lá eólio, 92 bpm | Encantamento e curiosidade: kalimba em ostinato, pizzicatos escuros, madeira soprada (timbre de ocarina/clarinete), celesta na seção lúdica, tambores de mão e floresta estilizada (pássaros sintéticos, folhas) bem ao fundo. |
| Gelo (`gelo.ogg`, 1:50) | Si menor, 70 bpm | Contemplação gelada: sinos de vidro, piano esparso, pads longos com reverb cristalino (shimmer de oitava), percussão de cristal. Na metade, a nevasca chega: zumbido grave, tambores distantes e cordas em trêmulo; depois o céu limpa. |
| Terra (`terra.ogg`, 1:43) | Ré dórico, 112 bpm | Paisagens vastas: tambores grandes (taiko, tons), ostinato de "violão" em semicolcheias, trompas híbridas com o motivo heroico, baixo dedilhado e areia granular. Modo dórico em vez de escalas "do deserto", para fugir de clichês. |
| Fogo (`fogo.ogg`, 1:42) | Dó menor, 132 bpm | Urgência: bateria propulsiva com sidechain, reese saturado (controlado), stabs distorcidos, cordas em spiccato, metais híbridos, coro no breakdown, crepitar de fogo e risers. O motivo vira chamado heroico para o clímax. |
| Grootspectro (`boss_1/2/3.ogg`, 1:30 cada) | Mi menor, 128 bpm | Batalha adaptativa em três camadas sincronizadas: (1) coro sombrio, cordas graves, motivo do Titã nos metais e "relógio" percussivo; (2) + tambores, baixo distorcido e spiccato; (3) + cordas rápidas, motivo do herói contra o do vilão, pratos e coro agudo. |
| Vitória (`vitoria.ogg`, 0:44) | Ré maior, 100 bpm | Resolução da jornada: impacto, cordas e coro crescendo, o motivo completo nos metais, final em acorde amplo com sinos. Toca uma vez; depois o tema do menu volta suavemente. |
| Derrota (`derrota.ogg`, 0:25) | Ré menor → Ré maior, 72 bpm | Fragmento do motivo no piano, descendente e discreto, terminando num acorde maior aberto ("tente de novo"). |

Medições (EBU R128, `assets/audio/manifest.json`): faixas em -16 LUFS integrados,
pico real entre -1 e -6 dBTP. Camadas do chefe medidas como o jogo as soma: fase 1
-19,0 LUFS (tensão contida logo após o Fogo), fase 2 -15,9 LUFS, fase 3 -15,4 LUFS.
Loops: a cauda de reverb/delay é dobrada sobre o início, então o fim encaixa no
começo sem salto (verificado na emenda de cada faixa).

## Efeitos sonoros

Afinados na paleta do jogo (Ré maior/pentatônica), em camadas (ataque, corpo e
cauda) e com variações. Efeitos frequentes são mais baixos.

| Grupo | Efeitos |
|---|---|
| Personagem | batida de asa (3 variações + afinação aleatória ±4%), dano (impacto + mordida + brilho de proteção), derrota, coração (+1 vida) |
| Coleta e progresso | coleta (pingo cristalino; cada coleta sobe uma nota na escala pentatônica, formando uma melodia até a bateria encher), bateria completa |
| Boosts | escudo (domo de energia), escudo rompido (estilhaço), ímã, slow motion ("fita parando"), x2, intangível (sopro fantasmagórico) |
| Portais | portal se abrindo (vórtice que acompanha a animação), travessia, troca de mundo sob o escurecimento |
| Perigos | aviso discreto quando um perigo surge; lançamento próprio de cada tipo (redemoinho, água-viva, cipó, estilhaço de gelo, rocha, bola de fogo); aviso de evento ambiental; evento ambiental de cada mundo (rajada, corrente, folhas, nevasca, areia, erupção) |
| Grootspectro | aparição, aviso de ataque, projétil espectral, energia de combate (acorde arco-íris que sobe a cada coleta), bateria pronta, **megarraio** (carga 0–0,58 s, impacto exatamente no golpe da animação, cauda arco-íris até ~1,4 s), chefe ferido, troca de fase, derrota do chefe (acompanha os 3,5 s da animação e resolve em maior) |
| Interface | navegação, confirmação, voltar, pausa, retomar, início da aventura (motivo em sinos), ajuste de volume |

Formatos: efeitos curtos em WAV 16 bits/44,1 kHz (carregamento imediato); efeitos com
mais de 1,5 s em Ogg Vorbis q6. Músicas em Ogg Vorbis q5 tocadas por streaming.

## Como o áudio reage ao jogo

- **Menu → mundo:** o tema do menu cede lugar à faixa do Ar.
- **Portal:** abre com o vórtice; ao atravessar, a faixa do mundo sai em 1,2 s; no
  escurecimento entra a do novo mundo em 2,5 s. Nunca há corte seco.
- **Fogo → Grootspectro:** transição + aparição do chefe; a música da batalha começa
  só com a camada 1 e ganha camadas a cada segmento destruído (fade de 2 s, sempre
  sincronizadas porque começam juntas e têm o mesmo tamanho).
- **Derrota do chefe:** a trilha da batalha se encerra junto com o efeito de derrota;
  a vitória toca em seguida.
- **Pausa e perda de foco:** música e efeitos param no ponto exato e continuam de onde
  estavam ao retomar.
- **Ambiente:** sons de perigos e eventos ambientais tocam nos momentos reais do jogo
  (surgimento, lançamento, aviso, início do evento), nunca em repetição contínua.

## Volume e silêncio

- **M**: liga/desliga o som a qualquer momento (o HUD mostra "MUDO").
- **Na pausa**: painel SOM com Geral, Música e Efeitos. Teclado: **1/2/3** escolhem o
  canal e **←/→** ajustam em passos de 10%; mouse: botões **-** e **+** e o botão de
  mudo. Vale imediatamente, sem reiniciar a partida.
- Padrão: geral 80%, música 70%, efeitos 90%.
- As preferências ficam em `morph-audio.cfg` (texto `chave=valor`) ao lado do
  executável. Se a pasta não permitir escrita, valem só durante a sessão.
- Um limitador suave na mixagem final impede distorção mesmo com tudo em 100%.

## Arquitetura

```
composition.c ──(eventos reais: coleta, boost, dano, portal, asa, perigos, ambiente)──┐
GameState (modo, mundo, fases e ataques do chefe) ────────────────────────────────────┤
UiInspect().focus (navegação) ────────────────────────────────────────────────────────┤
                                                                                       ▼
                         src/audio/audio_cues.c  (puro, sem raylib, em morph_core)
                         decide: efeitos deste quadro + plano musical
                                                                                       ▼
                         src/audio/audio.c  (raylib 5.5)
                         dispositivo, carregamento, vozes, faixas, fades, camadas,
                         atenuação, pausa, volumes, preferências, limitador
```

- `composition.c` envolve `player.step` (batida de asa realmente aplicada),
  `effects.event` (eventos do núcleo) e o `obstacles.step` que já existia (perigos e
  ambiente). **Nenhuma regra do jogo foi alterada.** O contrato continua na versão 5 (arcade).
- O diretor recebe os eventos durante `GameAdvance` e é consultado uma vez por quadro em
  `main.c` (`AudioFrame`). Um evento gera um efeito; nada toca só porque uma condição
  continua verdadeira. Intervalos mínimos evitam empilhar efeitos idênticos (coletas não
  têm intervalo: cada coleta tem seu som).
- `InitAudioDevice` é chamado uma única vez; tudo é carregado na abertura (músicas em
  streaming); aliases (`LoadSoundAlias`) dão polifonia sem duplicar memória; tudo é
  liberado em `AudioShutdown`.
- Falhas: sem placa de som ou sem `assets/audio`, o jogo continua em silêncio e registra
  o motivo no log. Arquivos ausentes são ignorados individualmente.
- Os arquivos são procurados em `<pasta do executável>/assets/audio` (e em `./assets/audio`).

## Compilar e empacotar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMORPH_MODULES_LINKED=ON
cmake --build build
./build/morph-adventure
```

O build copia `assets/audio` para junto do executável (alvo `morph-audio-assets`,
executado sempre). O pacote (`scripts/package/package.sh` ou `package.ps1`) instala
`morph-adventure`, `assets/audio/music`, `assets/audio/sfx`, `CREDITOS.txt` e
`LEIA-ME.txt`, e verifica que o jogo extraído numa pasta limpa encontra o áudio.

## Alterar, substituir e criar sons

Todo o áudio é gerado por código em `tools/audio/` (Python 3 + numpy + scipy; ffmpeg
com libvorbis para codificar e medir):

```bash
pip install numpy scipy
python3 tools/audio/render.py all              # tudo (≈ 7 min)
python3 tools/audio/render.py music fogo       # uma faixa
python3 tools/audio/render.py sfx beam collect # alguns efeitos
python3 tools/audio/check_instruments.py       # verificação dos instrumentos
```

- `morphsynth.py`: instrumentos e efeitos de estúdio (síntese, filtros, reverb por
  convolução, delay, chorus, compressão, limitador).
- `music_core.py`: sequenciador, teoria (modos, acordes, voicings) e mixagem por barramentos.
- `tracks.py`: as composições (melodias escritas como texto, ex.: `"A4/.5 D5/.5 E5/.5 F#5/2.5"`,
  validadas compasso a compasso).
- `sfx.py`: os efeitos. `export.py`: normalização EBU R128, Ogg/WAV e medições.

**Substituir uma trilha:** coloque um Ogg Vorbis com o mesmo nome em
`assets/audio/music/` (ex.: `fogo.ogg`). Normalize em torno de -16 LUFS. Para o chefe,
as três camadas precisam ter exatamente a mesma duração e o mesmo andamento.

**Novo efeito:** acrescente a função em `tools/audio/sfx.py` (ou um arquivo pronto em
`assets/audio/sfx/`), um valor em `AudioCue` (`audio_cues.h`), a linha em `sfx_defs`
(`audio.c`, com nome do arquivo, vozes e ganho) e o gatilho em `audio_cues.c` (a partir
de um evento ou de uma transição de estado).

## Testes

| Teste | O que prova |
|---|---|
| `audio-cues` | Lógica pura: um efeito por evento, intervalos mínimos, melodia da coleta, plano musical por tela, camadas do chefe por fase, megarraio, derrota, perigos e ambiente. |
| `audio-backend-run` | raylib real: preferências (leitura, gravação, texto inválido), modo silencioso sem pasta, carregamento dos 47 efeitos e 12 músicas, todos os efeitos tocando, polifonia limitada, fades sem salto, pausa sem reinício, portal, camadas, vitória/derrota, mudo e arquivos ausentes. Sem placa de som, verifica o modo silencioso e avisa SKIP. |
| `audio-flow` | Jogo completo com módulos reais: asa, coleta, cinco boosts, escudo, dano, pausa, perigos e eventos ambientais reais, seis portais com as faixas na ordem, batalha com energia, três megarraios, fases, derrota do chefe, vitória e derrota. |

Verificação manual feita neste ambiente (Linux, placa de som virtual PulseAudio):
gravação da saída real do jogo em tempo real durante o fluxo completo — 145 efeitos
decididos e 145 tocados, nenhum sample clipado (pico -0,9 dBFS no padrão; -1,0 dBFS com
todos os volumes em 100%), -22 LUFS na mixagem padrão.

**Limitação importante:** o áudio foi produzido e verificado por medições (loudness,
picos, espectrogramas, afinação, emendas de loop, cortes abruptos), mas **não foi
ouvido** por quem o produziu. A avaliação artística precisa de escuta humana.

## Versão educativa

Tudo o que está acima, mais 12 efeitos dos desafios (em `tools/audio/sfx_edu.py`, presente
só nesta branch). Princípio: informar sem assustar ou humilhar.

| Momento | Som |
|---|---|
| Letra coletada | Mesma família da energia (mesma melodia ascendente), com um leve "virar de página" e uma vogal sintetizada |
| Pergunta aparece | Duas notas que pedem atenção e um swell suave, sem urgência |
| Troca de alternativa | Toque curtíssimo de mallet |
| Acerto | Arpejo maior ascendente que floresce num acorde, com brilho |
| Erro | Dois tons aveludados descendo — sem buzina, sem dissonância agressiva |
| Últimos segundos | Só em 3, 2 e 1: tique de relógio de madeira, discreto, subindo levemente |
| Tempo esgotado | Sino abafado duplo e queda suave |
| Nova pergunta | Página virando e uma nota leve |
| Desafio de portal concluído | Escala mágica até a oitava, cordas e brilho; em seguida o som do portal se abrindo |
| Segunda chance | Batimento cardíaco e swell esperançoso |
| Ressurreição | Acorde maior que se abre, batimento forte e faíscas |
| Segunda chance encerrada | Descida suave e acolhedora (no lugar do som de derrota) |

Durante qualquer desafio a música do momento (mundo ou batalha, com suas camadas) continua
tocando a **30%** e volta gradualmente ao fim da pergunta; o cronômetro não depende do áudio.

Com o DualShock 4 (só nesta versão): **Triângulo** liga/desliga o som; na pausa, **L1/R1**
escolhem o canal e **←/→** ajustam o volume (↑/↓ continuam trocando de botão). O teste
`--gamepad-test` também mostra Triângulo, L1 e R1.

Nesta branch o pacote leva 59 efeitos (47 compartilhados + 12 educativos) e as mesmas 12 músicas.
