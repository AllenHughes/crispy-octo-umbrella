@echo off

IF NOT EXIST C:\Users\AllenWorkstation\Repos\windows-window\build mkdir C:\Users\AllenWorkstation\Repos\windows-window\build
pushd build\
cl -MT -nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Fmwin32_handmade.map ..\src\win32_handmade.cpp /link -subsystem:windows,5.1 user32.lib gdi32.lib
popd
