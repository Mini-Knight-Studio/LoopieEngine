cmake_minimum_required(VERSION 3.10)
project("LoopieCompiler" NONE)

set(CS_PROJECT "${CMAKE_CURRENT_SOURCE_DIR}/LoopieCompiler.csproj")

add_custom_target(LoopieCompilerBuild ALL
    COMMAND dotnet build "${CS_PROJECT}"
        -c $<CONFIG>
        -o "$<TARGET_FILE_DIR:Loopie>"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Building LoopieCompiler (C#)"
)