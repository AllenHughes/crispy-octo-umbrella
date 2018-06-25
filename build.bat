@echo off

mkdir  C:\Users\AllenWorkstation\Repos\windows-window\build
pushd build\
cd
cl -FC -Zi ..\src\win32_handmade.cpp user32.lib gdi32.lib
popd
