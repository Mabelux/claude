# Dhyana Camera Control

Control de cámara Dhyana 2100 mediante línea de comandos (CLI) usando el SDK de Tucsen Photonics.

## Características

- ✅ Control de tiempo de exposición
- ✅ Control de ganancia
- ✅ Configuración de binning (sum/average)
- ✅ Región de interés (ROI)
- ✅ Captura de múltiples frames
- ✅ Control de buffer de frames
- ✅ Tiempo total de adquisición
- ✅ Guardado en múltiples formatos (TIFF, PNG, BMP, JPG)
- ✅ Interfaz de línea de comandos intuitiva

## Requisitos del Sistema

### Windows 11
- Visual Studio 2019 o superior (con herramientas C++)
- CMake 3.15 o superior
- SDK de Tucsen (incluido en el repositorio)

### Controladores
- Drivers de la cámara Dhyana instalados
- La cámara debe estar conectada y reconocida por Windows

## Estructura del Proyecto

```
dhyana-camera-control/
├── CMakeLists.txt              # Configuración de CMake
├── README.md                   # Este archivo
├── .gitignore                  # Archivos ignorados por Git
├── sdk/                        # SDK de Tucsen
│   ├── inc/                    # Headers del SDK
│   │   ├── TUCamApi.h
│   │   └── TUDefine.h
│   └── lib/                    # Librerías y DLLs
│       ├── TUCam.lib
│       ├── TUCam.dll
│       └── ...
├── include/                    # Headers del proyecto
│   ├── DhyanaCamera.h          # Clase wrapper de la cámara
│   └── DhyanaTypes.h           # Tipos y estructuras
├── src/                        # Código fuente
│   ├── DhyanaCamera.cpp        # Implementación del wrapper
│   └── main.cpp                # CLI principal
└── output/                     # Directorio de salida (generado)
    └── frame_*.tif             # Imágenes capturadas
```

## Compilación

### Opción 1: Visual Studio (Recomendado para Windows)

1. **Abrir el proyecto en Visual Studio:**
   ```cmd
   cd dhyana-camera-control
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

2. **Abrir la solución generada:**
   - Abrir `build/DhyanaCameraControl.sln` en Visual Studio
   - Seleccionar configuración `Release` o `Debug`
   - Compilar (F7 o Build -> Build Solution)

3. **Ejecutable generado:**
   - `build/Release/dhyana_control.exe`

### Opción 2: Línea de comandos con CMake

1. **Generar archivos de compilación:**
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

2. **Compilar:**
   ```cmd
   cmake --build . --config Release
   ```

3. **Ejecutable:**
   - `build/Release/dhyana_control.exe`

### Opción 3: Visual Studio Code

1. **Instalar extensiones:**
   - C/C++ (Microsoft)
   - CMake Tools

2. **Abrir la carpeta del proyecto en VS Code**

3. **Configurar CMake:**
   - Ctrl+Shift+P -> "CMake: Configure"
   - Seleccionar compilador (Visual Studio)

4. **Compilar:**
   - Ctrl+Shift+P -> "CMake: Build"
   - O presionar F7

## Uso

### Ayuda

```cmd
dhyana_control.exe --help
```

### Información de la Cámara

```cmd
dhyana_control.exe --info
```

### Ejemplos de Uso

#### 1. Captura simple con exposición de 50ms

```cmd
dhyana_control.exe -e 50
```

#### 2. Capturar 10 frames con binning 2x2

```cmd
dhyana_control.exe -n 10 --binning 2 2
```

#### 3. Captura con exposición, ganancia y ROI personalizados

```cmd
dhyana_control.exe -e 100 -g 2.5 -n 5 --roi 100 100 512 512
```

#### 4. Captura durante 60 segundos

```cmd
dhyana_control.exe -e 100 -t 60 -n 1000
```

#### 5. Captura con todas las opciones

```cmd
dhyana_control.exe ^
    -e 150 ^
    -g 3.0 ^
    -n 20 ^
    -b 15 ^
    --binning 2 2 ^
    --bin-mode avg ^
    --roi 200 200 1024 1024 ^
    -o ./my_captures ^
    -f PNG
```

## Opciones de Línea de Comandos

### Opciones Generales

| Opción | Descripción | Por Defecto |
|--------|-------------|-------------|
| `-h, --help` | Muestra ayuda | - |
| `-i, --info` | Muestra información de la cámara | - |
| `-c, --camera INDEX` | Índice de cámara a usar | 0 |

### Configuración de Captura

| Opción | Descripción | Por Defecto |
|--------|-------------|-------------|
| `-e, --exposure TIME` | Tiempo de exposición (ms) | 100 |
| `-g, --gain GAIN` | Ganancia | 1.0 |
| `-n, --frames NUM` | Número de frames a capturar | 1 |
| `-b, --buffer NUM` | Número de frames en buffer | 10 |
| `-t, --time SECONDS` | Tiempo total de adquisición (s) | 0 (sin límite) |

### Configuración de Binning

| Opción | Descripción | Por Defecto |
|--------|-------------|-------------|
| `--binning X Y` | Binning horizontal y vertical | Deshabilitado |
| `--bin-mode MODE` | Modo: `avg` (promedio) o `sum` (suma) | avg |

### Configuración de ROI

| Opción | Descripción | Por Defecto |
|--------|-------------|-------------|
| `--roi X Y W H` | ROI: offset X, Y, ancho, alto | Sensor completo |

### Configuración de Salida

| Opción | Descripción | Por Defecto |
|--------|-------------|-------------|
| `-o, --output PATH` | Directorio de salida | ./output |
| `-f, --format FORMAT` | Formato: TIFF, PNG, BMP, JPG | TIFF |

## Formatos de Salida Soportados

- **TIFF** (.tif): Recomendado para imágenes científicas (sin pérdida)
- **PNG** (.png): Sin pérdida, compresión
- **BMP** (.bmp): Sin compresión
- **JPG** (.jpg): Con pérdida, menor tamaño de archivo

## Notas de Uso

### Binning
- **Sum mode**: Suma los píxeles (mayor señal, misma relación S/N)
- **Average mode**: Promedia los píxeles (mantiene rango dinámico)
- Valores comunes: 1x1 (sin binning), 2x2, 4x4

### ROI (Region of Interest)
- Permite capturar solo una región del sensor
- Reduce el tamaño de archivo y aumenta la velocidad
- Coordenadas deben estar dentro del sensor
- Considerar binning al calcular ROI

### Buffer de Frames
- Buffer más grande = menor pérdida de frames
- Consume más memoria RAM
- Recomendado: 10-50 frames

### Tiempo de Adquisición
- Si `-t` es 0: captura exactamente `-n` frames
- Si `-t` > 0: captura hasta alcanzar el tiempo o `-n` frames (lo que ocurra primero)

## Solución de Problemas

### "No cameras found"
1. Verificar que la cámara está conectada (USB/Cable)
2. Verificar que los drivers están instalados
3. Verificar que la cámara tiene alimentación
4. Comprobar en el Administrador de Dispositivos de Windows

### "Failed to initialize SDK"
1. Verificar que las DLLs del SDK están en el mismo directorio que el .exe
2. Verificar que no hay otra aplicación usando la cámara
3. Reiniciar la cámara (desconectar/conectar)

### "Failed to allocate buffer"
1. Reducir el número de buffers
2. Reducir resolución (usar ROI o binning)
3. Cerrar otras aplicaciones que usan memoria

### Frames perdidos
1. Aumentar buffer (`-b` mayor)
2. Reducir velocidad de captura (mayor exposición)
3. Usar binning para reducir datos
4. Guardar en disco más rápido (SSD)

## Documentación Adicional

- [Manual de Usuario Dhyana 2100](Dhyana%202100%20User%20Manual_20250813_V1.0.1.pdf)
- [SDK de Tucsen](sdk/)
- Código fuente documentado en headers (`include/`)

## Desarrollo

### Agregar Nuevas Funcionalidades

1. **Modificar la clase `Dhyana::Camera`** en `include/DhyanaCamera.h`
2. **Implementar en** `src/DhyanaCamera.cpp`
3. **Actualizar CLI** en `src/main.cpp`
4. **Recompilar** con CMake

### Estructura de Clases

```cpp
namespace Dhyana {
    class Camera {
        // Métodos públicos para control de cámara
        ErrorCode Initialize();
        ErrorCode Open(uint32_t cameraIndex);
        ErrorCode SetExposure(double exposureMs);
        ErrorCode SetGain(double gain);
        ErrorCode SetBinning(int32_t x, int32_t y, bool avgMode);
        ErrorCode CaptureFrames(const CaptureConfig& config);
        // ...
    };

    struct CaptureConfig {
        // Configuración de captura
    };

    struct CameraInfo {
        // Información de la cámara
    };
}
```

## Licencia

Este proyecto utiliza el SDK de Tucsen Photonics. Ver documentación del SDK para términos de licencia.

## Autor

Proyecto creado para el control de cámaras Dhyana 2100.

## Versión

**v1.0.0** - Primera versión estable
- Control completo de exposición, ganancia, binning, ROI
- Soporte para múltiples formatos de imagen
- Interfaz CLI completa

## Contacto y Soporte

Para problemas relacionados con:
- **Hardware de cámara**: Contactar a Tucsen Photonics
- **Software/SDK**: Consultar manual del SDK
- **Este proyecto**: Abrir un issue en el repositorio
