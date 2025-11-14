cmake_minimum_required(VERSION 3.15)
project(DhyanaCameraControl VERSION 1.0.0 LANGUAGES CXX)

# Configuración de C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configuración para Windows
if(WIN32)
    add_definitions(-DWIN32 -D_WINDOWS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # Linkear estáticamente con el runtime de Visual Studio
    # Esto evita necesitar DLLs de Visual C++ Redistributables
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Incluir directorios del SDK de Tucsen
include_directories(
    ${CMAKE_SOURCE_DIR}/sdk/inc
    ${CMAKE_SOURCE_DIR}/include
)

# Buscar la librería TUCam
find_library(TUCAM_LIBRARY
    NAMES TUCam
    PATHS ${CMAKE_SOURCE_DIR}/sdk/lib
    NO_DEFAULT_PATH
)

if(NOT TUCAM_LIBRARY)
    message(FATAL_ERROR "TUCam library not found in ${CMAKE_SOURCE_DIR}/sdk/lib")
endif()

message(STATUS "Found TUCam library: ${TUCAM_LIBRARY}")

# Archivos fuente del proyecto
set(SOURCES
    src/DhyanaCamera.cpp
    src/main.cpp
)

set(HEADERS
    include/DhyanaCamera.h
    include/DhyanaTypes.h
)

# Crear el ejecutable
add_executable(dhyana_control ${SOURCES} ${HEADERS})

# Linkear con la librería TUCam
target_link_libraries(dhyana_control ${TUCAM_LIBRARY})

# Copiar DLLs necesarias al directorio de salida en Windows
if(WIN32)
    # 1. Copiar TODAS las DLLs del SDK al directorio de salida
    file(GLOB SDK_DLLS "${CMAKE_SOURCE_DIR}/sdk/lib/*.dll")

    foreach(DLL_PATH ${SDK_DLLS})
        add_custom_command(TARGET dhyana_control POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_PATH}"
                "$<TARGET_FILE_DIR:dhyana_control>/"
            COMMENT "Copying SDK DLL: ${DLL_PATH}"
        )
    endforeach()

    list(LENGTH SDK_DLLS SDK_DLL_COUNT)
    message(STATUS "Found ${SDK_DLL_COUNT} SDK DLLs to copy")

    # 2. Copiar DLLs de Visual C++ Runtime necesarias
    # Lista de DLLs de runtime requeridas por las DLLs del SDK de Tucsen
    set(RUNTIME_DLLS
        # Visual C++ 2013 Runtime (VC120) - requerido por algunas DLLs del SDK
        "msvcp120.dll"
        "msvcr120.dll"
        # Visual C++ 2015-2022 Runtime (VC140/141/142/143) - requerido por nuestro ejecutable
        "msvcp140.dll"
        "vcruntime140.dll"
        "vcruntime140_1.dll"
        # DLLs de sistema de Windows (multimedia/video)
        "msvfw32.dll"
        "msacm32.dll"
        "winmm.dll"
        "mpr.dll"
    )

    foreach(RUNTIME_DLL ${RUNTIME_DLLS})
        # Buscar primero en C:\Windows\System32
        set(RUNTIME_DLL_PATH "C:/Windows/System32/${RUNTIME_DLL}")

        if(EXISTS "${RUNTIME_DLL_PATH}")
            add_custom_command(TARGET dhyana_control POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RUNTIME_DLL_PATH}"
                    "$<TARGET_FILE_DIR:dhyana_control>/"
                COMMENT "Copying runtime DLL: ${RUNTIME_DLL}"
            )
            message(STATUS "Will copy runtime DLL: ${RUNTIME_DLL} from System32")
        else()
            # Si no está en System32, buscar en el SDK de Tucsen instalado
            # (útil en PCs sin Visual Studio instalado)
            set(TUCSEN_SDK_PATH "C:/Program Files/TUCam_SDK/sdk/lib/x64/${RUNTIME_DLL}")
            if(EXISTS "${TUCSEN_SDK_PATH}")
                add_custom_command(TARGET dhyana_control POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${TUCSEN_SDK_PATH}"
                        "$<TARGET_FILE_DIR:dhyana_control>/"
                    COMMENT "Copying runtime DLL: ${RUNTIME_DLL} from Tucsen SDK"
                )
                message(STATUS "Will copy runtime DLL: ${RUNTIME_DLL} from Tucsen SDK")
            else()
                message(WARNING "${RUNTIME_DLL} not found in System32 or Tucsen SDK. May need manual installation.")
            endif()
        endif()
    endforeach()

    message(STATUS "Runtime DLLs will be copied automatically during build")
    message(STATUS "Note: If build succeeds but executable fails to run, ensure Visual C++ Redistributables are installed")
endif()

# Configuración de instalación
install(TARGETS dhyana_control DESTINATION bin)
install(DIRECTORY sdk/lib/ DESTINATION bin FILES_MATCHING PATTERN "*.dll")

# Información de compilación
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
