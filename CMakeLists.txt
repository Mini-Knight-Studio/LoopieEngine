cmake_minimum_required (VERSION 3.10)
project ("LoopieEngine" VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/EHs" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/EHc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_options(/GR- /EHs-)
else()
    add_compile_options(-fno-rtti -fno-exceptions)
endif()

#Mono Configuration
set(MONO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono")
set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
set(MONO_LIB_DIR "${MONO_ROOT}/lib")
set(MONO_BIN_DIR "${MONO_ROOT}/bin")

set(FMOD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmod")

find_library(MONO_LIBRARY NAMES mono-2.0-sgen PATHS "${MONO_LIB_DIR}" NO_DEFAULT_PATH)

add_library(Mono::Runtime SHARED IMPORTED GLOBAL)
set_target_properties(Mono::Runtime PROPERTIES IMPORTED_LOCATION "${MONO_BIN_DIR}/mono-2.0-sgen.dll" IMPORTED_IMPLIB "${MONO_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES "${MONO_INCLUDE_DIR}")

#Subdirectories
add_subdirectory("Loopie")
add_subdirectory("LoopieEditor")
add_subdirectory("LoopieScripting")
add_subdirectory("LoopieCompiler")
add_subdirectory("LoopieProjectBuild")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT LoopieEditor)    