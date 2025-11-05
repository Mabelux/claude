@echo off
REM Script para copiar todas las DLLs del SDK al directorio del ejecutable
REM Uso: copy_dlls.bat [directorio_destino]
REM Si no se especifica directorio, se usa build\Release

setlocal enabledelayedexpansion

echo ========================================
echo  Copiando DLLs del SDK de Tucsen
echo ========================================
echo.

REM Obtener el directorio de destino
if "%~1"=="" (
    set "DEST_DIR=build\Release"
) else (
    set "DEST_DIR=%~1"
)

REM Verificar que el directorio de destino existe
if not exist "%DEST_DIR%" (
    echo Error: El directorio de destino no existe: %DEST_DIR%
    echo.
    echo Opciones:
    echo   1. Compilar el proyecto primero
    echo   2. Especificar un directorio valido: copy_dlls.bat ruta\al\directorio
    pause
    exit /b 1
)

REM Verificar que el directorio sdk/lib existe
if not exist "sdk\lib" (
    echo Error: El directorio sdk\lib no existe
    echo Asegurese de ejecutar este script desde la raiz del proyecto
    pause
    exit /b 1
)

echo Directorio origen: sdk\lib
echo Directorio destino: %DEST_DIR%
echo.

REM Copiar todas las DLLs
set COUNT=0
for %%f in (sdk\lib\*.dll) do (
    echo Copiando %%~nxf...
    copy /Y "%%f" "%DEST_DIR%\" >nul
    if !errorlevel! equ 0 (
        set /a COUNT+=1
    ) else (
        echo   [ERROR] No se pudo copiar %%~nxf
    )
)

echo.
echo ========================================
echo  Resumen
echo ========================================
echo Total de DLLs copiadas: %COUNT%
echo Directorio: %DEST_DIR%
echo.
echo Las DLLs han sido copiadas correctamente.
echo Ahora puedes ejecutar dhyana_control.exe
echo.
pause
