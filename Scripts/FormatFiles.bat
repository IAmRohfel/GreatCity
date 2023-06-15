@echo off

pushd %~dp0\
call powershell.exe -ExecutionPolicy Bypass -File FormatFiles.ps1
popd

pause