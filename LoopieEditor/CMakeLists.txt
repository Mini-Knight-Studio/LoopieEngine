cmake_minimum_required (VERSION 3.10)
project ("LoopieEditor" VERSION 0.1.0 LANGUAGES C CXX)


set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB_RECURSE  SOURCES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

target_link_libraries(${PROJECT_NAME} PUBLIC loopie-core)


file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${OUTPUT_PATH})


add_dependencies(${PROJECT_NAME}
    LoopieScriptingBuild LoopieCompilerBuild
)

# Copy Mono DLL
add_custom_command(TARGET LoopieEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${MONO_BIN_DIR}/mono-2.0-sgen.dll"
    $<TARGET_FILE_DIR:LoopieEditor>
)

# Copy Mono lib folder
add_custom_command(TARGET LoopieEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${MONO_ROOT}/lib"
    $<TARGET_FILE_DIR:LoopieEditor>/mono/lib
)

# Copy Mono etc folder
add_custom_command(TARGET LoopieEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${MONO_ROOT}/etc"
    $<TARGET_FILE_DIR:LoopieEditor>/mono/etc
)

#Copy fmod dlls
set(FMOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/fmod")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FMOD_ARCH_DIR "x64")
else()
    set(FMOD_ARCH_DIR "x86")
endif()

add_custom_command(TARGET LoopieEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_DIR}/api/core/lib/${FMOD_ARCH_DIR}/fmod.dll"
    $<TARGET_FILE_DIR:LoopieEditor>
)

add_custom_command(TARGET LoopieEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_DIR}/api/studio/lib/${FMOD_ARCH_DIR}/fmodstudio.dll"
    $<TARGET_FILE_DIR:LoopieEditor>
)

