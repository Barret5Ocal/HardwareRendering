@echo off 

IF NOT EXIST ..\..\build mkdir ..\..\build

pushd ..\..\build 

REM  d3dx10.lib

SET Linkers=user32.lib gdi32.lib winmm.lib opengl32.lib d3d11.lib "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64\d3dx11.lib" "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64\d3dx10.lib" 

cl -Z7 -nologo ..\project\code\win32_layer.cpp -DOPENGL=0 -DVULKAN=0 -DDIRECTX=1 %Linkers% 

popd 