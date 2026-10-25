@echo off
title DayZ - Zombie Virus Playtest Server
if exist "%~dp0tools\build.py" (
    cd /d "%~dp0"
) else (
    cd /d "C:\Users\tnapi\Desktop\dayz-zombie-virus-playable"
)

echo ===================================================================
echo             ZOMBIE VIRUS - PLAYABLE ZOMBIE PLAYTEST
echo ===================================================================
echo Working directory: %CD%
echo.
echo [1/3] Building latest mod PBO...
python tools\build.py
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed! Check the output above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [2/3] Starting dedicated playtest server on Chernarus...
echo       - 100%% infection chance from zombie hits
echo       - 3 minute incubation timer (quick testing)
echo       - Starting antiviral injector in inventory
echo       - Playable puppet zombie enabled upon turning
echo.
echo [3/3] Launching DayZ game client to 127.0.0.1:2302...
python tools\server_test.py --play --fresh --join

echo.
echo Server session ended.
pause
