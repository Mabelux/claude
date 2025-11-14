# Script para buscar el archivo Dhyana 2100.dat en todo el sistema
# Ejecutar como Administrador para búsqueda completa

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  Buscando archivo Dhyana 2100.dat" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

# Buscar en todo el disco C:
Write-Host "[1/5] Buscando en todo C:\ (puede tardar varios minutos)..." -ForegroundColor Yellow
$allDatFiles = Get-ChildItem -Path "C:\" -Recurse -Filter "*.dat" -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -like "*Dhyana*" -or $_.Name -like "*2100*" }

if ($allDatFiles) {
    Write-Host "  Archivos .dat relacionados con Dhyana encontrados:" -ForegroundColor Green
    $allDatFiles | ForEach-Object { Write-Host "    $($_.FullName)" -ForegroundColor White }
} else {
    Write-Host "  No se encontraron archivos .dat de Dhyana" -ForegroundColor Red
}
Write-Host ""

# Buscar software de Tucsen instalado
Write-Host "[2/5] Buscando software de Tucsen..." -ForegroundColor Yellow
$tucsenPaths = @(
    "C:\Program Files\Tucsen Photonics",
    "C:\Program Files (x86)\Tucsen Photonics",
    "C:\Program Files\TUCam_SDK",
    "C:\Program Files (x86)\TUCam_SDK",
    "C:\Program Files\Tucsen",
    "C:\Program Files (x86)\Tucsen",
    "C:\TCapture",
    "C:\ISCapture",
    "C:\SamplePro"
)

$foundSoftware = @()
foreach ($path in $tucsenPaths) {
    if (Test-Path $path) {
        Write-Host "  ENCONTRADO: $path" -ForegroundColor Green
        $foundSoftware += $path

        # Buscar archivos .dat en este directorio
        $datFiles = Get-ChildItem -Path $path -Recurse -Filter "*.dat" -ErrorAction SilentlyContinue
        if ($datFiles) {
            Write-Host "    Archivos .dat en este directorio:" -ForegroundColor Cyan
            $datFiles | ForEach-Object { Write-Host "      $($_.FullName)" -ForegroundColor White }
        }
    }
}

if ($foundSoftware.Count -eq 0) {
    Write-Host "  No se encontró software de Tucsen instalado" -ForegroundColor Red
}
Write-Host ""

# Buscar en AppData
Write-Host "[3/5] Buscando en AppData..." -ForegroundColor Yellow
$appDataPaths = @(
    "$env:LOCALAPPDATA\Tucsen",
    "$env:APPDATA\Tucsen",
    "$env:PROGRAMDATA\Tucsen"
)

foreach ($path in $appDataPaths) {
    if (Test-Path $path) {
        Write-Host "  ENCONTRADO: $path" -ForegroundColor Green
        $datFiles = Get-ChildItem -Path $path -Recurse -Filter "*.dat" -ErrorAction SilentlyContinue
        if ($datFiles) {
            $datFiles | ForEach-Object { Write-Host "    $($_.FullName)" -ForegroundColor White }
        }
    }
}
Write-Host ""

# Verificar programas instalados en el registro
Write-Host "[4/5] Verificando programas instalados (Registro)..." -ForegroundColor Yellow
$uninstallPaths = @(
    "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*",
    "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*"
)

$tucsenPrograms = @()
foreach ($path in $uninstallPaths) {
    $programs = Get-ItemProperty $path -ErrorAction SilentlyContinue |
        Where-Object { $_.DisplayName -like "*Tucsen*" }
    $tucsenPrograms += $programs
}

if ($tucsenPrograms) {
    Write-Host "  Programas de Tucsen instalados:" -ForegroundColor Green
    $tucsenPrograms | ForEach-Object {
        Write-Host "    $($_.DisplayName)" -ForegroundColor White
        if ($_.InstallLocation) {
            Write-Host "      Ubicación: $($_.InstallLocation)" -ForegroundColor Gray
        }
    }
} else {
    Write-Host "  No se encontraron programas de Tucsen en el registro" -ForegroundColor Red
}
Write-Host ""

# Buscar en Downloads y Desktop por si lo descargó manualmente
Write-Host "[5/5] Buscando en Descargas y Escritorio..." -ForegroundColor Yellow
$userPaths = @(
    "$env:USERPROFILE\Downloads",
    "$env:USERPROFILE\Desktop",
    "$env:PUBLIC\Desktop"
)

foreach ($path in $userPaths) {
    if (Test-Path $path) {
        $datFiles = Get-ChildItem -Path $path -Recurse -Filter "*.dat" -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "*Dhyana*" -or $_.Name -like "*Tucsen*" }
        if ($datFiles) {
            Write-Host "  Encontrado en $path" ":" -ForegroundColor Green
            $datFiles | ForEach-Object { Write-Host "    $($_.FullName)" -ForegroundColor White }
        }
    }
}
Write-Host ""

# Resumen y recomendaciones
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  RESUMEN Y RECOMENDACIONES" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "El archivo 'Dhyana 2100.dat' es CRÍTICO para el funcionamiento de la cámara." -ForegroundColor Yellow
Write-Host ""
Write-Host "Este archivo contiene:" -ForegroundColor White
Write-Host "  - Configuración específica del modelo Dhyana 2100" -ForegroundColor Gray
Write-Host "  - Rangos de exposición y ganancia" -ForegroundColor Gray
Write-Host "  - Capacidades del sensor" -ForegroundColor Gray
Write-Host "  - Parámetros de calibración" -ForegroundColor Gray
Write-Host ""

Write-Host "OPCIONES PARA OBTENER EL ARCHIVO:" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. SOFTWARE OFICIAL DE TUCSEN" -ForegroundColor Yellow
Write-Host "   - Instalar TCapture, ISCapture o SamplePro desde:" -ForegroundColor White
Write-Host "     www.tucsen.com" -ForegroundColor Gray
Write-Host "   - Estos programas incluyen el archivo .dat correcto" -ForegroundColor Gray
Write-Host ""

Write-Host "2. CONTACTAR A TUCSEN" -ForegroundColor Yellow
Write-Host "   - Email: service@tucsen.com" -ForegroundColor White
Write-Host "   - Tel: +86-591-28055080-818" -ForegroundColor White
Write-Host "   - Solicitar específicamente: Dhyana 2100.dat" -ForegroundColor Gray
Write-Host ""

Write-Host "3. VERIFICAR SDK COMPLETO" -ForegroundColor Yellow
Write-Host "   - Reinstalar el SDK completo de TUCam" -ForegroundColor White
Write-Host "   - Asegurarse de instalar TODOS los componentes" -ForegroundColor Gray
Write-Host ""

Write-Host "4. BUSCAR EN EL USB QUE VINO CON LA CÁMARA" -ForegroundColor Yellow
Write-Host "   - El USB original debe contener el archivo .dat" -ForegroundColor White
Write-Host ""

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "Presiona cualquier tecla para salir..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
