# 🪽 Morph Adventure

### Uma jornada através dos elementos e da lógica.

**Morph Adventure** é um jogo de aventura com rolagem lateral, desenvolvido integralmente em **C99 com raylib 5.5**, no qual o jogador precisa explorar seis mundos, enfrentar obstáculos, coletar recursos, atravessar portais e derrotar uma ameaça que coloca em risco o equilíbrio dos elementos.

O projeto foi desenvolvido por estudantes de **Sistemas de Informação da CESAR School**, como parte de atividades acadêmicas envolvendo programação, lógica, desenvolvimento de jogos, engenharia de software e trabalho colaborativo.

Originalmente concebido como um protótipo inspirado em *Flappy Bird*, desenvolvido em HTML, o jogo passou por uma reconstrução completa em C, evoluindo para uma aventura com personagens próprios, cenários dinâmicos, mecânicas progressivas e uma versão educativa voltada à aprendizagem de Lógica Proposicional.

Atualmente, o projeto disponibiliza duas versões independentes:

- **Arcade:** experiência tradicional de aventura, exploração e coleta de energia.
- **Educativa:** incorpora letras coletáveis, perguntas de lógica com tempo limitado, segunda chance e suporte ao controle DualShock 4.

As duas versões compartilham a mesma base de código e são configuradas durante a compilação.

---

## 📥 Download e versões disponíveis

As versões para Windows podem ser baixadas diretamente pela página de Releases do projeto:

**[Acessar todas as Releases](https://github.com/victorbpalazzin/MorphAdventure/releases)**

### 🎮 Versão Arcade — v1.0.0

**[Baixar Morph Adventure Arcade](https://github.com/victorbpalazzin/MorphAdventure/releases/download/v1.0.0/MorphAdventure-Arcade-v1.0.0-Windows-x64.zip)**

Esta versão apresenta a experiência original, com:

- Seis mundos exploráveis.
- Coleta de fragmentos de energia.
- Obstáculos e perigos ambientais.
- Sistema de vidas e pontuação.
- Boosts temporários.
- Portais de transição.
- Batalha final contra Grootspectro.

**Tag:** `v1.0.0`

**Branch de referência:** `main`

### 🧠 Versão Educativa — v1.0.0

**[Baixar Morph Adventure Educativo](https://github.com/victorbpalazzin/MorphAdventure/releases/download/educativo-v1.0.0/MorphAdventure-Educativo-v1.0.0-Windows-x64.zip)**

Esta versão preserva a aventura original e acrescenta:

- Letras colecionáveis no lugar dos fragmentos de progressão.
- Perguntas de Lógica Proposicional.
- Cronômetro de 12 segundos.
- Desbloqueio de portais mediante respostas corretas.
- Segunda chance ao perder todas as vidas.
- Suporte ao controle de PlayStation 4 (DualShock 4).

**Tag:** `educativo-v1.0.0`

**Branch de referência:** `feature/modo-educativo`

### Como instalar

1. Acesse uma das Releases.
2. Baixe o arquivo ZIP correspondente.
3. Extraia todo o conteúdo para uma pasta.
4. Abra `entrega-arcade` ou `entrega-educativo`, conforme a versão.
5. Execute `morph-adventure.exe`.

O jogo funciona localmente e não exige conexão com a internet durante a execução.

Não é necessário instalar CMake, compiladores ou raylib para utilizar os executáveis já distribuídos.

**Compatibilidade principal:** Windows x64, com suporte gráfico a OpenGL 3.3.

---

## 🌌 História e objetivo

Os seis mundos de Morph Adventure estão ameaçados por uma força sombria conhecida como **Grootspectro, o Titã Espectral da Floresta**.

Essa criatura busca destruir o equilíbrio entre os elementos, colocando em risco diferentes ambientes e seus habitantes.

Para enfrentar a ameaça, os protagonistas precisam atravessar seis mundos elementais, superar perigos, reunir recursos e chegar ao confronto final.

Cada mundo apresenta um personagem, uma identidade visual e desafios específicos.

Ao longo da jornada, a dificuldade aumenta progressivamente, exigindo mais precisão, reflexos e domínio das mecânicas.

A aventura termina somente quando o jogador derrota Grootspectro.

---

## 🌍 Os seis mundos

| Mundo | Personagem | Meta de coleta | Principais desafios |
|---|---|---:|---|
| 🌤️ Ar | Passarinho | 4 | Tempestades, rajadas de vento e redemoinhos |
| 🌊 Água | Peixinho | 6 | Corais, rochas, correntes e águas-vivas |
| 🌿 Floresta | Borboleta | 8 | Galhos e cipós espinhosos |
| ❄️ Gelo | Pinguim | 10 | Gelo móvel, nevascas e estilhaços |
| 🏜️ Terra | Raposa voadora | 12 | Rochas, poeira e redemoinhos de areia |
| 🔥 Fogo | Dragãozinho | 16 | Rochas vulcânicas, erupções e bolas de fogo |

As metas de coleta são compartilhadas pelas duas versões.

No **Arcade**, representam a quantidade de fragmentos de energia necessários.

No **Educativo**, representam a quantidade de letras que devem ser coletadas antes do desafio de lógica.

### Progressão e dificuldade

A dificuldade aumenta conforme o jogador avança pelos mundos, com alterações na velocidade, nos obstáculos e nos perigos ambientais.

Cada mundo também possui:

- Cenário com identidade visual própria.
- Personagem temático.
- Camadas de parallax.
- Obstáculos com movimentação e padrões específicos.
- Perigos especiais com avisos visuais.
- Portal de transição personalizado.

Os cenários e personagens são renderizados por código com recursos gráficos da raylib, sem depender de imagens externas durante a execução.

---

## 🕹️ Mecânicas principais

### Movimento e física

O personagem possui movimentação vertical controlada por impulsos.

A gravidade atua continuamente, exigindo que o jogador utilize os comandos de voo para permanecer em uma posição segura.

Os obstáculos se deslocam horizontalmente, produzindo a sensação de progressão constante pelos cenários.

A simulação utiliza passos fixos de **1/60 de segundo**, garantindo consistência na atualização das regras do jogo.

### Sistema de vidas

Cada aventura começa com **três vidas**.

Ao colidir com obstáculos ou perigos ambientais, o jogador pode perder uma vida, respeitando os efeitos de proteção temporária.

Existem corações coletáveis que concedem uma vida adicional nos mundos:

- Água.
- Gelo.
- Fogo.

Cada um desses mundos pode oferecer esse benefício uma vez por aventura.

A passagem entre mundos não restaura automaticamente as vidas.

### Coleta de recursos

Na versão Arcade, o jogador reúne fragmentos de energia para completar a meta necessária à progressão.

Os fragmentos:

- Possuem identificação individual.
- Podem ser coletados por colisão.
- Contribuem para a pontuação.
- Alimentam o progresso do mundo.
- Podem reaparecer caso sejam perdidos.
- São afetados pelo boost de magnetismo.

Quando a meta é alcançada, o percurso chega a 100% e o sistema prepara a transição.

Na versão Educativa, essa mesma mecânica é preservada, substituindo visualmente os fragmentos por letras.

### Portais

Os portais conectam os seis mundos.

Na versão Arcade, são liberados conforme as condições originais de progressão.

Na versão Educativa, o jogador também precisa responder corretamente a um desafio de lógica antes de atravessar.

As transições preservam as animações e os efeitos visuais do jogo.

---

## ⚡ Sistema de boosts

Morph Adventure possui cinco tipos de boosts:

| Boost | Funcionamento | Duração |
|---|---|---|
| 🛡️ Escudo | Absorve um impacto | Até ser consumido |
| 🧲 Ímã | Atrai coletáveis próximos | 7 segundos |
| 👻 Intangibilidade | Permite atravessar perigos sem sofrer dano | 7 segundos |
| ⏳ Slow motion | Reduz a velocidade do mundo para 68% | 4 segundos |
| ✨ Pontos x2 | Duplica as recompensas de pontuação | 7 segundos |

O jogador pode manter até dois tipos de boosts ativos simultaneamente.

Ao coletar um terceiro tipo, o sistema substitui o mais antigo.

Coletar novamente um boost temporizado renova sua duração.

O sistema é compartilhado pelas duas versões do jogo.

---

## ⭐ Sistema de pontuação

A pontuação recompensa ações realizadas ao longo da aventura.

| Ação | Pontos normais | Com boost x2 |
|---|---:|---:|
| Ultrapassar obstáculo | 10 | 20 |
| Coletar recurso | 35 | 70 |
| Atravessar portal | 100 | 200 |

O boost de pontos x2 afeta apenas as recompensas obtidas enquanto estiver ativo.

A pontuação é preservada durante a passagem entre mundos.

---

## 🧠 Modo Educativo — Lógica Proposicional

A versão educativa foi desenvolvida para integrar conceitos introdutórios de Lógica Proposicional à experiência de jogo.

O objetivo é transformar a progressão pelos mundos em uma atividade que combine raciocínio, atenção e entretenimento.

Essa modalidade foi pensada especialmente para apresentações acadêmicas e visitantes que podem não possuir conhecimento prévio da disciplina.

### Letras coletáveis

Os fragmentos de progressão são substituídos visualmente por letras do alfabeto.

O jogador deve coletar a quantidade correspondente ao mundo atual.

Por exemplo, no mundo Água, é necessário reunir seis letras.

O HUD apresenta o progresso utilizando a indicação:

`LETRAS: 3/6`

As letras não precisam formar palavras ou anagramas.

A mecânica utiliza o sistema de coleta já existente, preservando colisões, pontuação, reaparecimento, boosts e progressão.

### Banco de perguntas

O projeto possui um banco com **41 perguntas de Lógica Proposicional**, organizado em `src/edu/questions.c`.

Cada questão apresenta:

- Enunciado.
- Quatro alternativas.
- Uma única resposta correta.
- Identificação do conteúdo.
- Explicação pedagógica breve.

Os temas incluem:

1. Identificação de proposições.
2. Valores lógicos: verdadeiro e falso.
3. Negação.
4. Conjunção (E).
5. Disjunção inclusiva (OU).
6. Condicional (SE... ENTÃO).
7. Tautologia, contradição e contingência.

As perguntas são selecionadas por um sistema de embaralhamento que evita a repetição imediata da mesma questão.

### Exemplo de pergunta

**Qual destas frases pode ser considerada uma proposição lógica?**

A) Feche a porta!

B) Que horas são?

C) O número 2 é par.

D) Bom dia!

**Resposta correta:** C.

**Explicação:** uma proposição é uma afirmação que pode ser classificada como verdadeira ou falsa.

### Cronômetro de 12 segundos

Toda pergunta possui um limite de **12 segundos**.

O tempo restante é apresentado na interface por meio de contador e indicadores visuais.

O cronômetro:

- Começa em 12 segundos para cada pergunta.
- Diminui durante a escolha da alternativa.
- Interrompe a contagem quando a resposta é registrada.
- Não continua diminuindo durante o feedback.
- É reiniciado quando uma nova pergunta aparece.
- É suspenso durante pausas e perda de foco da janela.

Enquanto o jogador responde, a simulação da aventura permanece congelada.

Isso evita colisões, movimentação de obstáculos ou consumo de boosts durante o desafio.

### Perguntas para atravessar portais

Após completar a coleta de letras, o jogador precisa responder corretamente a uma pergunta.

**Se acertar:**

- Recebe o feedback de acerto.
- Desbloqueia o portal.
- Continua a aventura.

**Se errar:**

- Recebe a resposta correta e uma explicação.
- Mantém as letras coletadas.
- Não perde vidas.
- Recebe outra pergunta com 12 segundos.

**Se o tempo acabar:**

- A tentativa é encerrada.
- O jogo informa que o tempo se esgotou.
- Apresenta a explicação.
- Oferece outra pergunta com 12 segundos.

O jogador pode realizar quantas tentativas forem necessárias para avançar.

**O portal somente é liberado após uma resposta correta dentro do prazo.**

No mundo Fogo, o desafio libera o acesso ao confronto final contra Grootspectro.

### Segunda chance — Ressurreição

A versão educativa também possui um sistema de segunda chance.

Quando o jogador perde todas as vidas pela primeira vez, uma pergunta especial substitui temporariamente a derrota.

O jogador possui **12 segundos e uma única tentativa** para responder corretamente.

**Se acertar:**

- Recupera exatamente uma vida.
- Continua no mundo atual.
- Mantém a pontuação.
- Preserva as letras já coletadas.
- Recebe proteção temporária ao retornar.
- Consome definitivamente a segunda chance.

**Se errar ou deixar o tempo acabar:**

- Recebe o feedback da pergunta.
- Não recupera vidas.
- É encaminhado à tela de derrota.

A ressurreição pode acontecer somente uma vez por aventura, inclusive durante o confronto contra o chefe final.

A disponibilidade é restaurada apenas quando uma nova aventura começa.

---

## 🎮 Controles do jogo

### Teclado e mouse

| Ação | Comando |
|---|---|
| Impulsionar personagem | Espaço, W, seta para cima ou clique na área de jogo |
| Disparar megarraio | J, durante a batalha final |
| Pausar ou continuar | P |
| Navegar entre botões | Tab / Shift + Tab |
| Confirmar botão selecionado | Enter |
| Fechar jogo | Esc |
| Responder perguntas | Teclas 1–4 ou A–D |
| Navegar pelas alternativas | Setas para cima e para baixo |
| Confirmar alternativa selecionada | Enter ou Espaço |
| Ligar ou desligar o som | M |
| Ajustar volumes (na pausa) | 1, 2 ou 3 escolhem Geral, Música ou Efeitos; setas ← → ajustam; botões − e + com o mouse |

Teclado e mouse permanecem disponíveis tanto no Arcade quanto no Educativo.

### Controle de PlayStation 4 — DualShock 4

A versão Educativa possui suporte nativo ao controle DualShock 4, conectado por cabo USB ao Windows.

O controle utiliza as funções de gamepad da raylib 5.5, sem exigir obrigatoriamente softwares adicionais.

| Botão do PS4 | Função |
|---|---|
| X (cruz) | Impulsionar personagem / confirmar opções |
| Quadrado | Disparar megarraio contra Grootspectro |
| OPTIONS | Pausar ou retomar |
| Direcional | Navegar pelos menus e perguntas |
| Analógico esquerdo | Navegar pelos menus e perguntas |
| Círculo | Voltar ou cancelar quando a tela permitir |

O sistema identifica conexão e desconexão do dispositivo durante a execução.

Também são utilizados mecanismos para evitar:

- Comandos involuntários ao conectar o controle.
- Repetições excessivas de navegação.
- Impulsos acidentais após confirmar perguntas.
- Conflitos entre ações de menus e ações do personagem.

**Importante:** na implementação atual, o suporte ao controle está ativado somente quando `MORPH_EDUCATIONAL=ON`. A versão Arcade distribuída originalmente utiliza teclado e mouse.

### Testar o controle

Na compilação educativa, o projeto disponibiliza uma tela de diagnóstico para verificar o reconhecimento do controle.

Execute:

```bash
./build-educativo/morph-adventure.exe --gamepad-test
```

Essa tela apresenta o nome do dispositivo conectado, o estado dos botões e os valores do analógico esquerdo.

O suporte prioriza o DualShock 4, mas outros controles compatíveis reconhecidos pela raylib também podem funcionar.

---

## 🌳 Grootspectro — Batalha final

Após atravessar os seis mundos, o jogador enfrenta **Grootspectro, o Titã Espectral da Floresta**.

O confronto apresenta um sistema próprio de combate.

### Mecânicas

- Três segmentos de vida do chefe.
- Projéteis espectrais.
- Ataques com dificuldade progressiva.
- Bateria especial de combate.
- Fragmentos multicoloridos de energia.
- Megarraio arco-íris.
- Animação de derrota e tela de vitória.

O jogador precisa coletar **quatro fragmentos especiais** para carregar um disparo do megarraio.

Cada disparo completo remove um segmento de vida do chefe.

São necessários três disparos bem-sucedidos para derrotá-lo.

Durante o megarraio, o personagem assume uma condição especial de combate e dissipa os ataques presentes.

### Funcionamento nas duas versões

A batalha final utiliza a mesma mecânica no Arcade e no Educativo.

Mesmo na versão Educativa, os fragmentos da bateria de combate **continuam sendo energia, não letras**.

A diferença é que o jogador pode utilizar sua segunda chance durante o confronto, caso ainda não a tenha consumido.

A vitória só é apresentada após a derrota completa de Grootspectro e a conclusão de sua animação.

---

## 🎵 Trilha sonora e efeitos

O Morph Adventure tem trilha e efeitos **originais**, compostos e sintetizados para o jogo, presentes nas versões Arcade e Educativa.

| Momento | Música |
|---|---|
| Menu | Tema de abertura em Ré maior que apresenta o **motivo de Morph** |
| Ar | Flauta, harpa e pads aéreos (Sol lídio) |
| Água | Piano elétrico líquido, sub-grave e voz etérea (Mi dórico) |
| Floresta | Kalimba, pizzicato e madeiras (Lá eólio) |
| Gelo | Sinos de vidro, piano esparso e reverb cristalino (Si menor) |
| Terra | Tambores, violão e trompas híbridas (Ré dórico) |
| Fogo | Percussão intensa, reese, cordas e metais (Dó menor) |
| Grootspectro | Batalha adaptativa em **três camadas** que crescem a cada fase |
| Vitória e derrota | Resolução do motivo e um tema breve para tentar de novo |

O motivo de Morph reaparece transformado em todos os mundos, criando a sensação de uma única jornada.

Mais de quarenta efeitos acompanham voo, coleta (cada coleta sobe uma nota até a bateria encher), boosts, portais, perigos de cada mundo, interface e a batalha final, incluindo a sequência do **megarraio arco-íris** sincronizada com a animação.

A música faz transições suaves entre mundos, congela na pausa e o volume pode ser ajustado sem reiniciar a partida. Todo o áudio é gerado por código no próprio repositório (`tools/audio/`). Detalhes em [AUDIO.md](docs/AUDIO.md).

---

## 🛠️ Tecnologias utilizadas

| Tecnologia | Aplicação |
|---|---|
| C99 | Linguagem principal do jogo |
| raylib 5.5 | Renderização, janela, teclado, mouse, gamepads e áudio |
| Python 3 + numpy + scipy | Composição e síntese do áudio original (ferramentas em `tools/audio/`) |
| FFmpeg / libvorbis | Codificação Ogg Vorbis e medição de loudness do áudio |
| CMake | Configuração do projeto |
| Ninja | Automação da compilação |
| GCC / MSYS2 UCRT64 | Compilação para Windows |
| CTest | Execução dos testes automatizados |
| Git | Controle de versões |
| GitHub | Hospedagem, colaboração e Releases |
| VS Code | Desenvolvimento e manutenção |

O jogo executável foi implementado nativamente em C, sem depender de HTML, CSS ou JavaScript durante sua execução.

O arquivo HTML original foi mantido no repositório como parte do histórico do protótipo.

---

## 🧩 Arquitetura e organização

O projeto utiliza uma estrutura modular, com responsabilidades distribuídas entre os componentes.

```text
MorphAdventure/
│
├── CMakeLists.txt
├── README.md
├── MorphAdventure.html
│
├── src/
│   ├── main.c
│   │
│   ├── core/
│   │   ├── game.c
│   │   ├── game.h
│   │   ├── game_types.h
│   │   ├── composition.c
│   │   └── viewport.c
│   │
│   ├── player/
│   │   └── Personagens, física e colisões
│   │
│   ├── worlds/
│   │   └── Cenários e parallax
│   │
│   ├── obstacles/
│   │   └── Obstáculos e perigos ambientais
│   │
│   ├── items/
│   │   └── Coletáveis, energia e boosts
│   │
│   ├── ui/
│   │   └── HUD, menus e perguntas
│   │
│   ├── effects/
│   │   └── Portais, partículas e transições
│   │
│   ├── boss/
│   │   └── Grootspectro e regras de combate
│   │
│   ├── edu/
│   │   ├── questions.c
│   │   ├── questions.h
│   │   ├── quiz.c
│   │   └── quiz.h
│   │
│   └── input/
│       ├── gamepad.c
│       └── gamepad.h
│
├── tests/
│   ├── core/
│   ├── player/
│   ├── worlds/
│   ├── obstacles/
│   ├── items/
│   ├── ui/
│   ├── boss/
│   ├── edu/
│   └── input/
│
├── cmake/
│   └── Configuração e empacotamento
│
├── docs/
│   └── Contratos e documentação técnica
│
└── scripts/
    └── Scripts auxiliares
```

### Responsabilidades principais

**Core:** coordena os estados, a simulação, os eventos e a progressão da aventura.

**Player:** implementa física, movimentação, colisões e representação dos personagens.

**Worlds:** produz os seis ambientes e as camadas gráficas.

**Obstacles:** controla obstáculos, dificuldade e perigos especiais.

**Items:** gerencia coletáveis, energia, boosts e corações extras.

**UI:** apresenta HUD, botões, menus e desafios.

**Effects:** controla partículas, portais e transições.

**Boss:** implementa as regras do confronto contra Grootspectro.

**Edu:** organiza o banco de perguntas, a seleção das questões, o cronômetro e a avaliação das respostas.

**Input:** converte comandos do controle em ações compatíveis com o sistema de entrada do jogo.

A arquitetura permite aproveitar os mesmos módulos nas versões Arcade e Educativa, evitando duplicação integral do código.

---

## ⚙️ Como compilar

A compilação é realizada preferencialmente pelo **MSYS2 UCRT64**, no Windows.

### Pré-requisitos

- Git.
- MSYS2 UCRT64.
- GCC.
- CMake 3.16 ou superior.
- Ninja.
- Conexão com a internet na primeira configuração, caso a raylib precise ser obtida pelo CMake.

A versão utilizada da raylib é fixada em 5.5.

### Clonar o repositório

```bash
git clone https://github.com/victorbpalazzin/MorphAdventure.git

cd MorphAdventure
```

### Compilar a versão Arcade

A versão Arcade utiliza `MORPH_EDUCATIONAL=OFF`.

```bash
cmake -S . -B build-arcade -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMORPH_MODULES_LINKED=ON \
  -DMORPH_EDUCATIONAL=OFF
```

Compilar:

```bash
cmake --build build-arcade
```

Executar:

```bash
./build-arcade/morph-adventure.exe
```

### Compilar a versão Educativa

Primeiro, selecione a branch que contém a implementação educativa:

```bash
git fetch origin

git switch feature/modo-educativo
```

Configure a compilação:

```bash
cmake -S . -B build-educativo -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMORPH_MODULES_LINKED=ON \
  -DMORPH_EDUCATIONAL=ON
```

Compilar:

```bash
cmake --build build-educativo
```

Executar:

```bash
./build-educativo/morph-adventure.exe
```

### Alternância entre versões

A opção CMake `MORPH_EDUCATIONAL` determina qual experiência será compilada.

| Configuração | Resultado |
|---|---|
| `OFF` | Arcade |
| `ON` | Educativo |

O padrão é `OFF`.

Não existe seleção de modos no menu inicial: as versões são definidas na compilação.

As pastas de build devem permanecer separadas para evitar conflitos de configuração.

---

## ✅ Testes e validação

O projeto utiliza o **CTest** para executar testes automatizados.

### Executar os testes do Arcade

```bash
ctest --test-dir build-arcade --output-on-failure
```

### Executar os testes do Educativo

```bash
ctest --test-dir build-educativo --output-on-failure
```

### Testes realizados

Os testes abrangem:

- Inicialização e encerramento do jogo.
- Atualização da simulação.
- Física e colisões.
- Sistema de vidas.
- Pontuação.
- Fragmentos e letras coletáveis.
- Metas de progressão.
- Boosts.
- Portais e transições.
- Interface e navegação.
- Comportamento da batalha final.
- Banco de questões.
- Cronômetro de 12 segundos.
- Respostas corretas e incorretas.
- Tempo esgotado.
- Segunda chance e ressurreição.
- Conexão e desconexão de controles.
- Mapeamento de comandos do DualShock 4.
- Compatibilidade entre as regras das versões Arcade e Educativa.

### Laboratórios e testes gráficos

O projeto também contém executáveis específicos de validação visual.

Entre eles estão:

- `worlds-preview`: pré-visualização dos seis mundos.
- `edu-lab`: teste de integração das mecânicas educativas.
- `pad-lab`: teste de navegação e comandos de gamepad.

O teste visual dos mundos pode produzir capturas dos cenários e personagens, auxiliando na validação gráfica e na documentação.

### Validação no Windows

A versão Arcade v1.0.0 passou por compilação Release, execução manual e **17 testes automatizados aprovados** na validação registrada.

A versão Educativa v1.0.0 também foi compilada e teve seus testes automatizados executados com sucesso no Windows, além de testes manuais de funcionamento, incluindo o controle DualShock 4.

Os testes automatizados complementam a validação humana, mas não substituem a avaliação contínua de dificuldade e jogabilidade.

---

## 📦 Distribuição e empacotamento

As versões são distribuídas em arquivos ZIP compactos, contendo apenas os elementos necessários à execução.

### Arcade

```text
MorphAdventure-Arcade-v1.0.0-Windows-x64.zip
└── entrega-arcade/
    ├── morph-adventure.exe
    └── LEIA-ME.txt
```

### Educativo

```text
MorphAdventure-Educativo-v1.0.0-Windows-x64.zip
└── entrega-educativo/
    ├── morph-adventure.exe
    └── LEIA-ME.txt
```

A partir das versões com áudio, cada pacote também leva a pasta `assets/audio/` (músicas em Ogg Vorbis e efeitos em WAV/Ogg) ao lado do executável e o arquivo `CREDITOS.txt`. Sem essa pasta o jogo funciona, mas em silêncio.

Os pacotes de distribuição não precisam incluir:

- Código-fonte.
- Cabeçalhos de desenvolvimento da raylib.
- Bibliotecas estáticas usadas na compilação.
- Arquivos de teste.
- Configurações internas do CMake.

O executável é compilado com as dependências apropriadas para Windows, sendo necessário apenas um sistema compatível.

As Releases são mantidas de maneira independente para preservar o histórico do projeto.

---

## 🔀 Organização das branches

| Branch | Finalidade |
|---|---|
| `main` | Base estável da versão Arcade |
| `feature/modo-educativo` | Implementação educativa e suporte ao DualShock 4 |
| `feature/grootspectro` | Desenvolvimento e integração da batalha final |

A versão educativa foi desenvolvida separadamente para evitar alterações involuntárias na experiência Arcade já publicada.

A utilização de branches, commits e revisões facilita o acompanhamento da evolução do projeto.

---

## 🚀 Evolução do Morph Adventure

O desenvolvimento passou por diferentes etapas:

**1. Protótipo inicial**

Criação de um jogo de voo lateral em HTML, inspirado em mecânicas de Flappy Bird.

**2. Migração para C99**

Reconstrução do projeto com raylib 5.5, permitindo execução nativa no Windows.

**3. Modularização**

Divisão do desenvolvimento em sistemas de física, cenários, obstáculos, itens, interface e transições.

**4. Construção dos seis mundos**

Desenvolvimento dos ambientes elementais, com personagens próprios e dificuldade progressiva.

**5. Expansão da jogabilidade**

Implementação de novos perigos ambientais, boosts, coleta de energia e aprimoramentos visuais.

**6. Implementação de Grootspectro**

Criação da batalha final, com projéteis, bateria de combate, três segmentos de vida e megarraio arco-íris.

**7. Versão Arcade**

Compilação e distribuição do jogo completo para Windows.

**8. Versão Educativa**

Adição de letras, banco de perguntas, cronômetro, desafios de progressão e ressurreição.

**9. Suporte ao controle de PS4**

Integração do DualShock 4 à versão educativa, permitindo uma experiência mais próxima de um arcade tradicional.

**10. Testes e distribuição**

Validação automatizada, testes manuais e disponibilização das duas versões por meio do GitHub Releases.

---

## 🤝 Desenvolvimento e colaboração

O projeto foi organizado em sete frentes principais:

1. Núcleo do jogo e integração.
2. Personagens, física e colisões.
3. Cenários e direção de arte.
4. Obstáculos e dificuldade.
5. Coletáveis, energia e boosts.
6. Interface, efeitos e portais.
7. Compilação, testes e distribuição.

O desenvolvimento utiliza Git e GitHub para organização das alterações, integração de funcionalidades e acompanhamento da evolução do software.

As novas funcionalidades educativas e o suporte ao controle foram incorporados à base modular, respeitando os sistemas previamente implementados.

---

## 📚 Documentação adicional

Consulte os documentos técnicos do repositório:

- [BUILD.md](docs/BUILD.md) — compilação e ambiente.
- [TESTING.md](docs/TESTING.md) — testes e validação.
- [CONTRACTS.md](docs/CONTRACTS.md) — contratos de integração.
- [INTEGRATION.md](docs/INTEGRATION.md) — arquitetura e integração.
- [AUDIO.md](docs/AUDIO.md) — trilha sonora, efeitos, volumes e como alterar o áudio.
- [Documentação das entregas](docs/handoffs/) — registros técnicos das frentes de desenvolvimento.

---

## 🎓 Objetivos acadêmicos

O Morph Adventure demonstra a aplicação prática de conhecimentos de Sistemas de Informação, especialmente:

- Programação estruturada em C.
- Organização modular de software.
- Lógica computacional.
- Gerenciamento de estados.
- Desenvolvimento de interfaces.
- Física e detecção de colisões.
- Programação gráfica.
- Integração de dispositivos de entrada.
- Testes automatizados.
- Controle de versões.
- Trabalho colaborativo.

Na versão Educativa, o jogo também explora a utilização da tecnologia como ferramenta de aprendizagem, incentivando o contato com conceitos de Lógica Proposicional de maneira interativa.

---

## 📄 Situação do projeto

**Arcade v1.0.0:** versão publicada para Windows.

**Educativo v1.0.0:** versão publicada para Windows, incluindo desafios de lógica e suporte ao DualShock 4.

O projeto permanece aberto a ajustes de balanceamento, acessibilidade, interface e novas funcionalidades.

Para relatar problemas ou sugerir melhorias, utilize a seção de [Issues](https://github.com/victorbpalazzin/MorphAdventure/issues) do repositório.

---

### 🪽 Morph Adventure

**Seis mundos. Uma única aventura. Novas formas de aprender.**

*Desenvolvido em C99 com raylib 5.5 — CESAR School, 2026.*

**[Repositório no GitHub](https://github.com/victorbpalazzin/MorphAdventure)**
