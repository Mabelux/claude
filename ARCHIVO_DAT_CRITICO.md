# ⚠️ PROBLEMA CRÍTICO: Falta archivo Dhyana 2100.dat

## 🔴 Síntoma del Problema

Cuando ejecutas el programa, ves:
```
[TUCSEN]: C:\Users\...\engine\Dhyana 2100.dat does not exist !
Camera opened successfully.
Initializing camera...
  WARNING: Could not get camera model. Error: 0x1

> info
Model:                    (vacío)
Serial Number:            (vacío)
Firmware Version: 0
Sensor Size:      0 x 0 pixels

> caps
Exposicion: NO SOPORTADA (Error: 0x80000312)
Ganancia: NO SOPORTADA (Error: 0x80000312)
```

## 🎯 Causa Raíz

**El SDK de Tucsen requiere un archivo de configuración específico para cada modelo de cámara.**

El archivo `Dhyana 2100.dat` contiene:
- ✅ Rangos válidos de exposición
- ✅ Rangos válidos de ganancia
- ✅ Capacidades del sensor
- ✅ Parámetros de calibración de fábrica
- ✅ Información del modelo

**Sin este archivo, el SDK:**
- ❌ No puede identificar el modelo de cámara
- ❌ No conoce los rangos válidos de parámetros
- ❌ Retorna error 0x80000312 (NOT_SUPPORTED) para todas las propiedades
- ❌ No puede configurar exposición, ganancia, etc.

## 📁 ¿Dónde debería estar el archivo?

```
dhyana_control/
└── build/
    └── Release/
        ├── dhyana_control.exe
        └── engine/               ← Directorio crítico
            └── Dhyana 2100.dat   ← Este archivo DEBE existir
```

## 🔍 Cómo Encontrar el Archivo

### Opción 1: Ejecutar el Script de Búsqueda

```powershell
cd C:\Users\Optica\Desktop\claude\claude
.\find_dhyana_dat.ps1
```

Este script buscará el archivo en:
- Todo el disco C:\
- Carpetas de software de Tucsen
- AppData y ProgramData
- Registro de Windows
- Descargas y Escritorio

### Opción 2: Buscar Manualmente

**Lugares comunes donde puede estar:**

1. **Software oficial de Tucsen:**
   ```
   C:\Program Files\Tucsen Photonics\TCapture\engine\
   C:\Program Files\Tucsen Photonics\ISCapture\engine\
   C:\Program Files\Tucsen Photonics\SamplePro\engine\
   ```

2. **SDK de Tucsen:**
   ```
   C:\Program Files\TUCam_SDK\sdk\lib\All_x64\engine\
   C:\Program Files\TUCam_SDK\sdk\lib\x64\engine\
   ```

3. **USB que vino con la cámara:**
   - El USB original de fábrica debe contener el archivo

## ✅ Soluciones

### Solución 1: Instalar Software Oficial de Tucsen

El software oficial (TCapture, ISCapture, o SamplePro) incluye el archivo .dat correcto.

1. **Descargar desde:**
   - Sitio oficial: https://www.tucsen.com
   - Sección de "Downloads" o "Software"
   - Buscar "Dhyana 2100" o "TCapture"

2. **Instalar el software completo**

3. **Copiar el archivo .dat:**
   ```powershell
   # Después de instalar, buscar el archivo
   Get-ChildItem "C:\Program Files\Tucsen Photonics" -Recurse -Filter "Dhyana 2100.dat"

   # Copiarlo a tu proyecto
   Copy-Item "C:\Program Files\Tucsen Photonics\...\Dhyana 2100.dat" `
             "C:\Users\Optica\Desktop\claude\claude\build\Release\engine\"
   ```

### Solución 2: Contactar a Tucsen Directamente

Si no puedes encontrar el archivo, solicítalo directamente:

📧 **Email:** service@tucsen.com
📞 **Tel:** +86-591-28055080-818
🌐 **Web:** http://www.tucsen.com

**Mensaje sugerido:**
```
Asunto: Solicitud de archivo Dhyana 2100.dat

Estimado equipo de Tucsen,

Tengo una cámara Dhyana 2100 y necesito el archivo de configuración
"Dhyana 2100.dat" para usar la cámara con el SDK de TUCAM.

Información de mi cámara:
- Modelo: Dhyana 2100
- [Incluir número de serie si lo tienes]

¿Podrían proporcionarme este archivo o indicarme cómo obtenerlo?

Gracias,
[Tu nombre]
```

### Solución 3: Reinstalar SDK Completo

1. **Desinstalar completamente el SDK actual:**
   ```powershell
   # Panel de Control → Programas y características
   # Buscar "Tucsen" o "TUCam SDK"
   # Desinstalar
   ```

2. **Descargar la versión más reciente del SDK**

3. **Instalar con TODAS las opciones marcadas:**
   - ☑️ SDK Headers
   - ☑️ SDK Libraries
   - ☑️ **SDK Data Files** ← MUY IMPORTANTE
   - ☑️ Examples
   - ☑️ Documentation

### Solución 4: Verificar USB Original

El USB que vino con la cámara debería contener:
```
USB/
├── SDK/
│   └── engine/
│       └── Dhyana 2100.dat   ← Buscar aquí
├── Software/
│   └── TCapture/
│       └── engine/
│           └── Dhyana 2100.dat   ← O aquí
└── Drivers/
```

## 🔧 Una Vez que Tengas el Archivo

1. **Copiar a la ubicación correcta:**
   ```powershell
   # Asegurarse de que el directorio existe
   mkdir "C:\Users\Optica\Desktop\claude\claude\build\Release\engine" -Force

   # Copiar el archivo
   Copy-Item "RUTA_DONDE_ENCONTRASTE\Dhyana 2100.dat" `
             "C:\Users\Optica\Desktop\claude\claude\build\Release\engine\"

   # Verificar
   dir "C:\Users\Optica\Desktop\claude\claude\build\Release\engine"
   ```

2. **Actualizar CMakeLists.txt para copiarlo automáticamente:**

   El archivo ya está configurado para copiar automáticamente desde:
   ```
   C:/Program Files/TUCam_SDK/sdk/lib/All_x64/engine
   ```

   Si tu archivo está en otra ubicación, actualizar la ruta en `CMakeLists.txt`.

3. **Ejecutar el programa de nuevo:**
   ```powershell
   cd C:\Users\Optica\Desktop\claude\claude\build\Release
   .\dhyana_control.exe
   ```

   Deberías ver:
   ```
   [TUCSEN]: Inference device - CPU.
   Camera opened successfully.
   Initializing camera...
     Allocating buffer...
     Buffer allocated successfully.
     Setting trigger mode to SEQUENCE...
     Trigger mode set successfully.
     Testing camera communication...
     Camera model detected: Dhyana 2100    ← ¡ÉXITO!
   Camera initialization complete.
   ```

## ⚠️ IMPORTANTE: NO Usar Archivos de Otras Cámaras

**NO intentes usar:**
- ❌ Libra 3412C.dat
- ❌ Dhyana 400D.dat
- ❌ Cualquier otro archivo .dat de modelos diferentes

Cada modelo tiene un archivo específico. Usar el archivo incorrecto causará:
- Detección incorrecta de la cámara
- Parámetros fuera de rango
- Mal funcionamiento o daño potencial

## 📊 Estado Actual

| Componente | Estado | Notas |
|-----------|--------|-------|
| SDK Instalado | ✅ | Funciona correctamente |
| Camera Open | ✅ | La cámara se detecta y abre |
| Buffer Allocation | ✅ | Se asigna correctamente |
| Trigger Mode | ✅ | Se configura correctamente |
| **Dhyana 2100.dat** | ❌ | **FALTA - CRÍTICO** |
| Camera Properties | ❌ | Fallan sin el archivo .dat |

## 🎯 Próximos Pasos

1. ⬜ Ejecutar `find_dhyana_dat.ps1` para buscar el archivo
2. ⬜ Si no se encuentra, instalar software oficial de Tucsen
3. ⬜ Si aún no se encuentra, contactar a Tucsen
4. ⬜ Copiar `Dhyana 2100.dat` a `build/Release/engine/`
5. ⬜ Probar el programa de nuevo
6. ⬜ Verificar que `info` y `caps` funcionen correctamente

## 📝 Notas Adicionales

- El archivo .dat es específico de fábrica y puede contener calibración única de tu cámara
- Tucsen debería proporcionar este archivo sin problemas
- Es posible que necesites el número de serie de tu cámara al contactar a Tucsen
- El archivo suele ser de pocos KB de tamaño

---

**¿Necesitas ayuda?**
- Contacta a Tucsen: service@tucsen.com
- Revisa la documentación oficial en www.tucsen.com
