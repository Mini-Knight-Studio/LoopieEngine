cmake_minimum_required(VERSION 3.10)
project("Loopie" VERSION 0.1.0 LANGUAGES C CXX)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/fmod")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FMOD_ARCH_DIR "x64")
else()
    set(FMOD_ARCH_DIR "x86")
endif()

find_library(FMOD_CORE_LIB 
    NAMES fmod fmodL fmod_vc fmodL_vc libfmod
    HINTS "${FMOD_DIR}/api/core/lib/${FMOD_ARCH_DIR}"
    NO_DEFAULT_PATH
)

find_library(FMOD_STUDIO_LIB 
    NAMES fmodstudio fmodstudioL fmodstudio_vc fmodstudioL_vc libfmodstudio
    HINTS "${FMOD_DIR}/api/studio/lib/${FMOD_ARCH_DIR}"
    NO_DEFAULT_PATH
)

if(NOT FMOD_CORE_LIB OR NOT FMOD_STUDIO_LIB)
    message(FATAL_ERROR "No se encontró FMOD en ${FMOD_DIR}. Verifica la ruta.")
endif()


file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h" "${VENDOR_DIR}/*.cpp" "${VENDOR_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

find_package(spdlog REQUIRED)
find_package(glad REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(SDL3 REQUIRED)
find_package(nfd REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(assimp REQUIRED)
find_package(lz4 REQUIRED)
find_package(imguizmo REQUIRED)
find_package(imgui REQUIRED)
find_package(DevIL REQUIRED)

add_library(${PROJECT_NAME} ${SOURCES})
add_library(loopie-core ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}" "${VENDOR_DIR}" "${FMOD_DIR}/api/core/inc" "${FMOD_DIR}/api/studio/inc")

target_link_libraries(${PROJECT_NAME} PUBLIC
    spdlog::spdlog
    glad::glad
    OpenGL::GL
    glm::glm
    SDL3::SDL3
    nfd::nfd
    nlohmann_json::nlohmann_json
    assimp::assimp
    lz4::lz4
    imguizmo::imguizmo
    imgui::imgui
    DevIL::IL
    DevIL::ILU
    Mono::Runtime
    ${FMOD_CORE_LIB}
    ${FMOD_STUDIO_LIB}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS GLFW_INCLUDE_NONE)
