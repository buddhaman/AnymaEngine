@echo off
setlocal

set SDL2_PATH=external\sdl2
set IMGUI_PATH=external\imgui
set IMGPLOT_PATH=external\implot

rem Check if em++ is available
where em++ >nul 2>&1
if errorlevel 1 (
    echo Emscripten is not installed or not in PATH. Exiting...
    exit /b 1
)

rem Check for release or debug argument
if "%1"=="release" (
    echo Building in Release mode...
    set BUILD_TYPE=Release
    set COMPILER_OPTS=-O3 -std=c++20 -v
    set OUTPUT_DIR=Build\ReleaseWasm
) else if "%1"=="debug" (
    echo Building in Debug mode...
    set BUILD_TYPE=Debug
    set COMPILER_OPTS=-O0 -g -std=c++20 -v
    set OUTPUT_DIR=Build\DebugWasm
) else (
    echo No build type specified, defaulting to Debug...
    set BUILD_TYPE=Debug
    set COMPILER_OPTS=-O0 -g -std=c++20 -v
    set OUTPUT_DIR=Build\DebugWasm
)

rem Include directories for SDL2, ImGui, and ImPlot
set INCLUDE_OPTS=-I Src -I %SDL2_PATH%\include -I %IMGUI_PATH%\include -I %IMGUI_PATH%\backends -I %IMGPLOT_PATH%\include

rem Create Build folder if it doesn't exist
if not exist Build (
    echo Creating Build folder...
    mkdir Build
)
if not exist %OUTPUT_DIR% (
    echo Creating %OUTPUT_DIR% folder...
    mkdir %OUTPUT_DIR%
)

rem Clean object files for release builds
if "%BUILD_TYPE%"=="Release" (
    echo Cleaning old object files for release build...
    del /Q %OUTPUT_DIR%\*.o %OUTPUT_DIR%\*.tmp %OUTPUT_DIR%\*.html.mem
)

rem Compile all source files into WebAssembly without multithreading support and package the Assets folder
echo Compiling project...
em++ %COMPILER_OPTS% %INCLUDE_OPTS% ^
    Src\Main.cpp ^
    -o %OUTPUT_DIR%\anyma.html ^
    -s USE_SDL=2 -s USE_WEBGL2=1 ^
    -s WASM_MEM_MAX=512MB -s ALLOW_MEMORY_GROWTH ^
    -s ALLOW_BLOCKING_ON_MAIN_THREAD=1 -s ASYNCIFY=1 ^
    --preload-file Assets@/Assets

if errorlevel 1 (
    echo Build failed.
    exit /b 1
) else (
    echo Build succeeded.
)

pause
