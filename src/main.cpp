#include "DhyanaCamera.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Dhyana;

/**
 * @brief Pausa antes de salir (útil en Windows)
 */
void PauseBeforeExit() {
#ifdef _WIN32
    std::cout << "\nPresiona cualquier tecla para salir..." << std::endl;
    system("pause >nul");
#else
    std::cout << "\nPresiona Enter para salir..." << std::endl;
    std::cin.get();
#endif
}

/**
 * @brief Muestra comandos rápidos
 */
void ShowQuickCommands(const char* programName) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Comandos Rapidos - Dhyana Camera Control" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "\nComandos disponibles:" << std::endl;
    std::cout << "  " << programName << " -h                  # Ayuda completa" << std::endl;
    std::cout << "  " << programName << " -i                  # Informacion de camara" << std::endl;
    std::cout << "  " << programName << " -e 50               # Captura con exposicion 50ms" << std::endl;
    std::cout << "  " << programName << " -e 100 -n 10        # Captura 10 frames" << std::endl;
    std::cout << "  " << programName << " -e 100 -g 2.0       # Con ganancia 2.0" << std::endl;
    std::cout << "  " << programName << " --binning 2 2       # Con binning 2x2" << std::endl;
    std::cout << "\nParametros principales:" << std::endl;
    std::cout << "  -e  Exposicion (ms)     -g  Ganancia         -n  Numero de frames" << std::endl;
    std::cout << "  -b  Buffer frames       -o  Directorio       -f  Formato (TIFF/PNG)" << std::endl;
    std::cout << "  -i  Info de camara      -h  Ayuda completa" << std::endl;
    std::cout << "\nEjemplos de uso rapido:" << std::endl;
    std::cout << "  " << programName << " -e 50 -n 5 -o ./capturas" << std::endl;
    std::cout << "  " << programName << " -e 100 --binning 2 2 -n 10" << std::endl;
    std::cout << "\nPara ver todas las opciones, usa: " << programName << " -h" << std::endl;
    std::cout << "==================================================" << std::endl;
}

/**
 * @brief Muestra la ayuda del programa
 */
void ShowHelp(const char* programName) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Dhyana Camera Control - Command Line Interface" << std::endl;
    std::cout << "  Version 1.0.0" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "\nUsage: " << programName << " [options]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -h, --help              Show this help message" << std::endl;
    std::cout << "  -i, --info              Show camera information" << std::endl;
    std::cout << "  -c, --camera INDEX      Camera index (default: 0)" << std::endl;
    std::cout << "\nCapture Settings:" << std::endl;
    std::cout << "  -e, --exposure TIME     Exposure time in milliseconds (default: 100)" << std::endl;
    std::cout << "  -g, --gain GAIN         Gain value (default: 1.0)" << std::endl;
    std::cout << "  -n, --frames NUM        Number of frames to capture (default: 1)" << std::endl;
    std::cout << "  -b, --buffer NUM        Number of buffer frames (default: 10)" << std::endl;
    std::cout << "  -t, --time SECONDS      Total acquisition time in seconds (0 = no limit)" << std::endl;
    std::cout << "\nBinning Settings:" << std::endl;
    std::cout << "  --binning X Y           Enable binning with X,Y factors (e.g., --binning 2 2)" << std::endl;
    std::cout << "  --bin-mode MODE         Binning mode: avg (average) or sum (default: avg)" << std::endl;
    std::cout << "\nROI Settings:" << std::endl;
    std::cout << "  --roi X Y W H           Set ROI: X offset, Y offset, Width, Height" << std::endl;
    std::cout << "\nOutput Settings:" << std::endl;
    std::cout << "  -o, --output PATH       Output directory path (default: ./output)" << std::endl;
    std::cout << "  -f, --format FORMAT     Image format: TIFF, PNG, BMP, JPG (default: TIFF)" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  # Capture single frame with 50ms exposure" << std::endl;
    std::cout << "  " << programName << " -e 50" << std::endl;
    std::cout << "\n  # Capture 10 frames with binning 2x2" << std::endl;
    std::cout << "  " << programName << " -n 10 --binning 2 2" << std::endl;
    std::cout << "\n  # Capture with custom exposure, gain and ROI" << std::endl;
    std::cout << "  " << programName << " -e 100 -g 2.5 -n 5 --roi 100 100 512 512" << std::endl;
    std::cout << "\n  # Capture for 60 seconds" << std::endl;
    std::cout << "  " << programName << " -e 100 -t 60 -n 1000" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Muestra información de la cámara
 */
void ShowCameraInfo(Camera& camera) {
    CameraInfo info;
    ErrorCode err = camera.GetCameraInfo(info);
    if (err != ErrorCode::Success) {
        std::cerr << "Error: Failed to get camera info: "
                  << ErrorCodeToString(err) << std::endl;
        return;
    }

    double currentExposure = 0.0;
    double currentGain = 0.0;
    camera.GetExposure(currentExposure);
    camera.GetGain(currentGain);

    std::cout << "\n==================================================" << std::endl;
    std::cout << "            Camera Information" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Model:            " << info.model << std::endl;
    std::cout << "Serial Number:    " << info.serialNumber << std::endl;
    std::cout << "Firmware Version: " << info.firmwareVersion << std::endl;
    std::cout << "Sensor Size:      " << info.sensorWidth << " x " << info.sensorHeight << " pixels" << std::endl;
    std::cout << "Bit Depth:        " << info.bitDepth << " bits" << std::endl;
    std::cout << "Pixel Size:       " << std::fixed << std::setprecision(2)
              << info.pixelSize << " µm" << std::endl;
    std::cout << "\nCurrent Settings:" << std::endl;
    std::cout << "Exposure:         " << std::fixed << std::setprecision(3)
              << currentExposure << " ms" << std::endl;
    std::cout << "Gain:             " << std::fixed << std::setprecision(2)
              << currentGain << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Parsea argumentos de línea de comandos
 */
bool ParseArguments(int argc, char* argv[], CaptureConfig& config, int& cameraIndex, bool& showInfo) {
    std::map<std::string, std::string> args;

    // Convertir argumentos a mapa
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            ShowHelp(argv[0]);
            return false;
        } else if (arg == "-i" || arg == "--info") {
            showInfo = true;
        } else if (arg == "-c" || arg == "--camera") {
            if (i + 1 < argc) {
                cameraIndex = std::atoi(argv[++i]);
            }
        } else if (arg == "-e" || arg == "--exposure") {
            if (i + 1 < argc) {
                config.exposureTime = std::atof(argv[++i]);
            }
        } else if (arg == "-g" || arg == "--gain") {
            if (i + 1 < argc) {
                config.gain = std::atof(argv[++i]);
            }
        } else if (arg == "-n" || arg == "--frames") {
            if (i + 1 < argc) {
                config.numFrames = std::atoi(argv[++i]);
            }
        } else if (arg == "-b" || arg == "--buffer") {
            if (i + 1 < argc) {
                config.bufferFrames = std::atoi(argv[++i]);
            }
        } else if (arg == "-t" || arg == "--time") {
            if (i + 1 < argc) {
                config.totalTime = std::atof(argv[++i]);
            }
        } else if (arg == "--binning") {
            if (i + 2 < argc) {
                config.enableBinning = true;
                config.binningX = std::atoi(argv[++i]);
                config.binningY = std::atoi(argv[++i]);
            }
        } else if (arg == "--bin-mode") {
            if (i + 1 < argc) {
                std::string mode = argv[++i];
                config.binningMode = (mode == "avg" || mode == "average");
            }
        } else if (arg == "--roi") {
            if (i + 4 < argc) {
                config.enableROI = true;
                config.roiX = std::atoi(argv[++i]);
                config.roiY = std::atoi(argv[++i]);
                config.roiWidth = std::atoi(argv[++i]);
                config.roiHeight = std::atoi(argv[++i]);
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                config.outputPath = argv[++i];
            }
        } else if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) {
                config.outputFormat = argv[++i];
            }
        }
    }

    return true;
}

/**
 * @brief Función principal
 */
int main(int argc, char* argv[]) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Dhyana Camera Control v1.0.0" << std::endl;
    std::cout << "==================================================" << std::endl;

    // Configuración por defecto
    CaptureConfig config;
    int cameraIndex = 0;
    bool showInfo = false;

    // Si no hay argumentos, mostrar comandos rápidos
    if (argc == 1) {
        ShowQuickCommands(argv[0]);
        PauseBeforeExit();
        return 0;
    }

    // Parsear argumentos
    if (!ParseArguments(argc, argv, config, cameraIndex, showInfo)) {
        PauseBeforeExit();
        return 0; // Salir si se mostró ayuda
    }

    // Crear objeto de cámara
    Camera camera;

    // Inicializar SDK
    std::cout << "\nInitializing camera SDK..." << std::endl;
    ErrorCode err = camera.Initialize();
    if (err != ErrorCode::Success) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: Failed to initialize SDK" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nError code: " << ErrorCodeToString(err) << std::endl;
        std::cerr << "\nPosibles causas:" << std::endl;
        std::cerr << "  1. Las DLLs del SDK no estan en el directorio del .exe" << std::endl;
        std::cerr << "  2. Falta alguna DLL (copiar todas desde sdk/lib/)" << std::endl;
        std::cerr << "  3. El SDK no esta instalado correctamente" << std::endl;
        std::cerr << "\nSolucion: Ejecuta copy_dlls.bat para copiar las DLLs" << std::endl;
        PauseBeforeExit();
        return 1;
    }

    // Verificar si hay cámaras disponibles
    if (camera.GetCameraCount() == 0) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: No se encontro ninguna camara conectada" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nVerifica lo siguiente:" << std::endl;
        std::cerr << "  1. La camara Dhyana esta conectada al USB/Puerto" << std::endl;
        std::cerr << "  2. Los drivers de la camara estan instalados" << std::endl;
        std::cerr << "  3. La camara tiene alimentacion electrica" << std::endl;
        std::cerr << "  4. El cable USB/conexion esta funcionando" << std::endl;
        std::cerr << "  5. La camara aparece en el Administrador de Dispositivos" << std::endl;
        std::cerr << "\nNOTA: El SDK se inicializo correctamente, pero no detecta camaras." << std::endl;
        std::cerr << "      Esto significa que el software funciona, pero la camara" << std::endl;
        std::cerr << "      no esta conectada o no tiene los drivers instalados." << std::endl;
        PauseBeforeExit();
        return 1;
    }

    std::cout << "Camara(s) detectada(s): " << camera.GetCameraCount() << std::endl;

    // Abrir cámara
    std::cout << "Opening camera " << cameraIndex << "..." << std::endl;
    err = camera.Open(cameraIndex);
    if (err != ErrorCode::Success) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: No se pudo abrir la camara" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nError code: " << ErrorCodeToString(err) << std::endl;
        std::cerr << "\nPosibles causas:" << std::endl;
        std::cerr << "  1. La camara esta siendo usada por otra aplicacion" << std::endl;
        std::cerr << "  2. No tienes permisos para acceder a la camara" << std::endl;
        std::cerr << "  3. El indice de camara es incorrecto" << std::endl;
        std::cerr << "\nSolucion:" << std::endl;
        std::cerr << "  - Cierra otras aplicaciones que usen la camara" << std::endl;
        std::cerr << "  - Desconecta y reconecta la camara" << std::endl;
        std::cerr << "  - Ejecuta como Administrador" << std::endl;
        PauseBeforeExit();
        return 1;
    }

    // Mostrar información si se solicitó
    if (showInfo) {
        ShowCameraInfo(camera);
        if (argc == 2 || argc == 3) {
            // Solo se pidió info, salir
            PauseBeforeExit();
            return 0;
        }
    }

    // Realizar captura
    std::cout << "\nStarting capture with the following settings:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Exposure:      " << config.exposureTime << " ms" << std::endl;
    std::cout << "Gain:          " << config.gain << std::endl;
    std::cout << "Frames:        " << config.numFrames << std::endl;
    std::cout << "Buffer frames: " << config.bufferFrames << std::endl;
    if (config.totalTime > 0) {
        std::cout << "Total time:    " << config.totalTime << " s" << std::endl;
    }
    if (config.enableBinning) {
        std::cout << "Binning:       " << config.binningX << "x" << config.binningY
                  << " (" << (config.binningMode ? "average" : "sum") << ")" << std::endl;
    }
    if (config.enableROI) {
        std::cout << "ROI:           x=" << config.roiX << ", y=" << config.roiY
                  << ", w=" << config.roiWidth << ", h=" << config.roiHeight << std::endl;
    }
    std::cout << "Output:        " << config.outputPath << std::endl;
    std::cout << "Format:        " << config.outputFormat << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // Capturar frames
    CaptureStats stats;
    err = camera.CaptureFrames(config, &stats);
    if (err != ErrorCode::Success) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: La captura fallo" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nError code: " << ErrorCodeToString(err) << std::endl;
        PauseBeforeExit();
        return 1;
    }

    std::cout << "\n==================================================" << std::endl;
    std::cout << "  CAPTURA COMPLETADA EXITOSAMENTE!" << std::endl;
    std::cout << "==================================================" << std::endl;

    PauseBeforeExit();
    return 0;
}
