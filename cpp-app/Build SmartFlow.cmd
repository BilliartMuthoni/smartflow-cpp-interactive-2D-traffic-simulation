@echo off
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "scripts\build.ps1"
if errorlevel 1 (
  echo Build failed. Read the message above.
  pause
  exit /b 1
)
echo Build complete. Open SmartFlow.cmd launches the application.
pause
