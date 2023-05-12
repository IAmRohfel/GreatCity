@echo off

pushd %~dp0\..\
call premake5.exe --file=GreatCity.build.lua vs2022
popd

pause