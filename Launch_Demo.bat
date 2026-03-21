@echo off
echo Starting B.Tech Security Demo Initialization...
echo.

:: 1. Launch the App first
echo [1/3] Launching Target Environment...
start "Shipwreck Environment" cmd /k "app.exe"

:: 2. Wait 3 full seconds to guarantee the app has loaded its memory state
echo Waiting for Memory Allocation...
timeout /t 3 /nobreak >nul

:: 3. Launch the Watchdog
echo [2/3] Launching Watchdog Monitor...
start "Watchdog Monitor" cmd /k "watchdog.exe"

:: Wait 1 second before opening the hack tool
timeout /t 1 /nobreak >nul

:: 4. Launch the Hack
echo [3/3] Launching State Injector...
start "State Injector" cmd /k "hack.exe"

exit