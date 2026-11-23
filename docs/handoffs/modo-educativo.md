# Handoff — Modo educativo (feira de Lógica)

Responsável: Lívia Buonora (GitHub `liviabuonora`). Branch: `feature/modo-educativo`, criada a partir de `main` (`f1c6c5b`).

## Duas versões, um projeto

| | Arcade (`MORPH_EDUCATIONAL=OFF`, padrão) | Educativo (`MORPH_EDUCATIONAL=ON`) |
|---|---|---|
| Coletáveis de progressão | Fragmentos de energia | Letras (mesmo item, mesmas regras) |
| HUD | `ENERGIA 3/6 50%` | `LETRAS: 3/6` |
| Portal | Bateria completa + caminho livre | Idem + acerto de uma pergunta de Lógica |
| Derrota | Tela de derrota | Uma pergunta de "ÚLTIMA CHANCE!" por aventura |
| Grootspectro | Original | Original (energia de combate, não letras) |

```bash
cmake -S . -B build-edu -DCMAKE_BUILD_TYPE=Release -DMORPH_MODULES_LINKED=ON -DMORPH_EDUCATIONAL=ON
cmake --build build-edu
./build-edu/morph-adventure
```

## Arquivos

Novos: `src/edu/questions.{h,c}` (banco), `src/edu/quiz.{h,c}` (regras puras do desafio),
`src/ui/ui_quiz_draw.c` (painel), `tests/edu/` (quatro testes) e este documento.

Alterados: `CMakeLists.txt` (opção e fontes em `morph_core`), `src/core/game_types.h`
(contrato v6), `src/core/game.{h,c}` (coordenação), `src/main.c` (teclas do desafio e semente),
`src/items/items.{h,c}`, `items_internal.h`, `items_draw.c` (letras), `src/ui/ui.{h,c}`,
`ui_internal.h`, `ui_draw.c` (HUD, entrada e painel), `src/obstacles/obstacles.c`
(intervalo 510), testes arcade fixados com `GameInitMode(..., false)`, docs.

## Banco de perguntas

41 perguntas fáceis em `src/edu/questions.c`, cada uma com enunciado, quatro alternativas,
uma correta (0..3), explicação curta e assunto: proposições, verdadeiro/falso, negação,
conjunção (E), disjunção inclusiva (OU), condicional (SE... ENTÃO) e tautologia/contradição/
contingência. Conectivos por extenso, sem símbolos. Sorteio por baralho embaralhado
(semente da sessão + `run_id`): todas aparecem antes de repetir e nunca a mesma em seguida.
Para acrescentar perguntas basta incluir itens no vetor `bank` (limite: 64).

## Cronômetro

`QuizState.seconds_left` começa em 12. Corre com o tempo real de cada quadro (limitado a
0,1 s), nunca com `world_steps`, slow motion ou o acumulador da simulação. Pausa (P) e
perda de foco suspendem; ao voltar, o tempo da pausa não é aplicado. Em um quadro, o tempo
é descontado antes da resposta; se chegar a zero, a resposta daquele quadro é recusada
(TEMPO ESGOTADO). No feedback o relógio fica parado. Barra decrescente, número grande,
amarelo a partir de 5 s, vermelho e pulsação nos últimos 3 s.

## Controles do desafio

Clique na alternativa; teclas `1`–`4` ou `A`–`D`; setas ↑/↓ e `Enter`/`Espaço`.
Durante o desafio `W`, `↑` e `J` não voam/atacam, e o foco começa vazio, então um `Espaço`
que o jogador já vinha apertando não responde. As respostas só valem após 0,35 s e o botão
de continuar após 0,5 s de feedback.

## Controle (DualShock 4) — somente na versão educativa

Apenas o build educativo lê o controle; o arcade continua só com teclado e mouse,
exatamente como antes. Na versão educativa: X voa e confirma, Quadrado dispara o megarraio,
OPTIONS pausa e retoma, direcional ou analógico esquerdo navega, Círculo volta (pausa,
derrota e vitória). Textos de ajuda trocam para os botões do controle quando ele está
conectado. Detalhes em `docs/CONTRACTS.md` (seção Controle). Para conferir no Windows:

```bash
morph-adventure --gamepad-test
```

Pressione X, Círculo, Quadrado, OPTIONS e o direcional: cada linha deve acender com o nome
certo (o número raylib esperado aparece na tela). Se o controle aparecer mas nenhum botão
acender, a raylib não tem mapeamento para ele e o jogo continua apenas com teclado e mouse.

## Testes executados

Arcade e educativo (pastas de build separadas), Release, Linux, xvfb: 23/23 em cada
(inclui `input-gamepad` e `input-pad-flow`, que exercita o controle na versão educativa). Lógica com AddressSanitizer + UBSan
(`MORPH_BUILD_GAME=OFF`): 9/9. O controle foi testado por simulação do estado da raylib;
o DualShock 4 físico ainda precisa ser conferido no Windows com `--gamepad-test`. Executável
`morph-adventure` dos dois modos abre e roda 30 quadros. Capturas do laboratório:
`build-edu/tests/edu/captures/edu-*.png`.

## Limitações

- O laboratório gráfico prepara a energia completa e a posição das letras, como `ui_lab`;
  a sequência real de jogo, portal, quiz e boss vem dos módulos de produção.
- Testado em Linux; não executado em Windows nesta entrega.
- Proteção após reviver: 180 passos (3 s) no core, além dos 75 passos de invulnerabilidade
  visual do jogador. Balanceamento com visitantes reais pode pedir ajuste.
- O ajuste dos perigos horizontais para 8,5 s foi aplicado nos dois modos (a observação
  não restringia o modo).
