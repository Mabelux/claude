# Script para copiar todas las DLLs de runtime necesarias
# Para Windows - ejecutar desde PowerShell

param(
    [string]$TargetDir = "build\Release"
)

Write-Host "Copiando DLLs de runtime a: $TargetDir" -ForegroundColor Cyan
Write-Host ""

# Verificar que el directorio existe
if (-not (Test-Path $TargetDir)) {
    Write-Host "Error: El directorio $TargetDir no existe" -ForegroundColor Red
    exit 1
}

# Lista completa de DLLs de runtime que pueden ser necesarias
$runtimeDlls = @(
    # Visual C++ 2015-2022 Runtime (VC140/VC141/VC142/VC143)
    "MSVCP140.dll",
    "MSVCP140_1.dll",
    "MSVCP140_2.dll",
    "MSVCP140_ATOMIC_WAIT.dll",
    "MSVCP140_CODECVT_IDS.dll",
    "VCRUNTIME140.dll",
    "VCRUNTIME140_1.dll",
    "VCRUNTIME140D.dll",
    "MSVCP140D.dll",
    "CONCRT140.dll",
    "VCCORLIB140.dll",

    # Visual C++ 2013 Runtime (VC120)
    "msvcr120.dll",
    "msvcp120.dll",
    "msvcr120d.dll",
    "msvcp120d.dll",

    # Visual C++ 2012 Runtime (VC110) - por si acaso
    "msvcr110.dll",
    "msvcp110.dll",

    # Visual C++ 2010 Runtime (VC100) - por si acaso
    "msvcr100.dll",
    "msvcp100.dll"
)

$copiedCount = 0
$notFoundCount = 0

foreach ($dll in $runtimeDlls) {
    # Buscar en System32 primero (64-bit)
    $source = "C:\Windows\System32\$dll"

    if (Test-Path $source) {
        try {
            Copy-Item $source $TargetDir -Force -ErrorAction Stop
            Write-Host "[OK] Copiado: $dll" -ForegroundColor Green
            $copiedCount++
        } catch {
            Write-Host "[ERROR] No se pudo copiar: $dll - $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "[SKIP] No encontrado: $dll" -ForegroundColor Gray
        $notFoundCount++
    }
}

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Resumen:" -ForegroundColor Cyan
Write-Host "  DLLs copiadas: $copiedCount" -ForegroundColor Green
Write-Host "  DLLs no encontradas: $notFoundCount" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Listar todas las DLLs en el directorio destino
Write-Host "DLLs presentes en $TargetDir:" -ForegroundColor Cyan
Get-ChildItem "$TargetDir\*.dll" | ForEach-Object {
    $size = "{0:N2}" -f ($_.Length / 1MB)
    Write-Host "  $($_.Name) - $size MB"
}

Write-Host ""
Write-Host "Ahora prueba ejecutar: .\$TargetDir\dhyana_control.exe --help" -ForegroundColor Yellow
