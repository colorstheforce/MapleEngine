@echo off
if not exist build\win_x64 (
    mkdir build\win_x64
)
cd build\win_x64
cmake ..\..\app  -DArch=x64 -DTarget=Windows 
cd ..\..\
pause