# Notas de Despliegue - Dhyana Camera Control

## ⚠️ Problema: Dependencias Ocultas de Software de Cámaras

### Contexto

Si tienes instalado **Euresys eGrabber**, **GenICam**, u otro software de captura de frames, este puede crear **dependencias ocultas** que hacen que tu aplicación funcione en el PC de desarrollo pero falle en otros PCs.

### Síntomas

1. **En PC de desarrollo (con eGrabber/GenICam instalado):**
   - ✅ El programa funciona perfectamente
   - ✅ Las DLLs se encuentran automáticamente
   - ✅ La cámara se inicializa sin problemas

2. **En PC de despliegue (sin eGrabber/GenICam):**
   - ❌ Error 0xc000007b al ejecutar
   - ❌ "DLL not found" aunque las DLLs estén presentes
   - ❌ Necesitas copiar archivos manualmente

### ¿Por qué ocurre esto?

Cuando instalas software como **Euresys eGrabber**, este:

1. **Agrega directorios al PATH del sistema:**
   ```
   C:\Program Files\Euresys\eGrabber\bin
   C:\Program Files\Common Files\GenICam\v3.1\bin\Win64_x64
   ```

2. **Configura variables de entorno:**
   ```
   GENICAM_ROOT_V3_1=C:\Program Files\Common Files\GenICam\v3.1
   GENICAM_GENTL64_PATH=C:\Program Files\Euresys\eGrabber\bin
   ```

3. **Instala DLLs del sistema:**
   - GCBase_MD_VC141_v3_2.dll
   - GenApi_MD_VC141_v3_2.dll
   - Y muchas otras...

Estas DLLs están **disponibles globalmente** en tu PC de desarrollo, por lo que Windows las encuentra aunque no estén en el directorio del ejecutable.

### Solución

Para que tu aplicación funcione en **cualquier PC** sin depender de software instalado:

#### Opción 1: Copiar TODO automáticamente (RECOMENDADO)

El `CMakeLists.txt` actualizado ahora copia:
- ✅ Todas las DLLs del SDK (37 archivos)
- ✅ DLLs de Visual C++ Runtime
- ✅ Directorio `engine/` con archivos `.dat`

```bash
# Recompilar después de actualizar CMakeLists.txt
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Opción 2: Configurar PATH del sistema (Para desarrollo)

```powershell
# En PowerShell como Administrador
$tucsenPath = "C:\Program Files\TUCam_SDK\sdk\lib\x64"
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "Machine") + ";$tucsenPath",
    "Machine"
)
```

**Ventaja:** No necesitas copiar DLLs cada vez que compiles
**Desventaja:** Solo funciona en tu PC, no es portable

#### Opción 3: Crear paquete de despliegue

Crea un directorio con TODO lo necesario:

```powershell
# Crear paquete
mkdir dhyana_control_package
cd dhyana_control_package

# Copiar ejecutable
Copy-Item "..\build\Release\dhyana_control.exe" .

# Copiar TODAS las DLLs del SDK
Copy-Item "C:\Program Files\TUCam_SDK\sdk\lib\*.dll" .

# Copiar engine/
mkdir engine
Copy-Item "C:\Program Files\TUCam_SDK\sdk\lib\All_x64\engine\*" .\engine\

# Copiar Visual C++ Runtime
Copy-Item "C:\Windows\System32\msvcp120.dll" .
Copy-Item "C:\Windows\System32\msvcr120.dll" .
Copy-Item "C:\Windows\System32\msvcp140.dll" .
Copy-Item "C:\Windows\System32\vcruntime140.dll" .
Copy-Item "C:\Windows\System32\vcruntime140_1.dll" .
```

Ahora puedes copiar toda la carpeta `dhyana_control_package` a cualquier PC Windows.

### Verificar Dependencias

Usa el script de diagnóstico incluido:

```powershell
.\check_environment.ps1
```

Este script te mostrará:
- ✅ Qué software de cámaras tienes instalado
- ✅ Qué directorios están en tu PATH
- ✅ Qué variables de entorno están configuradas
- ✅ Qué DLLs tienes en System32
- ✅ Qué archivos faltan en tu build

### Herramientas de Diagnóstico

#### Dependencies.exe

Descarga: https://github.com/lucasg/Dependencies

```powershell
# Analizar dependencias
Dependencies.exe dhyana_control.exe
```

Busca:
- ❌ Líneas rojas = DLLs faltantes
- ⚠️ "FILE NOT FOUND" = Problema de ruta

#### dumpbin (Visual Studio)

```powershell
# Ver dependencias de DLL
dumpbin /dependents dhyana_control.exe
dumpbin /dependents TUCam.dll

# Ver arquitectura (debe ser x64)
dumpbin /headers dhyana_control.exe | Select-String "machine"
```

### Checklist de Despliegue

Antes de copiar tu ejecutable a otro PC:

- [ ] Ejecutar `check_environment.ps1` y verificar que no haya "dependencias ocultas"
- [ ] Verificar que `build\Release\` tiene ~40 DLLs
- [ ] Verificar que `build\Release\engine\` existe y tiene archivos `.dat`
- [ ] Probar en una máquina virtual limpia (sin eGrabber/GenICam)
- [ ] Documentar cualquier software prerequisito

### Notas Adicionales

#### ¿Qué es eGrabber?

**Euresys eGrabber** es un framework de captura de frames para cámaras industriales que soporta:
- GenICam / GenTL
- GigE Vision
- Camera Link
- CoaXPress

Si lo usas, probablemente tienes cámaras adicionales o hardware de captura de frames instalado.

#### ¿Afecta a este proyecto?

Solo si:
1. Las DLLs de eGrabber están en tu PATH
2. El SDK de Tucsen depende indirectamente de componentes GenICam
3. No copias todos los archivos necesarios al directorio Release

La solución es **siempre copiar todas las DLLs localmente** para que el ejecutable sea auto-contenido.

---

## Resumen

**Problema:** Software instalado (eGrabber, GenICam) agrega DLLs al PATH, creando dependencias ocultas.

**Solución:** CMake copia automáticamente TODAS las DLLs necesarias al directorio Release.

**Verificación:** Usa `check_environment.ps1` para diagnosticar tu entorno.

**Despliegue:** Copia toda la carpeta Release (con DLLs y engine/) al PC destino.

---

**Última actualización:** 2025-11-14
**Versión:** 1.0.0
