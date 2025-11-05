#include "DhyanaCamera.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

namespace Dhyana {

Camera::Camera()
    : hCamera_(nullptr)
    , isInitialized_(false)
    , isOpen_(false)
    , isCapturing_(false)
    , cameraCount_(0)
{
    memset(&currentFrame_, 0, sizeof(TUCAM_FRAME));
}

Camera::~Camera()
{
    if (isCapturing_) {
        StopCapture();
    }
    if (isOpen_) {
        Close();
    }
    if (isInitialized_) {
        TUCAM_Api_Uninit();
    }
}

ErrorCode Camera::Initialize()
{
    if (isInitialized_) {
        return ErrorCode::Success;
    }

    TUCAM_INIT initParam;
    memset(&initParam, 0, sizeof(TUCAM_INIT));
    initParam.pstrConfigPath = nullptr; // Usar configuración por defecto

    TUCAMRET ret = TUCAM_Api_Init(&initParam);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to initialize TUCAM API. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    cameraCount_ = initParam.uiCamCount;
    isInitialized_ = true;

    std::cout << "TUCAM API initialized successfully." << std::endl;
    std::cout << "Found " << cameraCount_ << " camera(s)." << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::Open(uint32_t cameraIndex)
{
    if (!isInitialized_) {
        return ErrorCode::CameraNotInitialized;
    }

    if (isOpen_) {
        return ErrorCode::Success;
    }

    if (cameraIndex >= cameraCount_) {
        std::cerr << "Error: Camera index " << cameraIndex
                  << " out of range (0-" << cameraCount_ - 1 << ")" << std::endl;
        return ErrorCode::CameraNotFound;
    }

    TUCAM_OPEN openParam;
    memset(&openParam, 0, sizeof(TUCAM_OPEN));
    openParam.uiIdxOpen = cameraIndex;

    TUCAMRET ret = TUCAM_Dev_Open(&openParam);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to open camera. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    hCamera_ = openParam.hIdxTUCam;
    isOpen_ = true;

    std::cout << "Camera opened successfully." << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::Close()
{
    if (!isOpen_) {
        return ErrorCode::Success;
    }

    if (isCapturing_) {
        StopCapture();
    }

    ReleaseBuffer();

    TUCAMRET ret = TUCAM_Dev_Close(hCamera_);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Warning: Failed to close camera properly. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
    }

    hCamera_ = nullptr;
    isOpen_ = false;

    std::cout << "Camera closed." << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::GetCameraInfo(CameraInfo& info)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    // Obtener modelo de cámara
    char modelBuffer[64] = {0};
    TUCAM_VALUE_INFO valueInfo;
    memset(&valueInfo, 0, sizeof(TUCAM_VALUE_INFO));
    valueInfo.nID = TUIDI_CAMERA_MODEL;
    valueInfo.pText = modelBuffer;
    valueInfo.nTextSize = sizeof(modelBuffer);

    TUCAMRET ret = TUCAM_Dev_GetInfo(hCamera_, &valueInfo);
    if (TUCAMRET_SUCCESS == ret) {
        info.model = std::string(modelBuffer);
    }

    // Obtener versión de firmware
    valueInfo.nID = TUIDI_VERSION_FRMW;
    ret = TUCAM_Dev_GetInfo(hCamera_, &valueInfo);
    if (TUCAMRET_SUCCESS == ret) {
        info.firmwareVersion = std::to_string(valueInfo.nValue);
    }

    // Obtener dimensiones del sensor (desde las capacidades)
    TUCAM_CAPA_ATTR capaAttr;
    memset(&capaAttr, 0, sizeof(TUCAM_CAPA_ATTR));
    capaAttr.idCapa = TUIDC_RESOLUTION;

    ret = TUCAM_Capa_GetAttr(hCamera_, &capaAttr);
    if (TUCAMRET_SUCCESS == ret) {
        // El valor máximo contiene width y height empaquetados
        info.sensorWidth = (capaAttr.nValMax >> 16) & 0xFFFF;
        info.sensorHeight = capaAttr.nValMax & 0xFFFF;
    }

    // Obtener profundidad de bits
    capaAttr.idCapa = TUIDC_BITOFDEPTH;
    ret = TUCAM_Capa_GetAttr(hCamera_, &capaAttr);
    if (TUCAMRET_SUCCESS == ret) {
        info.bitDepth = capaAttr.nValMax;
    }

    // Para Dhyana 2100, el tamaño de píxel es típicamente 11 micrones
    info.pixelSize = 11.0;

    return ErrorCode::Success;
}

ErrorCode Camera::SetExposure(double exposureMs)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    TUCAMRET ret = TUCAM_Prop_SetValue(hCamera_, TUIDP_EXPOSURETM, exposureMs);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to set exposure time. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    std::cout << "Exposure time set to " << exposureMs << " ms" << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::GetExposure(double& exposureMs)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    TUCAMRET ret = TUCAM_Prop_GetValue(hCamera_, TUIDP_EXPOSURETM, &exposureMs);
    if (TUCAMRET_SUCCESS != ret) {
        return ConvertSDKError(ret);
    }

    return ErrorCode::Success;
}

ErrorCode Camera::SetGain(double gain)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    TUCAMRET ret = TUCAM_Prop_SetValue(hCamera_, TUIDP_GLOBALGAIN, gain);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to set gain. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    std::cout << "Gain set to " << gain << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::GetGain(double& gain)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    TUCAMRET ret = TUCAM_Prop_GetValue(hCamera_, TUIDP_GLOBALGAIN, &gain);
    if (TUCAMRET_SUCCESS != ret) {
        return ConvertSDKError(ret);
    }

    return ErrorCode::Success;
}

ErrorCode Camera::SetBinning(int32_t binX, int32_t binY, bool averageMode)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    if (isCapturing_) {
        std::cerr << "Error: Cannot change binning while capturing." << std::endl;
        return ErrorCode::InvalidParameter;
    }

    TUCAM_BIN_ATTR binAttr;
    memset(&binAttr, 0, sizeof(TUCAM_BIN_ATTR));
    binAttr.bEnable = TRUE;
    binAttr.nMode = averageMode ? TUBIN_MODE_AVERAGE : TUBIN_MODE_SUM;
    binAttr.nWidth = binX;
    binAttr.nHeight = binY;

    TUCAMRET ret = TUCAM_Cap_SetBIN(hCamera_, binAttr);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to set binning. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    std::cout << "Binning set to " << binX << "x" << binY
              << " (" << (averageMode ? "average" : "sum") << " mode)" << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::SetROI(int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    if (isCapturing_) {
        std::cerr << "Error: Cannot change ROI while capturing." << std::endl;
        return ErrorCode::InvalidParameter;
    }

    TUCAM_ROI_ATTR roiAttr;
    memset(&roiAttr, 0, sizeof(TUCAM_ROI_ATTR));
    roiAttr.bEnable = TRUE;
    roiAttr.nHOffset = x;
    roiAttr.nVOffset = y;
    roiAttr.nWidth = width;
    roiAttr.nHeight = height;

    TUCAMRET ret = TUCAM_Cap_SetROI(hCamera_, roiAttr);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to set ROI. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    std::cout << "ROI set to: x=" << x << ", y=" << y
              << ", width=" << width << ", height=" << height << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::DisableROI()
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    if (isCapturing_) {
        std::cerr << "Error: Cannot change ROI while capturing." << std::endl;
        return ErrorCode::InvalidParameter;
    }

    TUCAM_ROI_ATTR roiAttr;
    memset(&roiAttr, 0, sizeof(TUCAM_ROI_ATTR));
    roiAttr.bEnable = FALSE;

    TUCAMRET ret = TUCAM_Cap_SetROI(hCamera_, roiAttr);
    if (TUCAMRET_SUCCESS != ret) {
        return ConvertSDKError(ret);
    }

    std::cout << "ROI disabled (using full sensor)." << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::AllocateBuffer(int32_t numBuffers)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    // Liberar buffer existente si hay
    ReleaseBuffer();

    // Preparar estructura de frame
    memset(&currentFrame_, 0, sizeof(TUCAM_FRAME));
    currentFrame_.ucFormatGet = TUFRM_FMT_USUAl;
    currentFrame_.uiRsdSize = numBuffers; // Número de buffers

    TUCAMRET ret = TUCAM_Buf_Alloc(hCamera_, &currentFrame_);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to allocate buffer. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    std::cout << "Buffer allocated: " << currentFrame_.usWidth << "x"
              << currentFrame_.usHeight << " (" << numBuffers << " frames)" << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::ReleaseBuffer()
{
    if (!isOpen_) {
        return ErrorCode::Success;
    }

    TUCAMRET ret = TUCAM_Buf_Release(hCamera_);
    if (TUCAMRET_SUCCESS != ret && TUCAMRET_NOT_INIT != ret) {
        std::cerr << "Warning: Failed to release buffer. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
    }

    memset(&currentFrame_, 0, sizeof(TUCAM_FRAME));

    return ErrorCode::Success;
}

ErrorCode Camera::StartCapture(const CaptureConfig& config)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    if (isCapturing_) {
        return ErrorCode::Success;
    }

    currentConfig_ = config;

    // Configurar exposición
    ErrorCode err = SetExposure(config.exposureTime);
    if (err != ErrorCode::Success) {
        return err;
    }

    // Configurar ganancia
    err = SetGain(config.gain);
    if (err != ErrorCode::Success) {
        return err;
    }

    // Configurar binning si está habilitado
    if (config.enableBinning) {
        err = SetBinning(config.binningX, config.binningY, config.binningMode);
        if (err != ErrorCode::Success) {
            return err;
        }
    }

    // Configurar ROI si está habilitado
    if (config.enableROI) {
        err = SetROI(config.roiX, config.roiY, config.roiWidth, config.roiHeight);
        if (err != ErrorCode::Success) {
            return err;
        }
    } else {
        DisableROI();
    }

    // Asignar buffer
    err = AllocateBuffer(config.bufferFrames);
    if (err != ErrorCode::Success) {
        return err;
    }

    // Iniciar captura en modo secuencia
    TUCAMRET ret = TUCAM_Cap_Start(hCamera_, TUCCM_SEQUENCE);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to start capture. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    isCapturing_ = true;
    std::cout << "Capture started." << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::StopCapture()
{
    if (!isCapturing_) {
        return ErrorCode::Success;
    }

    TUCAMRET ret = TUCAM_Cap_Stop(hCamera_);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Warning: Failed to stop capture. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
    }

    isCapturing_ = false;
    std::cout << "Capture stopped." << std::endl;

    return ErrorCode::Success;
}

ErrorCode Camera::WaitForFrame(int32_t timeoutMs)
{
    if (!isCapturing_) {
        return ErrorCode::CameraNotInitialized;
    }

    TUCAMRET ret = TUCAM_Buf_WaitForFrame(hCamera_, &currentFrame_, timeoutMs);
    if (TUCAMRET_SUCCESS != ret) {
        if (ret == TUCAMRET_TIMEOUT) {
            return ErrorCode::FrameWaitTimeout;
        }
        return ConvertSDKError(ret);
    }

    return ErrorCode::Success;
}

ErrorCode Camera::SaveCurrentFrame(const std::string& filename, const std::string& format)
{
    if (!isOpen_) {
        return ErrorCode::CameraNotInitialized;
    }

    if (currentFrame_.pBuffer == nullptr) {
        std::cerr << "Error: No frame data available." << std::endl;
        return ErrorCode::InvalidParameter;
    }

    TUCAM_FILE_SAVE fileSave;
    memset(&fileSave, 0, sizeof(TUCAM_FILE_SAVE));
    fileSave.nSaveFmt = GetImageFormat(format);
    fileSave.pstrSavePath = const_cast<char*>(filename.c_str());
    fileSave.pFrame = &currentFrame_;

    TUCAMRET ret = TUCAM_File_SaveImage(hCamera_, fileSave);
    if (TUCAMRET_SUCCESS != ret) {
        std::cerr << "Error: Failed to save image. Error code: 0x"
                  << std::hex << ret << std::dec << std::endl;
        return ConvertSDKError(ret);
    }

    return ErrorCode::Success;
}

ErrorCode Camera::CaptureFrames(const CaptureConfig& config, CaptureStats* stats)
{
    // Crear directorio de salida si no existe
    if (!CreateDirectory(config.outputPath)) {
        std::cerr << "Error: Failed to create output directory: " << config.outputPath << std::endl;
        return ErrorCode::InvalidParameter;
    }

    // Iniciar captura
    ErrorCode err = StartCapture(config);
    if (err != ErrorCode::Success) {
        return err;
    }

    CaptureStats localStats;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::cout << "\nCapturing " << config.numFrames << " frame(s)..." << std::endl;
    std::cout << "Exposure: " << config.exposureTime << " ms, Gain: " << config.gain << std::endl;
    std::cout << "Output: " << config.outputPath << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    for (int32_t i = 0; i < config.numFrames; ++i) {
        // Esperar frame con timeout
        err = WaitForFrame(5000);
        if (err != ErrorCode::Success) {
            std::cerr << "Error waiting for frame " << (i + 1) << std::endl;
            localStats.framesDropped++;
            continue;
        }

        // Generar nombre de archivo
        std::ostringstream oss;
        oss << config.outputPath << "/frame_"
            << std::setfill('0') << std::setw(4) << (i + 1)
            << "." << (config.outputFormat == "TIFF" ? "tif" :
                      config.outputFormat == "PNG" ? "png" :
                      config.outputFormat == "BMP" ? "bmp" : "jpg");
        std::string filename = oss.str();

        // Guardar frame
        err = SaveCurrentFrame(filename, config.outputFormat);
        if (err == ErrorCode::Success) {
            localStats.framesCaptured++;
            std::cout << "Saved: " << filename << " [" << (i + 1)
                      << "/" << config.numFrames << "]" << std::endl;
        } else {
            std::cerr << "Failed to save frame " << (i + 1) << std::endl;
            localStats.framesDropped++;
        }

        // Verificar tiempo total si está configurado
        if (config.totalTime > 0.0) {
            auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
            double elapsedSec = std::chrono::duration<double>(elapsed).count();
            if (elapsedSec >= config.totalTime) {
                std::cout << "Total acquisition time reached: " << elapsedSec << " s" << std::endl;
                break;
            }
        }
    }

    // Calcular estadísticas
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double>(endTime - startTime);
    localStats.elapsedTime = elapsed.count();
    localStats.averageFPS = localStats.framesCaptured / localStats.elapsedTime;

    // Detener captura
    StopCapture();

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Capture complete!" << std::endl;
    std::cout << "Frames captured: " << localStats.framesCaptured << std::endl;
    std::cout << "Frames dropped: " << localStats.framesDropped << std::endl;
    std::cout << "Elapsed time: " << std::fixed << std::setprecision(2)
              << localStats.elapsedTime << " s" << std::endl;
    std::cout << "Average FPS: " << std::fixed << std::setprecision(2)
              << localStats.averageFPS << std::endl;

    if (stats) {
        *stats = localStats;
    }

    return ErrorCode::Success;
}

ErrorCode Camera::ConvertSDKError(TUCAMRET ret)
{
    switch (ret) {
        case TUCAMRET_SUCCESS:
            return ErrorCode::Success;
        case TUCAMRET_NO_CAMERA:
            return ErrorCode::CameraNotFound;
        case TUCAMRET_FAILOPEN_CAMERA:
            return ErrorCode::CannotOpenCamera;
        case TUCAMRET_INVALID_PARAM:
        case TUCAMRET_INVALID_VALUE:
            return ErrorCode::InvalidParameter;
        case TUCAMRET_NO_MEMORY:
            return ErrorCode::BufferAllocationFailed;
        case TUCAMRET_TIMEOUT:
            return ErrorCode::FrameWaitTimeout;
        default:
            return ErrorCode::SDKError;
    }
}

bool Camera::CreateDirectory(const std::string& path)
{
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
        return false;
    }
}

int32_t Camera::GetImageFormat(const std::string& format)
{
    if (format == "TIFF" || format == "TIF") {
        return TUFMT_TIF;
    } else if (format == "PNG") {
        return TUFMT_PNG;
    } else if (format == "BMP") {
        return TUFMT_BMP;
    } else if (format == "JPG" || format == "JPEG") {
        return TUFMT_JPG;
    } else {
        return TUFMT_TIF; // Default
    }
}

} // namespace Dhyana
