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
    # Copiar TODAS las DLLs del SDK al directorio de salida
    file(GLOB SDK_DLLS "${CMAKE_SOURCE_DIR}/sdk/lib/*.dll")

    foreach(DLL_PATH ${SDK_DLLS})
        add_custom_command(TARGET dhyana_control POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_PATH}"
                "$<TARGET_FILE_DIR:dhyana_control>/"
            COMMENT "Copying SDK DLLs to output directory"
        )
    endforeach()

    message(STATUS "Found ${SDK_DLLS} SDK DLLs to copy")
endif()

# Configuración de instalación
install(TARGETS dhyana_control DESTINATION bin)
install(DIRECTORY sdk/lib/ DESTINATION bin FILES_MATCHING PATTERN "*.dll")

# Información de compilación
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
