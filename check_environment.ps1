# Script de diagnóstico del entorno para Dhyana Camera Control
# Verifica configuración de DLLs, PATH y variables de entorno relacionadas con cámaras

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Diagnóstico de Entorno" -ForegroundColor Cyan
Write-Host "  Dhyana Camera Control" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. Verificar PATH del sistema
Write-Host "[1] Verificando PATH del sistema..." -ForegroundColor Yellow
Write-Host ""

$pathEntries = $env:Path -split ';'
$relevantPaths = $pathEntries | Where-Object {
    $_ -like "*Tucsen*" -or
    $_ -like "*TUCam*" -or
    $_ -like "*Euresys*" -or
    $_ -like "*eGrabber*" -or
    $_ -like "*GenICam*" -or
    $_ -like "*GenTL*"
}

if ($relevantPaths) {
    Write-Host "Directorios relacionados con cámaras en PATH:" -ForegroundColor Green
    foreach ($path in $relevantPaths) {
        Write-Host "  - $path" -ForegroundColor White
    }
} else {
    Write-Host "No se encontraron directorios relacionados con cámaras en PATH" -ForegroundColor Yellow
}

Write-Host ""

# 2. Verificar variables de entorno de GenICam/GenTL
Write-Host "[2] Verificando variables de entorno GenICam/GenTL..." -ForegroundColor Yellow
Write-Host ""

$genicamVars = @(
    "GENICAM_ROOT_V3_0",
    "GENICAM_ROOT_V3_1",
    "GENICAM_ROOT_V3_2",
    "GENICAM_GENTL32_PATH",
    "GENICAM_GENTL64_PATH",
    "GENICAM_LOG_CONFIG_V3_0",
    "GENICAM_LOG_CONFIG_V3_1"
)

$foundVars = $false
foreach ($varName in $genicamVars) {
    $value = [Environment]::GetEnvironmentVariable($varName, "Machine")
    if ($value) {
        Write-Host "  $varName = $value" -ForegroundColor Green
        $foundVars = $true
    }
}

if (-not $foundVars) {
    Write-Host "No se encontraron variables de entorno GenICam" -ForegroundColor Yellow
    Write-Host "Nota: Esto es normal si no tienes eGrabber u otro software GenICam instalado" -ForegroundColor Gray
}

Write-Host ""

# 3. Verificar instalaciones de software relacionado
Write-Host "[3] Verificando software de cámaras instalado..." -ForegroundColor Yellow
Write-Host ""

$softwareToCheck = @(
    @{Name="Tucsen SDK"; Path="C:\Program Files\TUCam_SDK"},
    @{Name="Euresys eGrabber"; Path="C:\Program Files\Euresys"},
    @{Name="Euresys eGrabber (x86)"; Path="C:\Program Files (x86)\Euresys"}
)

foreach ($sw in $softwareToCheck) {
    if (Test-Path $sw.Path) {
        Write-Host "  [OK] $($sw.Name) encontrado: $($sw.Path)" -ForegroundColor Green
    } else {
        Write-Host "  [ ] $($sw.Name) no encontrado" -ForegroundColor Gray
    }
}

Write-Host ""

# 4. Verificar DLLs críticas en System32
Write-Host "[4] Verificando DLLs críticas en System32..." -ForegroundColor Yellow
Write-Host ""

$criticalDlls = @(
    "msvcp120.dll",
    "msvcr120.dll",
    "msvcp140.dll",
    "vcruntime140.dll",
    "vcruntime140_1.dll"
)

foreach ($dll in $criticalDlls) {
    $path = "C:\Windows\System32\$dll"
    if (Test-Path $path) {
        $fileInfo = Get-Item $path
        $size = "{0:N0}" -f ($fileInfo.Length / 1KB)
        Write-Host "  [OK] $dll ($size KB)" -ForegroundColor Green
    } else {
        Write-Host "  [X] $dll NO ENCONTRADO" -ForegroundColor Red
    }
}

Write-Host ""

# 5. Verificar archivos en directorio actual
Write-Host "[5] Verificando archivos en directorio Release..." -ForegroundColor Yellow
Write-Host ""

$releaseDir = ".\build\Release"
if (Test-Path $releaseDir) {
    $dllCount = (Get-ChildItem "$releaseDir\*.dll" -ErrorAction SilentlyContinue).Count
    $datFiles = Get-ChildItem "$releaseDir\engine\*.dat" -ErrorAction SilentlyContinue

    Write-Host "  DLLs encontradas: $dllCount" -ForegroundColor $(if ($dllCount -gt 30) {"Green"} else {"Yellow"})

    if ($datFiles) {
        Write-Host "  Archivos .dat en engine/:" -ForegroundColor Green
        foreach ($dat in $datFiles) {
            Write-Host "    - $($dat.Name)" -ForegroundColor White
        }
    } else {
        Write-Host "  [!] No se encontraron archivos .dat en engine/" -ForegroundColor Red
        Write-Host "      Ejecuta: mkdir $releaseDir\engine" -ForegroundColor Yellow
        Write-Host "      Luego copia archivos .dat desde C:\Program Files\TUCam_SDK\sdk\lib\All_x64\engine\" -ForegroundColor Yellow
    }
} else {
    Write-Host "  Directorio Release no encontrado. Compila primero." -ForegroundColor Yellow
}

Write-Host ""

# 6. Resumen y recomendaciones
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Resumen" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($relevantPaths) {
    Write-Host "[IMPORTANTE] Se detectó software de cámaras en PATH:" -ForegroundColor Yellow
    Write-Host "  Esto puede causar conflictos o dependencias ocultas." -ForegroundColor Yellow
    Write-Host "  En el PC de desarrollo funciona porque estas DLLs están disponibles," -ForegroundColor Yellow
    Write-Host "  pero en otro PC sin este software instalado, puede fallar." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Recomendación:" -ForegroundColor Cyan
    Write-Host "  Asegúrate de que CMake copie TODAS las DLLs necesarias automáticamente." -ForegroundColor White
}

Write-Host ""
Write-Host "Para más información, lee el README.md sección 'Despliegue a Nuevo Ordenador'" -ForegroundColor Cyan
Write-Host ""
