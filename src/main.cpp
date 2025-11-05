#include "DhyanaCamera.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <cstdlib>

using namespace Dhyana;

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

    // Si no hay argumentos, mostrar ayuda
    if (argc == 1) {
        ShowHelp(argv[0]);
        return 0;
    }

    // Parsear argumentos
    if (!ParseArguments(argc, argv, config, cameraIndex, showInfo)) {
        return 0; // Salir si se mostró ayuda
    }

    // Crear objeto de cámara
    Camera camera;

    // Inicializar SDK
    std::cout << "\nInitializing camera SDK..." << std::endl;
    ErrorCode err = camera.Initialize();
    if (err != ErrorCode::Success) {
        std::cerr << "Error: Failed to initialize SDK: "
                  << ErrorCodeToString(err) << std::endl;
        return 1;
    }

    // Verificar si hay cámaras disponibles
    if (camera.GetCameraCount() == 0) {
        std::cerr << "Error: No cameras found!" << std::endl;
        std::cerr << "Please check:" << std::endl;
        std::cerr << "  1. Camera is connected" << std::endl;
        std::cerr << "  2. Camera drivers are installed" << std::endl;
        std::cerr << "  3. Camera has power" << std::endl;
        return 1;
    }

    // Abrir cámara
    std::cout << "Opening camera " << cameraIndex << "..." << std::endl;
    err = camera.Open(cameraIndex);
    if (err != ErrorCode::Success) {
        std::cerr << "Error: Failed to open camera: "
                  << ErrorCodeToString(err) << std::endl;
        return 1;
    }

    // Mostrar información si se solicitó
    if (showInfo) {
        ShowCameraInfo(camera);
        if (argc == 2 || argc == 3) {
            // Solo se pidió info, salir
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
        std::cerr << "\nError: Capture failed: "
                  << ErrorCodeToString(err) << std::endl;
        return 1;
    }

    std::cout << "\nCapture completed successfully!" << std::endl;

    return 0;
}
