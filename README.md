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

Teclado e mouse permanecem disponíveis tanto no Arcade quanto no Educativo.

### Controle de PlayStation 4 — DualShock 4

A versão Educativa possui suporte nativo ao controle DualShock 4, conectado por cabo USB ao Windows.

O controle utiliza as funções de gamepad da raylib 5.5, sem exigir obrigatoriamente softwares
