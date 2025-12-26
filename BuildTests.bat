@echo off
rem Unity build test runner - Handmade Hero style

set SDL2_PATH=external\sdl2
set IMGUI_PATH=external\imgui
set IMGPLOT_PATH=external\implot
set OPENGL_PATH=external\opengl

rem Test build is always debug
set BUILD_TYPE=Debug
set COMPILER_OPTS=/EHsc /MT /Od /W3 /std:c++20 /Zi
set OUTPUT_DIR=Build\Test

rem Include directories
set INCLUDE_OPTS=/I Src /I %SDL2_PATH%\include /I %OPENGL_PATH% /I %IMGUI_PATH%\include /I %IMGUI_PATH%\backends /I %IMGPLOT_PATH%\include

rem Libraries to link
set LIB_OPTS=/link /LIBPATH:%SDL2_PATH%\lib SDL2.lib opengl32.lib user32.lib gdi32.lib shell32.lib

rem Create Build folder if it doesn't exist
if not exist Build mkdir Build
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

echo Building tests...
cl /Fo:%OUTPUT_DIR%\ /Fe:%OUTPUT_DIR%\tests.exe /Fd:%OUTPUT_DIR%\tests.pdb %COMPILER_OPTS% %INCLUDE_OPTS% Test\TestRunner.cpp %LIB_OPTS%

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Running tests...
    echo.
    %OUTPUT_DIR%\tests.exe
) else (
    echo Build failed!
    exit /b 1
)
