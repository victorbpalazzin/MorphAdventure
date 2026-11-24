# Gera o pacote ZIP no Windows e verifica que ele abre fora da pasta de desenvolvimento.
# Uso (PowerShell, na raiz do repositório):
#   .\scripts\package\package.ps1                         # MinGW (MSYS2 UCRT64) + Ninja
#   .\scripts\package\package.ps1 -Generator "Visual Studio 17 2022"
#   .\scripts\package\package.ps1 -RaylibDir C:\libs\raylib-5.5_win64_mingw-w64
# Não instala nada. Requer cmake, compilador C e git (ou -RaylibDir) no PATH.
# ATENÇÃO: escrito pela pessoa 7 mas AINDA NÃO EXECUTADO em Windows. Ver docs/TESTING.md.
param(
    [string]$BuildDir = "build-package",
    [string]$Generator = "Ninja",
    [string]$RaylibDir = ""
)
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
if (-not [System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir = Join-Path $Root $BuildDir }

function Invoke-Step([string]$Title, [scriptblock]$Block) {
    Write-Host "== $Title"
    & $Block
    if ($LASTEXITCODE -ne 0) { throw "Falhou: $Title (código $LASTEXITCODE)" }
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "cmake não encontrado no PATH. Veja docs/BUILD.md."
}

$configure = @("-S", $Root, "-B", $BuildDir, "-G", $Generator)
if ($Generator -notlike "Visual Studio*") { $configure += "-DCMAKE_BUILD_TYPE=Release" }
if ($RaylibDir) { $configure += @("-DMORPH_RAYLIB_PROVIDER=PREBUILT", "-DMORPH_RAYLIB_DIR=$RaylibDir") }

Invoke-Step "Configurando" { cmake @configure }
Invoke-Step "Compilando"   { cmake --build $BuildDir --config Release --parallel }
Invoke-Step "Testando"     { ctest --test-dir $BuildDir -C Release --output-on-failure }
Push-Location $BuildDir
try { Invoke-Step "Empacotando" { cpack -C Release } } finally { Pop-Location }

$zip = Get-ChildItem (Join-Path $BuildDir "package") -Filter "MorphAdventure-*.zip" |
       Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $zip) { throw "ZIP não encontrado em $BuildDir\package" }
Write-Host "== Pacote: $($zip.FullName)"

$clean = Join-Path ([System.IO.Path]::GetTempPath()) ("morph-clean-" + [guid]::NewGuid())
New-Item -ItemType Directory $clean | Out-Null
try {
    Expand-Archive $zip.FullName -DestinationPath $clean
    $pkg = Get-ChildItem $clean -Directory | Select-Object -First 1
    Write-Host "== Conteúdo extraído em pasta limpa:"
    Get-ChildItem $pkg.FullName | Format-Table Name, Length
    $exe = Join-Path $pkg.FullName "morph-core-shell.exe"
    Invoke-Step "Executando a partir da pasta limpa" {
        cmake "-DEXE=$exe" "-DWORK_DIR=$clean\smoke" "-DLAUNCHER=" -P (Join-Path $Root "tests\smoke\run_shell_smoke.cmake")
    }
    $game = Join-Path $pkg.FullName "morph-adventure.exe"
    if (Test-Path $game) {
        foreach ($f in @("assets\audio\music\menu.ogg", "assets\audio\music\boss_3.ogg", "assets\audio\sfx\beam.ogg", "CREDITOS.txt")) {
            if (-not (Test-Path (Join-Path $pkg.FullName $f))) { throw "Pacote sem $f" }
        }
        Invoke-Step "Jogo e áudio a partir da pasta limpa" {
            $log = Join-Path $clean "jogo.log"
            Push-Location $clean
            try { & $game --smoke-frames 90 *> $log } finally { Pop-Location }
            Get-Content $log | Select-String "AUDIO: Audio:|Audio desativado"
            if (Select-String -Path $log -Pattern "pasta assets/audio nao encontrada" -Quiet) { throw "O jogo extraído não achou o áudio" }
        }
    }
    Write-Host "OK: $($zip.Name) abre fora da pasta de desenvolvimento neste computador."
} finally {
    Remove-Item -Recurse -Force $clean -ErrorAction SilentlyContinue
}
