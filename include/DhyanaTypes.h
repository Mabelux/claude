#ifndef DHYANA_TYPES_H
#define DHYANA_TYPES_H

#include <string>
#include <cstdint>

namespace Dhyana {

/**
 * @brief Estructura de configuración de captura
 */
struct CaptureConfig {
    double exposureTime;        // Tiempo de exposición en milisegundos
    double gain;                // Ganancia (depende del modelo de cámara)
    int32_t numFrames;          // Número de frames a capturar
    int32_t bufferFrames;       // Número de frames en buffer
    double totalTime;           // Tiempo total de adquisición en segundos (0 = continuo)

    // Configuración de binning
    bool enableBinning;         // Habilitar binning
    int32_t binningX;           // Binning horizontal (1, 2, 4, 8...)
    int32_t binningY;           // Binning vertical (1, 2, 4, 8...)
    bool binningMode;           // true = average, false = sum

    // Configuración de ROI
    bool enableROI;             // Habilitar ROI
    int32_t roiX;               // Offset horizontal de ROI
    int32_t roiY;               // Offset vertical de ROI
    int32_t roiWidth;           // Ancho de ROI
    int32_t roiHeight;          // Alto de ROI

    // Configuración de guardado
    std::string outputPath;     // Ruta de guardado
    std::string outputFormat;   // Formato de salida (TIFF, PNG, BMP, etc.)
    bool saveRaw;               // Guardar datos RAW además del formato especificado

    // Constructor con valores por defecto
    CaptureConfig()
        : exposureTime(100.0)
        , gain(1.0)
        , numFrames(1)
        , bufferFrames(10)
        , totalTime(0.0)
        , enableBinning(false)
        , binningX(1)
        , binningY(1)
        , binningMode(true)
        , enableROI(false)
        , roiX(0)
        , roiY(0)
        , roiWidth(0)
        , roiHeight(0)
        , outputPath("./output")
        , outputFormat("TIFF")
        , saveRaw(false)
    {}
};

/**
 * @brief Información de la cámara
 */
struct CameraInfo {
    std::string model;          // Modelo de la cámara
    std::string serialNumber;   // Número de serie
    std::string firmwareVersion;// Versión del firmware
    int32_t sensorWidth;        // Ancho del sensor en píxeles
    int32_t sensorHeight;       // Alto del sensor en píxeles
    int32_t bitDepth;           // Profundidad de bits
    double pixelSize;           // Tamaño del píxel en micrones

    CameraInfo()
        : sensorWidth(0)
        , sensorHeight(0)
        , bitDepth(0)
        , pixelSize(0.0)
    {}
};

/**
 * @brief Estadísticas de captura
 */
struct CaptureStats {
    int32_t framesCaptured;     // Frames capturados
    int32_t framesDropped;      // Frames perdidos
    double averageFPS;          // FPS promedio
    double elapsedTime;         // Tiempo transcurrido en segundos

    CaptureStats()
        : framesCaptured(0)
        , framesDropped(0)
        , averageFPS(0.0)
        , elapsedTime(0.0)
    {}
};

/**
 * @brief Códigos de error personalizados
 */
enum class ErrorCode {
    Success = 0,
    CameraNotFound,
    CannotOpenCamera,
    InvalidParameter,
    BufferAllocationFailed,
    CaptureStartFailed,
    FrameWaitTimeout,
    SaveImageFailed,
    CameraNotInitialized,
    SDKError
};

/**
 * @brief Convierte ErrorCode a string
 */
inline std::string ErrorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success:
            return "Success";
        case ErrorCode::CameraNotFound:
            return "Camera not found";
        case ErrorCode::CannotOpenCamera:
            return "Cannot open camera";
        case ErrorCode::InvalidParameter:
            return "Invalid parameter";
        case ErrorCode::BufferAllocationFailed:
            return "Buffer allocation failed";
        case ErrorCode::CaptureStartFailed:
            return "Capture start failed";
        case ErrorCode::FrameWaitTimeout:
            return "Frame wait timeout";
        case ErrorCode::SaveImageFailed:
            return "Save image failed";
        case ErrorCode::CameraNotInitialized:
            return "Camera not initialized";
        case ErrorCode::SDKError:
            return "SDK error";
        default:
            return "Unknown error";
    }
}

} // namespace Dhyana

#endif // DHYANA_TYPES_H
