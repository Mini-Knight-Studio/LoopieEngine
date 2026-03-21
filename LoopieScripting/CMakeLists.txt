cmake_minimum_required(VERSION 3.10)
project("LoopieScripting" NONE)

set(CS_PROJECT "${CMAKE_CURRENT_SOURCE_DIR}/LoopieScripting.csproj")
set(DLL_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(DLL_DEST_FOLDER "$<TARGET_FILE_DIR:Loopie>")

file(GLOB DLL_FILES "${DLL_FOLDER}/*.dll")

add_custom_target(LoopieScriptingBuild ALL
    COMMAND dotnet build "${CS_PROJECT}"
        -c $<CONFIG>
        -o "${DLL_DEST_FOLDER}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Building LoopieScripting (C#)"
)

foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET LoopieScriptingBuild POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${DLL_FILE}" "${DLL_DEST_FOLDER}"
    )
endforeach()