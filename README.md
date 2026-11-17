# 🪽 Morph Adventure

**Uma jornada através dos elementos. Seis mundos, três vidas e energia para continuar.**

Morph Adventure é um jogo de voo com rolagem lateral desenvolvido em **C99 com raylib 5.5**. Desvie dos obstáculos, reúna energia e atravesse portais enquanto seu personagem se transforma para explorar cada ambiente.

O alvo principal é **Windows desktop**. Depois de instalado ou compilado, o jogo funciona localmente, sem conexão com a internet.

> 🚧 **Em desenvolvimento:** a integração foi validada localmente no Windows. Arte, dificuldade e balanceamento ainda podem receber ajustes.

## 🎮 Como jogar no Windows

### Baixar uma versão pronta

1. Acesse a [página de Releases](https://github.com/victorbpalazzin/MorphAdventure--PIF/releases).
2. Escolha uma versão com um **pacote do jogo para Windows** em **Assets**.
3. Baixe esse ZIP. Os arquivos automáticos **Source code** contêm os fontes, não o jogo compilado.
4. Extraia **todo o conteúdo** para uma pasta do computador.
5. Abra **`morph-adventure.exe`** dentro da pasta extraída.
6. Clique em **Começar aventura** ou pressione **Espaço**.

Para usar um pacote pronto, não é necessário instalar compilador, CMake ou raylib.

> **Se não houver um pacote Windows na página de Releases**, o download pronto ainda não está disponível. Nesse caso, siga as instruções de compilação abaixo. **Code → Download ZIP** também baixa apenas os fontes.

## 🕹️ Controles

| Ação | Controle |
|---|---|
| Impulsionar o personagem | **Espaço**, **↑**, **W** ou clique na área de jogo |
| Pausar ou continuar | **P** ou botão de pausa |
| Navegar pelos botões | **Tab** / **Shift + Tab** |
| Ativar o botão em foco | **Enter**; nos menus, também **Espaço** |
| Tentar novamente | Botão **Tentar novamente** na derrota |
| Reiniciar pelo menu | Botão **Voltar ao início** |
| Fechar o jogo | **Esc** |

Cliques nos botões não impulsionam o personagem. Ao perder o foco da janela durante a partida, o jogo pausa; retome com **P** ou **Continuar**.

## 🧭 Objetivo da aventura

Você tem **três vidas para os seis mundos**. A passagem por um portal **não recupera vidas**.

1. Desvie das colunas e dos perigos ambientais.
2. Colete a energia exigida pelo mundo atual.
3. Complete também o percurso indicado no HUD.
4. Quando **energia e percurso** estiverem completos, novos obstáculos deixam de surgir.
5. Espere o caminho ficar livre e atravesse o portal.
6. Passe pelo portal do **Fogo** para concluir a aventura.

Fragmentos que passam sem serem coletados voltam a ser oferecidos. Perder um fragmento não impede completar a bateria.

Ao perder todas as vidas, **Tentar novamente** inicia uma nova partida no Ar, com três vidas e pontuação zerada. A troca normal de mundo preserva vidas e pontuação, mas reinicia energia e boosts.

## 🌍 Os seis mundos

| Mundo | Personagem | Energia necessária | Desafios |
|---|---|---:|---|
| 🌤️ Ar | Passarinho | 4 | Nuvens de tempestade e rajadas de vento |
| 🌊 Água | Peixinho | 8 | Corais, rochas, correntes e águas-vivas |
| 🌿 Floresta | Borboleta | 12 | Galhos e cipós em movimento |
| ❄️ Gelo | Pinguim | 16 | Gelo móvel, nevasca e cristais em queda |
| 🏜️ Terra | Raposa voadora | 20 | Rochas e redemoinhos de areia |
| 🔥 Fogo | Dragãozinho | 24 | Rochas vulcânicas, erupções e bolas de fogo |

A velocidade aumenta e as passagens ficam mais estreitas ao longo da aventura. Cada mundo possui cenário, personagem e moldura de portal próprios. Perigos especiais apresentam um aviso antes de causar dano.

## ⚡ Boosts

| Boost | Efeito | Duração |
|---|---|---|
| 🛡️ Escudo | Absorve um impacto | Até ser consumido; uma carga |
| 🧲 Ímã | Atrai fragmentos próximos | 5 segundos |
| ⏳ Slow motion | Reduz o ritmo do mundo para 68% | 4 segundos |
| ✨ Pontos x2 | Dobra as recompensas de pontos | 7 segundos |

É possível manter **até dois boosts ativos**. Um terceiro tipo substitui o mais antigo. Coletar novamente um boost temporizado renova sua duração; escudos não acumulam cargas.

O slow motion afeta o mundo, mantendo a resposta do voo. A pausa congela os temporizadores da partida.

## ⭐ Pontuação

| Ação | Pontos | Com x2 ativo |
|---|---:|---:|
| Ultrapassar um obstáculo | 10 | 20 |
| Coletar um fragmento | 35 | 70 |
| Entrar em um portal | 100 | 200 |

## 🛠️ Compilar no Windows

### 1. Preparar as ferramentas

Para desenvolver ou compilar, você precisa de **Git**, **MSYS2 UCRT64 com GCC**, **CMake 3.16 ou superior** e **Ninja**. Consulte as instruções de instalação em [docs/BUILD.md](docs/BUILD.md).

A raylib é fixada em **5.5** pelo projeto. Na configuração padrão, o CMake busca essa dependência pela internet. O build da dependência também precisa do compilador C++ incluído no pacote GCC do MSYS2, embora o jogo seja escrito em C.

### 2. Baixar o código

No VS Code, abra **Terminal → Novo Terminal**, selecione **PowerShell** e execute na pasta onde deseja guardar o projeto:

```powershell
git clone https://github.com/victorbpalazzin/MorphAdventure--PIF.git
cd MorphAdventure--PIF
```

Os comandos exigem a versão integrada do repositório, que contém `src/core/composition.c`. Se esse arquivo ainda não estiver na branch principal, use a branch de integração disponibilizada pela equipe.

### 3. Configurar e compilar

Se o MSYS2 estiver instalado em `C:\msys64`, adicione as ferramentas UCRT64 ao PATH **desse terminal**:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;" + $env:Path
```

Se a instalação estiver em outro local, ajuste o caminho. Depois execute:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_MODULES_LINKED=ON
cmake --build build
```

A configuração deve mostrar **`modulos_ligados=ON`**. Essa opção conecta personagens, cenários, obstáculos, itens, interface e portais ao executável do jogo.

### 4. Executar os testes

```powershell
ctest --test-dir build --output-on-failure
```

### 5. Abrir o jogo

```powershell
.\build\morph-adventure.exe
```

Depois da primeira configuração, use `cmake --build build` para recompilar alterações. `morph-core-shell.exe` é uma ferramenta de diagnóstico; para jogar, abra `morph-adventure.exe`.

### Usar uma cópia local da raylib

Se você já tem o pacote **raylib 5.5 para MinGW-w64** extraído, configure o caminho para a pasta que contém `include/` e `lib/`:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMORPH_MODULES_LINKED=ON -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR="C:/libs/raylib-5.5_win64_mingw-w64"
cmake --build build
```

Substitua `C:/libs/...` pelo caminho real. Essa opção evita o download da raylib durante a configuração.

### Dificuldades comuns

| Mensagem ou situação | O que verificar |
|---|---|
| `cmake`, `ninja` ou compilador não encontrado | Instalação das ferramentas e PATH do terminal |
| `modulos_ligados=OFF` | Configure novamente com `-DMORPH_MODULES_LINKED=ON` |
| `src/core/composition.c` ausente | Confira se está usando a versão integrada do repositório |
| Falha ao baixar raylib | Confira a conexão ou use o provedor `PREBUILT` |
| Gerador ou compilador diferente do cache anterior | Use uma nova pasta, como `build-ucrt`, em todos os comandos |
| Abriu uma tela de diagnóstico | Execute `morph-adventure.exe` |

## 🧩 Organização do projeto

```text
src/
├── main.c       # Janela, entrada e ciclo principal
├── core/        # Coordenação, regras, estados e composição
├── player/      # Personagens, física e colisões
├── worlds/      # Cenários e parallax
├── obstacles/   # Obstáculos, perigos e dificuldade
├── items/       # Fragmentos, energia e boosts
├── ui/          # HUD, menus e botões
└── effects/     # Partículas, portais e transições
tests/          # Testes automatizados e laboratórios visuais
docs/           # Contratos, build, integração e entregas
cmake/          # Configuração de dependências e empacotamento
scripts/        # Scripts auxiliares
```

O visual é produzido em código, com formas vetoriais, seis personagens animados, cenários em parallax e portais contínuos.

## ✅ Validação e documentação

A integração foi compilada e executada localmente em **Windows com MSYS2 UCRT64, Ninja e raylib 5.5**. Na validação registrada, **16 testes passaram**, abrangendo regras, física, colisões, energia, boosts, interface e transições.

Os testes de transição usam condições controladas para alcançar os portais. Eles não substituem uma partida humana completa nem garantem o balanceamento. A campanha integrada ainda não foi validada em todos os sistemas operacionais.

- [Como compilar](docs/BUILD.md)
- [Integração dos módulos](docs/INTEGRATION.md)
- [Testes](docs/TESTING.md)
- [Contratos de integração](docs/CONTRACTS.md)
- [Entregas da equipe](docs/handoffs/)

## 🤝 Equipe e colaboração

O desenvolvimento foi dividido em sete frentes:

1. Núcleo e integração.
2. Personagens, física e colisões.
3. Cenários e direção de arte.
4. Obstáculos, ambiente e dificuldade.
5. Energia, coletáveis e boosts.
6. Interface, efeitos e portais.
7. Build, testes e distribuição.

Encontrou um problema? Abra uma [Issue](https://github.com/victorbpalazzin/MorphAdventure--PIF/issues) com o mundo em que aconteceu, os passos para reproduzir, a versão utilizada e, se possível, uma captura de tela.

---

**Morph Adventure — uma jornada através dos elementos, desenvolvida em C com raylib.**


Feito com 💛 como protótipo de jogo.
