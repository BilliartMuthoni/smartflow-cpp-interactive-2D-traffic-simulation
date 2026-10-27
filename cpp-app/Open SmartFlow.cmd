@echo off
cd /d "%~dp0"
if not exist "bin\SmartFlow.exe" (
  echo Please run Build SmartFlow.cmd first.
  pause
  exit /b 1
)
start "" "bin\SmartFlow.exe"
