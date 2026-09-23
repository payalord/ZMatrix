@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\Build-Distribution.ps1" -SkipBuild
exit /b %errorlevel%
