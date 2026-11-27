# 🪽 Morph Adventure

### Seis mundos. Uma única aventura.

**Morph Adventure** é um jogo de aventura com rolagem lateral, desenvolvido em **C99 com raylib 5.5** por estudantes de Sistemas de Informação da **CESAR School**.

Inspirado inicialmente na mecânica de voo de *Flappy Bird*, o projeto evoluiu para uma aventura com seis mundos elementais, personagens próprios, obstáculos dinâmicos, coleta de energia, poderes especiais e uma batalha final contra Grootspectro, o Titã Espectral da Floresta.

O jogo possui duas edições: **Arcade**, voltada à aventura e aos reflexos, e **Educativa**, que incorpora desafios de Lógica Proposicional à progressão.

A versão mais recente do Arcade, **v1.1.0**, também conta com trilha sonora original, efeitos sonoros e suporte ao controle DualShock 4.

---

## 🎮 Versões disponíveis

| Versão | Características | Status |
|---|---|---|
| Arcade v1.1.0 | Seis mundos, energia, Grootspectro, áudio e controle de PS4 | Publicada |
| Educativo v1.1.0 | Letras, desafios de lógica, cronômetro, segunda chance, áudio e controle de PS4 | Publicada |
| Arcade v1.0.0 | Primeira edição Arcade completa | Versão anterior |
| Educativo v1.0.0 | Primeira edição com desafios de lógica | Versão anterior |

### 📥 Downloads

**[Baixar Arcade v1.1.0 — Áudio + DualShock 4](https://github.com/victorbpalazzin/MorphAdventure/releases/tag/v1.1.0)**

Edição Arcade atualizada, com músicas originais, efeitos sonoros, controles de volume e compatibilidade com DualShock 4 conectado por USB.

**[Baixar Educativo v1.1.0 — Áudio + DualShock 4](https://github.com/victorbpalazzin/MorphAdventure/releases/tag/educativov-1.1.0)**

Edição que combina desafios de Lógica Proposicional com coleta de letras, perguntas de múltipla escolha, sistema de segunda chance, trilhas sonoras e efeitos específicos para os desafios, além de suporte ao DualShock 4.

As versões anteriores continuam disponíveis na página de Releases do projeto.

**[Ver todas as Releases](https://github.com/victorbpalazzin/MorphAdventure/releases)**

### Como instalar

1. Acesse a Release desejada.
2. Baixe o arquivo ZIP disponível em **Assets**.
3. Extraia todo o conteúdo.
4. Abra a pasta `entrega-arcade` ou `entrega-educativo`.
5. Execute `morph-adventure.exe`.

Na versão Arcade v1.1.0, mantenha a pasta `assets/audio` junto do executável. Ela contém todas as músicas e os efeitos necessários.

Os pacotes prontos não exigem instalação de CMake, GCC ou raylib.

**Plataforma principal:** Windows x64.

---

## 🌌 História

Uma força sombria conhecida como **Grootspectro, o Titã Espectral da Floresta**, ameaça destruir o equilíbrio entre os mundos.

Diante dessa ameaça, seis personagens precisam enfrentar ambientes hostis, superar obstáculos e atravessar portais para alcançar o confronto final.

Cada mundo apresenta condições próprias de jogabilidade, exigindo adaptação e precisão.

A aventura começa no Ar e segue por Água, Floresta, Gelo, Terra e Fogo, até a batalha contra Grootspectro.

---

## 🌍 Os seis mundos

| Mundo | Personagem | Meta de coleta | Principais desafios |
|---|---|---:|---|
| 🌤️ Ar | Passarinho | 4 | Tempestades, rajadas e redemoinhos |
| 🌊 Água | Peixinho | 6 | Corais, correntes e águas-vivas |
| 🌿 Floresta | Borboleta | 8 | Galhos e cipós espinhosos |
| ❄️ Gelo | Pinguim | 10 | Nevascas, gelo móvel e estilhaços |
| 🏜️ Terra | Raposa voadora | 12 | Rochas, poeira e redemoinhos de areia |
| 🔥 Fogo | Dragãozinho | 16 | Erupções, rochas vulcânicas e bolas de fogo |

No Arcade, as metas representam fragmentos de energia necessários para avançar.

Na edição Educativa, as metas de progressão são representadas por letras colecionáveis.

### Progressão

O jogador deve evitar colisões, coletar os recursos necessários e completar a progressão do mundo para alcançar o portal.

Conforme a aventura avança, os obstáculos se tornam mais exigentes e novos perigos ambientais são apresentados.

Cada mundo possui cenários, personagens, animações e efeitos próprios.

Os ambientes utilizam camadas de parallax e elementos gráficos produzidos por código.

---

## 🕹️ Jogabilidade

### Movimento

A jogabilidade utiliza uma mecânica de voo baseada em impulsos verticais.

O personagem é afetado pela gravidade, exigindo comandos frequentes para manter sua posição e atravessar espaços entre os obstáculos.

O jogador também deve reagir a perigos ambientais que possuem movimentos e avisos próprios.

### Sistema de vidas

A aventura começa com **três vidas**.

Colisões e danos podem reduzir esse total, enquanto determinados coletáveis oferecem recuperação.

Há corações extras nos mundos Água, Gelo e Fogo, disponíveis uma vez por mundo.

As vidas restantes são preservadas durante as transições, inclusive na entrada da batalha final.

### Energia e portais

No Arcade, fragmentos de energia são coletados para completar a bateria do mundo.

Quando a meta de progressão é atingida, novos obstáculos deixam de surgir, permitindo que o jogador alcance o portal.

Fragmentos perdidos podem reaparecer, evitando que uma coleta não realizada impeça a conclusão da fase.

### Boosts

O jogo possui cinco poderes especiais:

| Boost | Efeito | Duração |
|---|---|---|
| 🛡️ Escudo | Protege contra um impacto | Até ser consumido |
| 🧲 Ímã | Atrai fragmentos próximos | 7 segundos |
| 👻 Intangibilidade | Permite atravessar perigos sem sofrer dano | 7 segundos |
| ⏳ Slow motion | Reduz o ritmo do mundo para 68% | 4 segundos |
| ✨ Pontos x2 | Duplica a pontuação recebida | 7 segundos |

O jogador pode manter até dois tipos de boosts simultaneamente. A coleta de um terceiro tipo substitui o mais antigo.

### Pontuação

| Ação | Pontuação |
|---|---:|
| Ultrapassar obstáculo | 10 pontos |
| Coletar fragmento | 35 pontos |
| Atravessar portal | 100 pontos |

O boost de pontos x2 duplica as recompensas recebidas durante seu efeito.

---

## 🌳 Batalha final — Grootspectro

Ao concluir o mundo Fogo, o jogador enfrenta **Grootspectro, o Titã Espectral da Floresta**.

A batalha possui três segmentos de vida e ataques que aumentam de intensidade conforme o chefe é atingido.

### Mecânicas do combate

- Projéteis espectrais e avisos de ataque.
- Fragmentos multicoloridos de energia.
- Bateria de combate independente da progressão dos mundos.
- Megarraio arco-íris.
- Três fases de dificuldade.
- Animação de derrota do chefe e encerramento narrativo.

O jogador precisa coletar **quatro fragmentos de combate** para carregar um disparo.

Cada megarraio completo remove um segmento de vida de Grootspectro. São necessários três disparos para derrotá-lo.

Na versão Arcade v1.1.0, a batalha também possui efeitos sonoros exclusivos e uma trilha dinâmica que evolui conforme as fases do chefe.

---

## 🎵 Sistema de áudio — novidade da v1.1.0

A versão Arcade v1.1.0 introduz um sistema de áudio completo, integrado à raylib 5.5.

As músicas e os efeitos foram desenvolvidos especificamente para Morph Adventure, seguindo uma identidade sonora que acompanha a progressão da aventura.

### Trilhas sonoras

| Ambiente | Identidade musical |
|---|---|
| Menu inicial | Tema de abertura e apresentação do motivo musical |
| Ar | Flautas, harpa e texturas leves |
| Água | Piano elétrico, sons fluidos e ambientação submersa |
| Floresta | Kalimba, instrumentos de madeira e atmosfera natural |
| Gelo | Sinos, piano e ambientação cristalina |
| Terra | Tambores, cordas e elementos épicos |
| Fogo | Percussão intensa, sintetizadores e sensação de urgência |
| Grootspectro | Música de combate com três camadas progressivas |
| Vitória | Tema de conclusão da aventura |
| Derrota | Tema breve de encerramento e recomeço |

As composições compartilham motivos musicais que reaparecem em diferentes ambientes, contribuindo para a continuidade da experiência.

### Efeitos sonoros

O jogo também possui sons específicos para ações e acontecimentos como:

- Impulsos de voo.
- Coleta de energia.
- Bateria completa.
- Colisões e perda de vida.
- Coleta de corações.
- Ativação dos cinco boosts.
- Abertura e travessia de portais.
- Surgimento de obstáculos e perigos ambientais.
- Aparição e ataques de Grootspectro.
- Carregamento e disparo do megarraio.
- Navegação, confirmação e pausa nos menus.

Os efeitos são acionados pelos eventos reais do jogo, evitando reproduções repetitivas sem necessidade.

### Áudio dinâmico

O sistema implementa transições graduais entre as músicas, evitando cortes bruscos nas mudanças de mundo.

Durante o confronto com Grootspectro, novas camadas musicais são incorporadas conforme a batalha avança.

A pausa também interrompe temporariamente a reprodução, que continua quando o jogador retoma a partida.

### Configurações de volume

É possível ajustar separadamente:

- Volume geral.
- Volume das músicas.
- Volume dos efeitos.

O jogador também pode silenciar completamente o jogo.

As configurações são armazenadas no arquivo `morph-audio.cfg`, ao lado do executável, quando a pasta permite gravação.

A ausência de um dispositivo de áudio ou dos arquivos musicais não impede a execução do jogo, que pode continuar em silêncio.

### Arquivos e documentação

O sistema está organizado principalmente em:

- `src/audio/` — reprodução, controle de volume e gerenciamento de efeitos.
- `assets/audio/music/` — músicas.
- `assets/audio/sfx/` — efeitos sonoros.
- `tools/audio/` — ferramentas de criação e processamento.
- `docs/AUDIO.md` — documentação técnica do sistema.
- `assets/audio/CREDITOS.txt` — créditos e origem dos recursos.

**[Consultar a documentação completa de áudio](docs/AUDIO.md)**

---

## 🕹️ Controles

### Teclado e mouse

| Ação | Comando |
|---|---|
| Voar | Espaço, W, ↑ ou clique na área de jogo |
| Megarraio | J |
| Pausar / continuar | P |
| Navegar pelos menus | Tab / Shift + Tab |
| Confirmar opção | Enter |
| Silenciar / ativar áudio | M |
| Selecionar canal de áudio na pausa | 1, 2 ou 3 |
| Ajustar volume na pausa | ← / → ou botões da interface |
| Fechar jogo | Esc |

### DualShock 4 — PlayStation 4

A partir da **Arcade v1.1.0**, o jogo também oferece suporte ao controle DualShock 4.

| Botão | Função |
|---|---|
| X (cruz) | Voar e confirmar opções |
| Quadrado | Disparar megarraio |
| OPTIONS | Pausar e continuar |
| Direcional | Navegar pelos menus |
| Analógico esquerdo | Navegar pelos menus |
| Círculo | Voltar ou cancelar quando permitido |

O suporte utiliza os recursos de gamepad da raylib, com detecção de conexão e desconexão durante a execução.

A utilização por cabo USB foi validada no Windows. Outros controles reconhecidos pela raylib podem funcionar, mas não fazem parte da validação específica desta Release.

**Atenção ao áudio do Windows:** ao conectar um DualShock 4 por USB, o sistema pode selecionar automaticamente o controle como dispositivo de saída sonora. Caso o jogo fique sem som, selecione os alto-falantes ou fones desejados nas configurações de áudio do Windows.

---

## 🧠 Versão Educativa

Além da experiência Arcade, Morph Adventure possui uma edição desenvolvida para o aprendizado introdutório de Lógica Proposicional.

Essa versão substitui os fragmentos de progressão por letras e adiciona desafios para permitir a passagem pelos portais.

### Funcionalidades

- Coleta de letras do alfabeto.
- Banco com 41 perguntas de lógica.
- Quatro alternativas por pergunta.
- Cronômetro de 12 segundos.
- Feedback de acertos e erros.
- Novas perguntas após erro ou tempo esgotado nos portais.
- Segunda chance ao perder todas as vidas.
- Recuperação de uma vida após acerto no desafio de ressurreição.
- Suporte ao DualShock 4.

O banco de questões abrange proposições, valores lógicos, negação, conjunção, disjunção, condicional e classificação de expressões lógicas.

A versão Educativa mantém a aventura pelos seis mundos e o confronto final contra Grootspectro.

**Importante:** a versão publicada `educativo-v1.0.0` é independente do Arcade v1.1.0. Suas funcionalidades são mantidas em branch e Release próprias, e as novidades do áudio Arcade não devem ser consideradas automaticamente presentes no pacote educativo já lançado.

---

## 🛠️ Tecnologias utilizadas

| Tecnologia | Utilização |
|---|---|
| C99 | Implementação do jogo |
| raylib 5.5 | Gráficos, entrada e reprodução de áudio |
| CMake | Configuração da compilação |
| Ninja | Sistema de build |
| GCC / MSYS2 UCRT64 | Compilação no Windows |
| CTest | Testes automatizados |
| Python, NumPy e SciPy | Geração das músicas e dos efeitos |
| FFmpeg / libvorbis | Codificação e processamento de áudio |
| Git | Controle de versões |
| GitHub | Repositório e distribuição |
| VS Code | Ambiente de desenvolvimento |

O executável utiliza C99 e raylib. Python e as bibliotecas de síntese são ferramentas utilizadas na produção dos recursos sonoros, não sendo necessários para jogar as versões distribuídas.

---

## 🧩 Estrutura do repositório

```text
MorphAdventure/
│
├── CMakeLists.txt
├── README.md
├── MorphAdventure.html
│
├── src/
│   ├── main.c
│   ├── core/
│   ├── player/
│   ├── worlds/
│   ├── obstacles/
│   ├── items/
│   ├── effects/
│   ├── ui/
│   ├── boss/
│   ├── input/
│   └── audio/
│
├── assets/
│   └── audio/
│       ├── music/
│       ├── sfx/
│       ├── manifest.json
│       └── CREDITOS.txt
│
├── tests/
│   ├── core/
│   ├── player/
│   ├── worlds/
│   ├── obstacles/
│   ├── items/
│   ├── ui/
│   ├── boss/
│   ├── input/
│   └── audio/
│
├── tools/
│   └── audio/
│
├── docs/
│   ├── BUILD.md
│   ├── TESTING.md
│   ├── CONTRACTS.md
│   ├── INTEGRATION.md
│   ├── AUDIO.md
│   └── handoffs/
│
├── cmake/
└── scripts/
```

Na branch Educativa, o projeto inclui adicionalmente `src/edu/` e os respectivos testes.

### Organização modular

O jogo é dividido em módulos responsáveis por gerenciar regras, física, cenários, obstáculos, itens, efeitos, interface, áudio e dispositivos de entrada.

O núcleo coordena os estados e a progressão, enquanto os demais módulos executam responsabilidades específicas.

Essa separação facilita testes, manutenção e integração entre as diferentes implementações.

---

## ⚙️ Compilação

### Pré-requisitos

Para compilar no Windows, recomenda-se utilizar:

- Git.
- MSYS2 UCRT64.
- GCC.
- CMake 3.16 ou superior.
- Ninja.

O projeto utiliza raylib 5.5, obtida automaticamente pelo CMake quando necessário.

### Clonar o repositório

```bash
git clone https://github.com/victorbpalazzin/MorphAdventure.git
cd MorphAdventure
```

### Compilar o Arcade

A branch `main` contém a versão Arcade atualizada.

No terminal MSYS2 UCRT64:

```bash
cmake -S . -B build-arcade -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMORPH_MODULES_LINKED=ON

cmake --build build-arcade
```

Executar:

```bash
./build-arcade/morph-adventure.exe
```

O build sincroniza os arquivos de áudio necessários para a pasta `build-arcade/assets/audio`.

### Compilar a versão Educativa

Para trabalhar com a edição Educativa, utilize sua branch específica:

```bash
git fetch origin
git switch feature/modo-educativo
```

Configure um diretório de build independente:

```bash
cmake -S . -B build-educativo -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMORPH_MODULES_LINKED=ON \
  -DMORPH_EDUCATIONAL=ON

cmake --build build-educativo
```

Executar:

```bash
./build-educativo/morph-adventure.exe
```

**Observação:** a opção `MORPH_EDUCATIONAL` pertence à implementação educativa. A branch `main` do Arcade não utiliza essa opção. Para alternar entre as edições, selecione a branch correspondente.

---

## ✅ Testes e validação

O projeto utiliza CTest para verificar os módulos e as regras de funcionamento.

### Executar testes

```bash
ctest --test-dir build-arcade --output-on-failure
```

Na validação da versão Arcade com áudio e controle DualShock 4, foram registrados:

- **20 testes executados e aprovados.**
- Nenhuma falha entre os testes executados.
- Um teste adicional de áudio desabilitado.
- Compilação Release bem-sucedida.
- Funcionamento manual do controle e do áudio confirmado no Windows.
- Pacote ZIP validado quanto à integridade.
- Execução verificada após extração em uma pasta separada do ambiente de desenvolvimento.

Os testes abrangem lógica, física, cenários, obstáculos, boosts, integração, áudio e comportamento do jogo.

O sistema de áudio possui verificações próprias de reprodução, eventos sonoros, preferências e integração com as diferentes etapas da aventura.

Os testes automatizados complementam, mas não substituem, a avaliação manual de jogabilidade e qualidade sonora.

Para detalhes adicionais, consulte [TESTING.md](docs/TESTING.md).

---

## 👥 Créditos e desenvolvimento

**Morph Adventure — Projeto acadêmico de Sistemas de Informação, CESAR School, 2026.**

O projeto foi construído de forma colaborativa, com desenvolvimento dividido em sete frentes principais.

| Frente | Responsabilidade |
|---|---|
| Pessoa 1 | Núcleo e integração |
| Pessoa 2 | Personagens, física e colisões |
| Pessoa 3 | Cenários e direção visual |
| Pessoa 4 | Obstáculos e dificuldade |
| Pessoa 5 | Energia, coletáveis e boosts |
| Pessoa 6 | Interface, efeitos e portais |
| Pessoa 7 | Build, testes, validação e distribuição |

O desenvolvimento também envolveu etapas posteriores de evolução do jogo, incluindo a batalha contra Grootspectro, a versão Educativa, o suporte ao DualShock 4 e a integração do sistema de áudio.

Os registros técnicos de cada frente estão disponíveis em [docs/handoffs/](docs/handoffs/).

### 🎵 Créditos de áudio

As músicas e os efeitos sonoros foram produzidos especificamente para Morph Adventure pela equipe do projeto.

Conforme registrado em `assets/audio/CREDITOS.txt`, a composição, síntese, mixagem e masterização dos recursos sonoros foram realizadas por meio das ferramentas próprias do repositório.

A produção utiliza Python, NumPy, SciPy e FFmpeg/libvorbis.

O material sonoro foi gerado por código, sem utilização de gravações, loops, amostras, melodias ou efeitos de terceiros.

Os arquivos de áudio podem ser regenerados pelas ferramentas presentes em `tools/audio/`.

A reprodução durante o jogo utiliza raylib 5.5, distribuída sob licença zlib/libpng, com componentes de áudio como miniaudio e stb_vorbis.

**Créditos e documentação oficial:**

- [CREDITOS.txt](assets/audio/CREDITOS.txt)
- [AUDIO.md](docs/AUDIO.md)
- [Ferramentas de produção sonora](tools/audio/)

### Reconhecimentos

O projeto representa a aplicação conjunta de programação, lógica, design de jogos, testes de software, colaboração e desenvolvimento de soluções tecnológicas.

Agradecimentos à equipe de desenvolvimento e à CESAR School pelo contexto acadêmico e pela oportunidade de transformar esses conhecimentos em uma aplicação prática.

---

## 📄 Licenciamento

O repositório ainda não possui uma licença geral de distribuição e reutilização formalmente declarada.

A ausência de uma licença explícita não significa que o código e os recursos estejam disponíveis para reutilização irrestrita.

As músicas e os efeitos acompanham a distribuição do jogo, conforme registrado no arquivo `assets/audio/CREDITOS.txt`.

A adoção de uma licença específica para o projeto e para os recursos sonoros deverá ser definida pela equipe.

A licença zlib/libpng da raylib é independente do licenciamento do código e do conteúdo original de Morph Adventure.

---

## 🚀 Histórico de versões

| Versão | Principais entregas |
|---|---|
| v0.1.0 | Primeira versão de testes para Windows |
| Arcade v1.0.0 | Aventura completa, seis mundos e Grootspectro |
| Educativo v1.0.0 | Letras, perguntas de lógica, cronômetro e segunda chance |
| Arcade v1.1.0 | Trilha sonora original, efeitos sonoros, volume e DualShock 4 |

As versões publicadas permanecem disponíveis na seção [Releases](https://github.com/victorbpalazzin/MorphAdventure/releases).

---

## 📚 Documentação

- [Compilação e ambiente](docs/BUILD.md)
- [Testes e validação](docs/TESTING.md)
- [Contratos do projeto](docs/CONTRACTS.md)
- [Integração dos módulos](docs/INTEGRATION.md)
- [Documentação de áudio](docs/AUDIO.md)
- [Entregas e registros técnicos](docs/handoffs/)

Para relatar um problema ou sugerir uma melhoria, utilize a seção [Issues](https://github.com/victorbpalazzin/MorphAdventure/issues).

---

### 🪽 Morph Adventure

**Seis mundos. Uma única aventura.**

*Desenvolvido em C99 com raylib 5.5 — CESAR School, 2026.*

[Repositório](https://github.com/victorbpalazzin/MorphAdventure) • [Releases](https://github.com/victorbpalazzin/MorphAdventure/releases)
