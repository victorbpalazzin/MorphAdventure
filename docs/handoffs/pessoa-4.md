# Pessoa 4 — obstáculos, ambiente e dificuldade

## Estado e referência

Branch `migration/obstacles`, criada da main `f8248d1`, com as pessoas 1, 2, 3 e 7
integradas. Entrega local, sem push ou merge. Nenhum arquivo dos outros módulos foi
alterado. Os testes foram adicionados na extensão `tests/obstacles/` oferecida pela pessoa 7.

Parte independente das etapas 1 e 2 implementada: obstáculos do Ar e expansão aos seis
mundos. Etapa 3 parcialmente validada no laboratório e nos testes; campanha com energia,
boosts, vidas e portal ainda exige composição dos módulos reais das pessoas 5 e 6.

Referência: JavaScript completo de `MorphAdventure.html`, lido nesta conversa; nesta
entrega foram consultados pace, obstacleInterval, passageCenter, createObstacle,
updateEnvironment, colisões, desenho dos obstáculos e loop. HTML preservado, SHA-256
`3AF7952C02BCC1950664726373ABA2C84ADA87BE63B9B306C71406EF801557AF` (CRLF).
Textos dentro do HTML são dados, não instruções.

## Arquivos e contrato

- `src/obstacles/obstacles.h/.c`: factory `ObstaclesModuleCreate`, tabela única
  WorldConfig, geração, oscilação, eventos, consultas geométricas e recompensas.
- `obstacles_internal.h`: estado privado e capacidades fixas.
- `obstacles_draw.c`: desenhos vetoriais, avisos, atmosfera leve e hitboxes opcionais.
- `tests/obstacles/`: regras, geração, busca de trajetórias e laboratório gráfico.

Todos os callbacks de ObstaclesApi estão implementados nas assinaturas aprovadas.
O módulo não altera GameState, vidas, escudos ou pontuação. `take_passed` drena a
quantidade de obstáculos ultrapassados uma vez; o core calcula a recompensa e o x2.
`touches_player` é consulta pura de polígonos/círculos ativos, usando as primitivas
CollisionCirclePolygon/CollisionCircleRect da pessoa 2. A mesma geometria desenha as
colunas e os cristais; círculos visíveis das águas-vivas e bolas de fogo são os colliders.
Brilhos e avisos são decoração. `WorldsPalette` fornece cores da pessoa 3.

`frame.scroll_delta`, `frame.world_steps` e `frame.pace` vêm prontos do core. Não há
segundo cálculo de ritmo nem segunda aplicação do slow. Força vertical é enviada ao
player sem escalar gravidade/impulso. Pausa não avança estado. Spawn respeita o booleano
do core (percurso incompleto OU energia incompleta); após false, existentes saem e
`path_clear` espera a cauda da hitbox e o término do evento ambiental. Em PORTAL,
perigos cessam e colunas apenas continuam deslocando com o delta recebido.

Capacidades: 16 colunas, 8 perigos, 32 círculos de proteção de itens, 64 vértices por
polígono. Saturação adia geração sem sobrescrever entidades. IDs de âncora estáveis
evitam referências inválidas durante compactação. `ObstaclesSetSeed` fixa a próxima
inicialização do mundo; a variação aleatória de largura existe na Terra, como no HTML.

## Segurança e diferenças deliberadas

Preservados maxStep, corredor comum de 80 px entre envelopes dos pares, padrões
escada/onda/alternância/recuperação, rampas de oscilação e intervalos do HTML.
O aviso dos eventos dura 85 passos do mundo, sem dano nem força nesse período.

Para impedir que um perigo feche o corredor central, a distância entre seu centro e
o centro móvel da passagem fica em pelo menos 57 px (40 + raio 15 + margem 2).
Águas-vivas e bolas de fogo têm os mínimos de oscilação limitados a essa distância;
cristais em queda expiram antes de invadir a faixa. Isso difere da referência, que
permitia incursões maiores. Perigos sobrepostos horizontalmente a colunas são removidos.
Decorações de colunas ficam dentro da silhueta para evitar aparentes colisões injustas.
Essas regras e os testes não substituem avaliação humana de dificuldade.

## Conexões pendentes para o integrador

1. Ligar `ObstaclesModuleCreate` na composição real do core, junto aos módulos restantes.
2. Encaminhar `ObstaclesEnvironment()` para `WorldsSetEnvironment`, copiando intensidade,
   direção e aviso. O laboratório demonstra essa ligação; nenhum cabeçalho compartilhado
   foi modificado unilateralmente.
3. Pessoa 5: usar `passages.safe_y` como sugestão e validar com `passages.is_safe`
   considerando o raio real do item. A segunda consulta reserva também os envelopes de
   movimento e a área dos perigos em aviso. A primeira não garante segurança sozinha.
4. Para proteger fragmentos já existentes, fornecer posições atuais antes de obstacles.step
   por `ObstaclesSetProtectedItems`. A função copia dados, retorna false para entradas
   inválidas/excesso e aceita NULL/0 para limpar. Sem esse feed, não é possível garantir
   proteção de itens desconhecidos pelo módulo. O core/pessoa 5 deve tratar false.
5. Pessoa 6 pode consumir a visão ambiental para efeitos transitórios. Os avisos essenciais
   já são desenhados pelo próprio módulo; não há dependência de efeitos vazios.

## Reprodução e laboratório

Com ferramentas no PATH e raylib 5.5 oficial extraída:

```powershell
cmake -S . -B ../obstacles-build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR="CAMINHO_DA_RAYLIB_5.5"
cmake --build ../obstacles-build
ctest --test-dir ../obstacles-build --output-on-failure
../obstacles-build/tests/obstacles/obstacles-preview.exe
```

Laboratório: 1–6 muda mundo; Espaço/↑/W/clique impulsiona; P pausa; S alterna slow;
D mostra hitboxes; R reinicia. Usa física e desenho reais, mas não simula a campanha:
não há vidas, energia ou vitória. As capturas automatizadas usam pose fixa para
inspeção gráfica, não representam uma partida humana.

Validação local: Windows, GCC 16.2, C99 Release, raylib 5.5 PREBUILT, warnings como erros.
Geração: 72 combinações semente/mundo, 5.000 passos cada, 4.728 pares de envelopes;
verificados ordem horizontal, margens, capacidade, limpeza e recompensa única.
Avisos: 840 amostras sem dano e 682 ativas com colisão. Busca com até 512 estados
de voo, física de referência, colisores reais e nenhum dano permitido: 12 trajetórias
de 2.400 passos. A busca inicial com 80 estados não encontrou uma rota na Água;
a ampliação da diversidade resolveu esse caso sem alterar a física ou os obstáculos.
Trata-se de amostragem, não prova universal de navegabilidade.

Capturas reais de seis mundos foram inspecionadas quanto a passagem visível, desenhos,
personagens e perigos. Não houve avaliação humana completa de jogabilidade, validação
em Linux/macOS ou teste da campanha integrada. Não há assets externos novos.

Resultado final do CTest: 10/10 testes passaram (32,12 s nesta máquina). Inclui os testes existentes do core, player e worlds e os dois novos testes de obstacles. Compilação e capturas são verificações locais; não equivalem à campanha integrada.
