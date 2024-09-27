@echo off
set SDL2_PATH=external\sdl2
set OPENGL_PATH=external\opengl
set IMGUI_PATH=external\imgui
set IMGPLOT_PATH=external\implot

rem Compiler options
set COMPILER_OPTS=/EHsc /MT /Od /W3 /std:c++20 /Zi
set DEBUG_DIR=Build\Debug

rem Include directories
set INCLUDE_OPTS=/I %SDL2_PATH%\include /I %OPENGL_PATH% /I %IMGUI_PATH%\include /I %IMGUI_PATH%\backends /I %IMGPLOT_PATH%\include

rem Libraries to link
set LIB_OPTS=/link /LIBPATH:%SDL2_PATH%\lib SDL2.lib opengl32.lib user32.lib gdi32.lib shell32.lib

rem Create Build folder if it doesn't exist
if not exist Build mkdir Build
if not exist Build\Debug mkdir Build\Debug

rem Compile gl3w.cpp and main.cpp into object files and output to build folder
cl /Fo:%DEBUG_DIR%\ /Fe:%DEBUG_DIR%\anyma.exe /Fd:%DEBUG_DIR%\anyma.pdb %COMPILER_OPTS% %INCLUDE_OPTS% Src\Main.cpp %OPENGL_PATH%\gl3w.cpp %LIB_OPTS%

rem Copy Assets folder to build directory
xcopy /E /I /Y Assets %DEBUG_DIR%\Assets

copy /Y %SDL2_PATH%\lib\SDL2.dll %DEBUG_DIR%\

pause
