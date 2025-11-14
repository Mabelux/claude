#include "DhyanaCamera.h"
#include "TUCamApi.h"
#include "TUDefine.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Dhyana;
namespace fs = std::filesystem;

// Global logger
std::ofstream g_logFile;
std::string g_outputDir = "./output";

/**
 * @brief Inicializa el sistema de logging
 */
void InitializeLogger() {
    // Crear directorio de salida si no existe
    if (!fs::exists(g_outputDir)) {
        fs::create_directories(g_outputDir);
    }

    // Abrir archivo de log
    std::string logPath = g_outputDir + "/session.log";
    g_logFile.open(logPath, std::ios::app);

    if (g_logFile.is_open()) {
        // Obtener timestamp
        std::time_t now = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        g_logFile << "\n========================================\n";
        g_logFile << "Session started: " << timestamp << "\n";
        g_logFile << "========================================\n";
        g_logFile.flush();
    }
}

/**
 * @brief Escribe mensaje al log
 */
void Log(const std::string& message) {
    if (g_logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%H:%M:%S", std::localtime(&now));

        g_logFile << "[" << timestamp << "] " << message << std::endl;
        g_logFile.flush();
    }
}

/**
 * @brief Cierra el logger
 */
void CloseLogger() {
    if (g_logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        g_logFile << "Session ended: " << timestamp << "\n";
        g_logFile << "========================================\n\n";
        g_logFile.close();
    }
}

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
 * @brief Extrae el nombre del archivo de la ruta completa
 */
std::string GetProgramName(const char* fullPath) {
    std::string path(fullPath);
    size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

/**
 * @brief Divide una cadena por espacios (respetando comillas)
 */
std::vector<std::string> SplitCommand(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;

    for (char c : input) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

/**
 * @brief Muestra capacidades de la cámara
 */
void ShowCameraCapabilities(Camera& camera) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Capacidades de la Camara" << std::endl;
    std::cout << "==================================================" << std::endl;

    // Esta función usa directamente la API del SDK para mostrar capacidades
    HDTUCAM handle = camera.GetHandle();
    if (handle == nullptr) {
        std::cerr << "Error: Handle de camara no valido" << std::endl;
        return;
    }

    // Capacidad de exposición
    TUCAM_CAPA_ATTR capaAttr;
    memset(&capaAttr, 0, sizeof(TUCAM_CAPA_ATTR));
    capaAttr.idCapa = TUIDC_EXPOSURETM;

    TUCAMRET ret = TUCAM_Capa_GetAttr(handle, &capaAttr);
    if (TUCAMRET_SUCCESS == ret) {
        std::cout << "\nExposicion:" << std::endl;
        std::cout << "  Rango: " << capaAttr.dbValMin << " - "
                  << capaAttr.dbValMax << " ms" << std::endl;
        std::cout << "  Paso: " << capaAttr.dbValStep << " ms" << std::endl;
        std::cout << "  Por defecto: " << capaAttr.dbValDft << " ms" << std::endl;
    } else {
        std::cout << "\nExposicion: NO SOPORTADA (Error: 0x"
                  << std::hex << ret << std::dec << ")" << std::endl;
    }

    // Capacidad de ganancia
    memset(&capaAttr, 0, sizeof(TUCAM_CAPA_ATTR));
    capaAttr.idCapa = TUIDC_GAIN;
    ret = TUCAM_Capa_GetAttr(handle, &capaAttr);
    if (TUCAMRET_SUCCESS == ret) {
        std::cout << "\nGanancia:" << std::endl;
        std::cout << "  Rango: " << capaAttr.dbValMin << " - "
                  << capaAttr.dbValMax << std::endl;
        std::cout << "  Paso: " << capaAttr.dbValStep << std::endl;
        std::cout << "  Por defecto: " << capaAttr.dbValDft << std::endl;
    } else {
        std::cout << "\nGanancia: NO SOPORTADA (Error: 0x"
                  << std::hex << ret << std::dec << ")" << std::endl;
    }

    // Resolución
    memset(&capaAttr, 0, sizeof(TUCAM_CAPA_ATTR));
    capaAttr.idCapa = TUIDC_RESOLUTION;
    ret = TUCAM_Capa_GetAttr(handle, &capaAttr);
    if (TUCAMRET_SUCCESS == ret) {
        int width = (capaAttr.nValMax >> 16) & 0xFFFF;
        int height = capaAttr.nValMax & 0xFFFF;
        std::cout << "\nResolucion:" << std::endl;
        std::cout << "  Maxima: " << width << " x " << height << " pixels" << std::endl;
    }

    std::cout << "==================================================" << std::endl;
    Log("Camera capabilities displayed");
}

/**
 * @brief Muestra ayuda del modo interactivo
 */
void ShowInteractiveHelp() {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Comandos Disponibles" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "\nControl de Camara:" << std::endl;
    std::cout << "  info                      - Muestra informacion de la camara" << std::endl;
    std::cout << "  show                      - Muestra configuracion actual" << std::endl;
    std::cout << "  caps                      - Muestra capacidades de la camara" << std::endl;
    std::cout << "\nConfiguracion:" << std::endl;
    std::cout << "  exposure <ms>             - Configura tiempo de exposicion" << std::endl;
    std::cout << "  gain <value>              - Configura ganancia" << std::endl;
    std::cout << "  binning <x> <y> [mode]    - Configura binning (mode: avg/sum)" << std::endl;
    std::cout << "  roi <x> <y> <w> <h>       - Configura region de interes" << std::endl;
    std::cout << "  roi reset                 - Resetea ROI a sensor completo" << std::endl;
    std::cout << "  format <fmt>              - Formato de salida (TIFF/PNG/BMP/JPG)" << std::endl;
    std::cout << "  output <path>             - Directorio de salida" << std::endl;
    std::cout << "\nCaptura:" << std::endl;
    std::cout << "  capture [n]               - Captura n frames (default: 1)" << std::endl;
    std::cout << "\nSistema:" << std::endl;
    std::cout << "  help, ?                   - Muestra esta ayuda" << std::endl;
    std::cout << "  quit, exit                - Sale del programa" << std::endl;
    std::cout << "\nEjemplos:" << std::endl;
    std::cout << "  > caps" << std::endl;
    std::cout << "  > exposure 50" << std::endl;
    std::cout << "  > binning 2 2 avg" << std::endl;
    std::cout << "  > capture 10" << std::endl;
    std::cout << "  > roi 100 100 512 512" << std::endl;
    std::cout << "==================================================" << std::endl;
}

/**
 * @brief Muestra configuración actual
 */
void ShowCurrentConfig(const CaptureConfig& config) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Configuracion Actual" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Exposicion:    " << config.exposureTime << " ms" << std::endl;
    std::cout << "Ganancia:      " << config.gain << std::endl;
    std::cout << "Buffer:        " << config.bufferFrames << " frames" << std::endl;

    if (config.enableBinning) {
        std::cout << "Binning:       " << config.binningX << "x" << config.binningY
                  << " (" << (config.binningMode ? "promedio" : "suma") << ")" << std::endl;
    } else {
        std::cout << "Binning:       Deshabilitado" << std::endl;
    }

    if (config.enableROI) {
        std::cout << "ROI:           x=" << config.roiX << ", y=" << config.roiY
                  << ", w=" << config.roiWidth << ", h=" << config.roiHeight << std::endl;
    } else {
        std::cout << "ROI:           Sensor completo" << std::endl;
    }

    std::cout << "Formato:       " << config.outputFormat << std::endl;
    std::cout << "Directorio:    " << config.outputPath << std::endl;
    std::cout << "==================================================" << std::endl;
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
        Log("ERROR: Failed to get camera info");
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

    Log("Camera info displayed");
}

/**
 * @brief Modo interactivo principal
 */
void RunInteractiveMode(Camera& camera) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Dhyana Camera Control v1.0.0" << std::endl;
    std::cout << "  Modo Interactivo" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "\nCamara conectada exitosamente." << std::endl;
    std::cout << "Escribe 'help' para ver comandos disponibles." << std::endl;
    std::cout << "Escribe 'quit' para salir." << std::endl;

    Log("Interactive mode started");

    // Configuración por defecto
    CaptureConfig config;

    // Loop interactivo
    std::string input;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, input);

        // Eliminar espacios al inicio y final
        input.erase(0, input.find_first_not_of(" \t"));
        input.erase(input.find_last_not_of(" \t") + 1);

        if (input.empty()) {
            continue;
        }

        // Log del comando
        Log("Command: " + input);

        // Dividir comando en tokens
        std::vector<std::string> tokens = SplitCommand(input);
        if (tokens.empty()) {
            continue;
        }

        std::string cmd = tokens[0];

        // Procesar comandos
        if (cmd == "help" || cmd == "?") {
            ShowInteractiveHelp();

        } else if (cmd == "quit" || cmd == "exit") {
            std::cout << "Cerrando camara..." << std::endl;
            Log("Interactive mode ended by user");
            break;

        } else if (cmd == "info") {
            ShowCameraInfo(camera);

        } else if (cmd == "show") {
            ShowCurrentConfig(config);

        } else if (cmd == "caps" || cmd == "capabilities") {
            ShowCameraCapabilities(camera);

        } else if (cmd == "exposure") {
            if (tokens.size() < 2) {
                std::cerr << "Error: Se requiere valor de exposicion (ms)" << std::endl;
                std::cerr << "Uso: exposure <ms>" << std::endl;
                Log("ERROR: exposure command missing value");
            } else {
                double exposure = std::atof(tokens[1].c_str());
                config.exposureTime = exposure;
                ErrorCode err = camera.SetExposure(exposure);
                if (err == ErrorCode::Success) {
                    std::cout << "Exposicion configurada: " << exposure << " ms" << std::endl;
                    Log("Exposure set to " + std::to_string(exposure) + " ms");
                } else {
                    std::cerr << "Error al configurar exposicion: " << ErrorCodeToString(err) << std::endl;
                    Log("ERROR: Failed to set exposure");
                }
            }

        } else if (cmd == "gain") {
            if (tokens.size() < 2) {
                std::cerr << "Error: Se requiere valor de ganancia" << std::endl;
                std::cerr << "Uso: gain <value>" << std::endl;
                Log("ERROR: gain command missing value");
            } else {
                double gain = std::atof(tokens[1].c_str());
                config.gain = gain;
                ErrorCode err = camera.SetGain(gain);
                if (err == ErrorCode::Success) {
                    std::cout << "Ganancia configurada: " << gain << std::endl;
                    Log("Gain set to " + std::to_string(gain));
                } else {
                    std::cerr << "Error al configurar ganancia: " << ErrorCodeToString(err) << std::endl;
                    Log("ERROR: Failed to set gain");
                }
            }

        } else if (cmd == "binning") {
            if (tokens.size() < 3) {
                std::cerr << "Error: Se requieren valores X e Y para binning" << std::endl;
                std::cerr << "Uso: binning <x> <y> [avg|sum]" << std::endl;
                Log("ERROR: binning command missing values");
            } else {
                int x = std::atoi(tokens[1].c_str());
                int y = std::atoi(tokens[2].c_str());
                bool avgMode = true;

                if (tokens.size() >= 4) {
                    std::string mode = tokens[3];
                    avgMode = (mode == "avg" || mode == "average");
                }

                config.enableBinning = true;
                config.binningX = x;
                config.binningY = y;
                config.binningMode = avgMode;

                ErrorCode err = camera.SetBinning(x, y, avgMode);
                if (err == ErrorCode::Success) {
                    std::cout << "Binning configurado: " << x << "x" << y
                              << " (modo " << (avgMode ? "promedio" : "suma") << ")" << std::endl;
                    Log("Binning set to " + std::to_string(x) + "x" + std::to_string(y) +
                        " (" + (avgMode ? "avg" : "sum") + ")");
                } else {
                    std::cerr << "Error al configurar binning: " << ErrorCodeToString(err) << std::endl;
                    Log("ERROR: Failed to set binning");
                }
            }

        } else if (cmd == "roi") {
            if (tokens.size() < 2) {
                std::cerr << "Error: Se requieren parametros para ROI" << std::endl;
                std::cerr << "Uso: roi <x> <y> <w> <h>  o  roi reset" << std::endl;
                Log("ERROR: roi command missing parameters");
            } else if (tokens[1] == "reset") {
                config.enableROI = false;
                ErrorCode err = camera.DisableROI();
                if (err == ErrorCode::Success) {
                    std::cout << "ROI reseteada a sensor completo" << std::endl;
                    Log("ROI reset to full sensor");
                } else {
                    std::cerr << "Error al resetear ROI: " << ErrorCodeToString(err) << std::endl;
                    Log("ERROR: Failed to reset ROI");
                }
            } else if (tokens.size() < 5) {
                std::cerr << "Error: ROI requiere 4 valores: x y width height" << std::endl;
                Log("ERROR: roi command incomplete");
            } else {
                int x = std::atoi(tokens[1].c_str());
                int y = std::atoi(tokens[2].c_str());
                int w = std::atoi(tokens[3].c_str());
                int h = std::atoi(tokens[4].c_str());

                config.enableROI = true;
                config.roiX = x;
                config.roiY = y;
                config.roiWidth = w;
                config.roiHeight = h;

                ErrorCode err = camera.SetROI(x, y, w, h);
                if (err == ErrorCode::Success) {
                    std::cout << "ROI configurada: x=" << x << ", y=" << y
                              << ", w=" << w << ", h=" << h << std::endl;
                    Log("ROI set to x=" + std::to_string(x) + " y=" + std::to_string(y) +
                        " w=" + std::to_string(w) + " h=" + std::to_string(h));
                } else {
                    std::cerr << "Error al configurar ROI: " << ErrorCodeToString(err) << std::endl;
                    Log("ERROR: Failed to set ROI");
                }
            }

        } else if (cmd == "format") {
            if (tokens.size() < 2) {
                std::cerr << "Error: Se requiere formato (TIFF/PNG/BMP/JPG)" << std::endl;
                Log("ERROR: format command missing value");
            } else {
                config.outputFormat = tokens[1];
                std::cout << "Formato configurado: " << config.outputFormat << std::endl;
                Log("Format set to " + config.outputFormat);
            }

        } else if (cmd == "output") {
            if (tokens.size() < 2) {
                std::cerr << "Error: Se requiere ruta de directorio" << std::endl;
                Log("ERROR: output command missing path");
            } else {
                config.outputPath = tokens[1];
                g_outputDir = config.outputPath;
                std::cout << "Directorio de salida: " << config.outputPath << std::endl;
                Log("Output directory set to " + config.outputPath);
            }

        } else if (cmd == "capture") {
            int numFrames = 1;
            if (tokens.size() >= 2) {
                numFrames = std::atoi(tokens[1].c_str());
            }

            config.numFrames = numFrames;

            std::cout << "Capturando " << numFrames << " frame(s)..." << std::endl;
            Log("Capture started: " + std::to_string(numFrames) + " frames");

            CaptureStats stats;
            ErrorCode err = camera.CaptureFrames(config, &stats);

            if (err == ErrorCode::Success) {
                std::cout << "Captura completada exitosamente!" << std::endl;
                std::cout << "  Frames capturados: " << stats.framesCaptured << std::endl;
                std::cout << "  Guardados en: " << config.outputPath << std::endl;
                Log("Capture completed successfully: " + std::to_string(stats.framesCaptured) + " frames");
            } else {
                std::cerr << "Error en captura: " << ErrorCodeToString(err) << std::endl;
                Log("ERROR: Capture failed - " + std::string(ErrorCodeToString(err)));
            }

        } else {
            std::cerr << "Comando desconocido: '" << cmd << "'" << std::endl;
            std::cerr << "Escribe 'help' para ver comandos disponibles." << std::endl;
            Log("ERROR: Unknown command '" + cmd + "'");
        }
    }
}

/**
 * @brief Muestra comandos rápidos (modo legacy)
 */
void ShowQuickCommands(const char* programName) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Dhyana Camera Control v1.0.0" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "\nModos de uso:" << std::endl;
    std::cout << "\n1. MODO INTERACTIVO (Recomendado):" << std::endl;
    std::cout << "   " << programName << std::endl;
    std::cout << "   (sin argumentos para modo interactivo)" << std::endl;
    std::cout << "\n2. MODO LINEA DE COMANDOS:" << std::endl;
    std::cout << "   " << programName << " -h                  # Ayuda completa" << std::endl;
    std::cout << "   " << programName << " -i                  # Informacion de camara" << std::endl;
    std::cout << "   " << programName << " -e 50               # Captura con exposicion 50ms" << std::endl;
    std::cout << "   " << programName << " -e 100 -n 10        # Captura 10 frames" << std::endl;
    std::cout << "\nEjemplos:" << std::endl;
    std::cout << "  " << programName << " -e 50 -n 5 -o ./capturas" << std::endl;
    std::cout << "  " << programName << " -e 100 --binning 2 2 -n 10" << std::endl;
    std::cout << "\nPara ayuda completa: " << programName << " -h" << std::endl;
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
    std::cout << "\nSin argumentos: Modo interactivo" << std::endl;
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
    std::cout << "  # Interactive mode" << std::endl;
    std::cout << "  " << programName << std::endl;
    std::cout << "\n  # Capture single frame with 50ms exposure" << std::endl;
    std::cout << "  " << programName << " -e 50" << std::endl;
    std::cout << "\n  # Capture 10 frames with binning 2x2" << std::endl;
    std::cout << "  " << programName << " -n 10 --binning 2 2" << std::endl;
    std::cout << "\n  # Capture with custom exposure, gain and ROI" << std::endl;
    std::cout << "  " << programName << " -e 100 -g 2.5 -n 5 --roi 100 100 512 512" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Parsea argumentos de línea de comandos
 */
bool ParseArguments(int argc, char* argv[], CaptureConfig& config, int& cameraIndex, bool& showInfo, bool& interactiveMode) {
    std::string programName = GetProgramName(argv[0]);
    const char* exeName = programName.c_str();

    // Si no hay argumentos, modo interactivo
    if (argc == 1) {
        interactiveMode = true;
        return true;
    }

    // Convertir argumentos a mapa
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            ShowHelp(exeName);
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
                g_outputDir = config.outputPath;
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
    // Inicializar logger
    InitializeLogger();
    Log("Program started");

    // Extraer solo el nombre del ejecutable (sin la ruta completa)
    std::string programName = GetProgramName(argv[0]);
    const char* exeName = programName.c_str();

    // Configuración por defecto
    CaptureConfig config;
    int cameraIndex = 0;
    bool showInfo = false;
    bool interactiveMode = false;

    // Parsear argumentos
    if (!ParseArguments(argc, argv, config, cameraIndex, showInfo, interactiveMode)) {
        CloseLogger();
        PauseBeforeExit();
        return 0; // Salir si se mostró ayuda
    }

    // Crear objeto de cámara
    Camera camera;

    // Inicializar SDK
    std::cout << "\nInicializando SDK de camara..." << std::endl;
    Log("Initializing camera SDK");
    ErrorCode err = camera.Initialize();
    if (err != ErrorCode::Success) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: No se pudo inicializar el SDK" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nCodigo de error: " << ErrorCodeToString(err) << std::endl;
        std::cerr << "\nPosibles causas:" << std::endl;
        std::cerr << "  1. Las DLLs del SDK no estan en el directorio del .exe" << std::endl;
        std::cerr << "  2. Falta alguna DLL (copiar todas desde sdk/lib/)" << std::endl;
        std::cerr << "  3. El SDK no esta instalado correctamente" << std::endl;
        std::cerr << "\nSolucion: Ejecuta copy_dlls.bat para copiar las DLLs" << std::endl;
        Log("ERROR: Failed to initialize SDK");
        CloseLogger();
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
        Log("ERROR: No cameras found");
        CloseLogger();
        PauseBeforeExit();
        return 1;
    }

    std::cout << "Camara(s) detectada(s): " << camera.GetCameraCount() << std::endl;
    Log("Cameras detected: " + std::to_string(camera.GetCameraCount()));

    // Abrir cámara
    std::cout << "Abriendo camara " << cameraIndex << "..." << std::endl;
    Log("Opening camera " + std::to_string(cameraIndex));
    err = camera.Open(cameraIndex);
    if (err != ErrorCode::Success) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: No se pudo abrir la camara" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nCodigo de error: " << ErrorCodeToString(err) << std::endl;
        Log("ERROR: Failed to open camera");
        CloseLogger();
        PauseBeforeExit();
        return 1;
    }

    Log("Camera opened successfully");

    // Si es modo interactivo, entrar en el loop
    if (interactiveMode) {
        RunInteractiveMode(camera);
        CloseLogger();
        return 0;
    }

    // Modo legacy (con argumentos)

    // Mostrar información si se solicitó
    if (showInfo) {
        ShowCameraInfo(camera);
        if (argc == 2 || argc == 3) {
            // Solo se pidió info, salir
            CloseLogger();
            PauseBeforeExit();
            return 0;
        }
    }

    // Realizar captura
    std::cout << "\nIniciando captura con la siguiente configuracion:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Exposicion:    " << config.exposureTime << " ms" << std::endl;
    std::cout << "Ganancia:      " << config.gain << std::endl;
    std::cout << "Frames:        " << config.numFrames << std::endl;
    std::cout << "Buffer frames: " << config.bufferFrames << std::endl;
    if (config.totalTime > 0) {
        std::cout << "Tiempo total:  " << config.totalTime << " s" << std::endl;
    }
    if (config.enableBinning) {
        std::cout << "Binning:       " << config.binningX << "x" << config.binningY
                  << " (" << (config.binningMode ? "promedio" : "suma") << ")" << std::endl;
    }
    if (config.enableROI) {
        std::cout << "ROI:           x=" << config.roiX << ", y=" << config.roiY
                  << ", w=" << config.roiWidth << ", h=" << config.roiHeight << std::endl;
    }
    std::cout << "Salida:        " << config.outputPath << std::endl;
    std::cout << "Formato:       " << config.outputFormat << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    Log("Starting capture - Legacy mode");

    // Capturar frames
    CaptureStats stats;
    err = camera.CaptureFrames(config, &stats);
    if (err != ErrorCode::Success) {
        std::cerr << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "  ERROR: La captura fallo" << std::endl;
        std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::cerr << "\nCodigo de error: " << ErrorCodeToString(err) << std::endl;
        Log("ERROR: Capture failed");
        CloseLogger();
        PauseBeforeExit();
        return 1;
    }

    std::cout << "\n==================================================" << std::endl;
    std::cout << "  CAPTURA COMPLETADA EXITOSAMENTE!" << std::endl;
    std::cout << "==================================================" << std::endl;

    Log("Capture completed successfully - " + std::to_string(stats.framesCaptured) + " frames");
    CloseLogger();
    PauseBeforeExit();
    return 0;
}
