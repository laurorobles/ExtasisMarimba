@echo off
echo =======================================================
echo    Extasis Marimba - Windows Installer
echo =======================================================

set SCRIPT_DIR=%~dp0
set VST3_DIR=%COMMONPROGRAMFILES%\VST3
set PRESETS_DIR=%USERPROFILE%\Documents\ExtasisRecords\ExtasisMarimba\Presets

if not exist "%VST3_DIR%" mkdir "%VST3_DIR%"
if not exist "%PRESETS_DIR%" mkdir "%PRESETS_DIR%"

if exist "%SCRIPT_DIR%Extasis Marimba.vst3" (
    echo Installing Extasis Marimba.vst3...
    xcopy /E /I /Y "%SCRIPT_DIR%Extasis Marimba.vst3" "%VST3_DIR%\Extasis Marimba.vst3"
)

echo.
echo Extasis Marimba successfully installed on Windows!
pause
