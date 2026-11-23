@echo off
setlocal
echo ============================================================
echo  Extasis Marimba - Windows Automated Installer
echo ============================================================

set "INSTALL_DIR=%~dp0"
set "VST3_DIR=%COMMONPROGRAMFILES%\VST3"

echo [1/1] Installing VST3 Plugin to %VST3_DIR%...
if not exist "%VST3_DIR%" mkdir "%VST3_DIR%"
if exist "%INSTALL_DIR%Extasis Marimba.vst3" (
    xcopy /E /I /Y /Q "%INSTALL_DIR%Extasis Marimba.vst3" "%VST3_DIR%\Extasis Marimba.vst3"
) else if exist "%INSTALL_DIR%..\Extasis Marimba.vst3" (
    xcopy /E /I /Y /Q "%INSTALL_DIR%..\Extasis Marimba.vst3" "%VST3_DIR%\Extasis Marimba.vst3"
)

echo.
echo ============================================================
echo  Installation Complete! Open your DAW and rescan plugins.
echo ============================================================
pause
