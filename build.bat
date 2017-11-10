@echo off

mkdir  C:\Users\AllenWorkstation\Repos\windows-window\build
pushd build\
cl -FC -Zi ..\win32_test.cpp user32.lib gdi32.lib
popd
