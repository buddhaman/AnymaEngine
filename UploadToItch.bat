@echo off
SET ProjectName=AnymaEngine
SET ItchUser=buddhaman
SET ItchGame=anyma-engine
SET Channel=win64-release
SET BuildDir=build/Release
SET ZipName=build/%ProjectName%_%Channel%.zip

REM Change to the script's directory
cd /d %~dp0

REM Zip the build directory using PowerShell
echo Zipping the build directory...
PowerShell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%BuildDir%\*' -DestinationPath '%ZipName%'"

REM Push the zip file to itch.io using butler
echo Uploading the build to itch.io...
butler push %ZipName% %ItchUser%/%ItchGame%:%Channel%

echo Upload complete!
pause
