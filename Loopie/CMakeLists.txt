cmake_minimum_required(VERSION 3.10)
project("Loopie" VERSION 0.1.0 LANGUAGES C CXX)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
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

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}" "${VENDOR_DIR}")

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
)

target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS GLFW_INCLUDE_NONE)
