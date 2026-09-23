@echo off
setlocal
if not "%~1"=="" (
    echo Set the release version explicitly before building. See BUILDING.md.
    exit /b 1
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\Build-Distribution.ps1"
exit /b %errorlevel%
