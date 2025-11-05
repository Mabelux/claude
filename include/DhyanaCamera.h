#ifndef DHYANA_CAMERA_H
#define DHYANA_CAMERA_H

#include "DhyanaTypes.h"
#include "TUCamApi.h"
#include <memory>
#include <vector>
#include <functional>

namespace Dhyana {

/**
 * @brief Clase wrapper para controlar cámaras Dhyana usando el SDK de Tucsen
 *
 * Esta clase proporciona una interfaz C++ moderna para controlar cámaras Dhyana,
 * encapsulando la API C del SDK de Tucsen.
 */
class Camera {
public:
    /**
     * @brief Constructor
     */
    Camera();

    /**
     * @brief Destructor
     */
    ~Camera();

    // Prevenir copia
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    /**
     * @brief Inicializa el SDK y busca cámaras disponibles
     * @return ErrorCode indicando el resultado
     */
    ErrorCode Initialize();

    /**
     * @brief Abre la cámara especificada
     * @param cameraIndex Índice de la cámara (0 para la primera)
     * @return ErrorCode indicando el resultado
     */
    ErrorCode Open(uint32_t cameraIndex = 0);

    /**
     * @brief Cierra la cámara
     * @return ErrorCode indicando el resultado
     */
    ErrorCode Close();

    /**
     * @brief Obtiene información de la cámara
     * @param info Estructura donde se almacenará la información
     * @return ErrorCode indicando el resultado
     */
    ErrorCode GetCameraInfo(CameraInfo& info);

    /**
     * @brief Configura el tiempo de exposición
     * @param exposureMs Tiempo de exposición en milisegundos
     * @return ErrorCode indicando el resultado
     */
    ErrorCode SetExposure(double exposureMs);

    /**
     * @brief Obtiene el tiempo de exposición actual
     * @param exposureMs Variable donde se almacenará el valor
     * @return ErrorCode indicando el resultado
     */
    ErrorCode GetExposure(double& exposureMs);

    /**
     * @brief Configura la ganancia
     * @param gain Valor de ganancia
     * @return ErrorCode indicando el resultado
     */
    ErrorCode SetGain(double gain);

    /**
     * @brief Obtiene la ganancia actual
     * @param gain Variable donde se almacenará el valor
     * @return ErrorCode indicando el resultado
     */
    ErrorCode GetGain(double& gain);

    /**
     * @brief Configura el binning
     * @param binX Binning horizontal
     * @param binY Binning vertical
     * @param averageMode true para modo average, false para modo sum
     * @return ErrorCode indicando el resultado
     */
    ErrorCode SetBinning(int32_t binX, int32_t binY, bool averageMode = true);

    /**
     * @brief Configura la región de interés (ROI)
     * @param x Offset horizontal
     * @param y Offset vertical
     * @param width Ancho
     * @param height Alto
     * @return ErrorCode indicando el resultado
     */
    ErrorCode SetROI(int32_t x, int32_t y, int32_t width, int32_t height);

    /**
     * @brief Deshabilita el ROI (usa el sensor completo)
     * @return ErrorCode indicando el resultado
     */
    ErrorCode DisableROI();

    /**
     * @brief Asigna el buffer de frames
     * @param numBuffers Número de buffers a asignar
     * @return ErrorCode indicando el resultado
     */
    ErrorCode AllocateBuffer(int32_t numBuffers);

    /**
     * @brief Libera el buffer de frames
     * @return ErrorCode indicando el resultado
     */
    ErrorCode ReleaseBuffer();

    /**
     * @brief Inicia la captura
     * @param config Configuración de captura
     * @return ErrorCode indicando el resultado
     */
    ErrorCode StartCapture(const CaptureConfig& config);

    /**
     * @brief Detiene la captura
     * @return ErrorCode indicando el resultado
     */
    ErrorCode StopCapture();

    /**
     * @brief Captura un número específico de frames
     * @param config Configuración de captura
     * @param stats Estadísticas de captura (opcional)
     * @return ErrorCode indicando el resultado
     */
    ErrorCode CaptureFrames(const CaptureConfig& config, CaptureStats* stats = nullptr);

    /**
     * @brief Espera por un frame
     * @param timeoutMs Timeout en milisegundos
     * @return ErrorCode indicando el resultado
     */
    ErrorCode WaitForFrame(int32_t timeoutMs = 5000);

    /**
     * @brief Guarda el frame actual
     * @param filename Nombre del archivo
     * @param format Formato de imagen (TIFF, PNG, BMP, etc.)
     * @return ErrorCode indicando el resultado
     */
    ErrorCode SaveCurrentFrame(const std::string& filename, const std::string& format = "TIFF");

    /**
     * @brief Verifica si la cámara está abierta
     * @return true si la cámara está abierta
     */
    bool IsOpen() const { return isOpen_; }

    /**
     * @brief Verifica si la captura está activa
     * @return true si la captura está activa
     */
    bool IsCapturing() const { return isCapturing_; }

    /**
     * @brief Obtiene el número de cámaras disponibles
     * @return Número de cámaras
     */
    uint32_t GetCameraCount() const { return cameraCount_; }

    /**
     * @brief Obtiene el handle del SDK (para uso avanzado)
     * @return Handle de la cámara
     */
    HDTUCAM GetHandle() const { return hCamera_; }

private:
    /**
     * @brief Convierte código de retorno del SDK a ErrorCode
     */
    ErrorCode ConvertSDKError(TUCAMRET ret);

    /**
     * @brief Crea el directorio si no existe
     */
    bool CreateDirectory(const std::string& path);

    /**
     * @brief Obtiene el formato de imagen del SDK según el string
     */
    int32_t GetImageFormat(const std::string& format);

private:
    HDTUCAM hCamera_;               // Handle de la cámara
    TUCAM_FRAME currentFrame_;      // Frame actual
    bool isInitialized_;            // SDK inicializado
    bool isOpen_;                   // Cámara abierta
    bool isCapturing_;              // Captura activa
    uint32_t cameraCount_;          // Número de cámaras disponibles
    CaptureConfig currentConfig_;   // Configuración actual
};

} // namespace Dhyana

#endif // DHYANA_CAMERA_H
