@echo off
set SDL2_PATH=external\sdl2
set IMGUI_PATH=external\imgui
set IMGPLOT_PATH=external\implot
set OPENGL_PATH=external\opengl

rem Check for release or debug argument
if "%1"=="release" (
    set BUILD_TYPE=Release
    set COMPILER_OPTS=/O2 /MT /W3 /std:c++20
    set OUTPUT_DIR=Build\Release
) else (
    set BUILD_TYPE=Debug
    set COMPILER_OPTS=/EHsc /MT /Od /W3 /std:c++20 /Zi
    set OUTPUT_DIR=Build\Debug
)

rem Include directories
set INCLUDE_OPTS=/I %SDL2_PATH%\include /I %OPENGL_PATH% /I %IMGUI_PATH%\include /I %IMGUI_PATH%\backends /I %IMGPLOT_PATH%\include

rem Libraries to link
set LIB_OPTS=/link /LIBPATH:%SDL2_PATH%\lib SDL2.lib opengl32.lib user32.lib gdi32.lib shell32.lib

rem Create Build folder if it doesn't exist
if not exist Build mkdir Build
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

rem Clean object files for release builds
if "%BUILD_TYPE%"=="Release" (
    del /Q %OUTPUT_DIR%\*.obj
)

rem Compile and link Anyma.cpp into object files and output to build folder
cl /Fo:%OUTPUT_DIR%\ /Fe:%OUTPUT_DIR%\anyma.exe /Fd:%OUTPUT_DIR%\anyma.pdb %COMPILER_OPTS% %INCLUDE_OPTS% Src\Anyma.cpp %LIB_OPTS%

rem Copy Assets folder to output directory
xcopy /E /I /Y Assets %OUTPUT_DIR%\Assets

rem Copy SDL2.dll to output directory
copy /Y %SDL2_PATH%\lib\SDL2.dll %OUTPUT_DIR%\
