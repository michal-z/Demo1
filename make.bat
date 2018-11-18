@echo off
setlocal
setlocal enableextensions

set NAME=SimplePbr
if "%1" == "clean" if exist External.pch del External.pch

set FINAL=/O2 /DNDEBUG /MT
set RELEASE=/Zi /O2 /DNDEBUG /MT
set DEBUG=/Zi /Od /D_DEBUG /MTd
set DEBUGNOPDB=/Od /D_DEBUG /MTd
set LFLAGS=/incremental:no /opt:ref /machine:x64
set HLSLC=fxc.exe /Ges /O3 /WX /nologo /Qstrip_reflect /Qstrip_debug /Qstrip_priv

if not defined CONFIG set CONFIG=%DEBUGNOPDB%
::/d2cgsummary
set CFLAGS=%CONFIG% /EHa- /GR- /Gy /Gw /W3 /nologo /I"External" /Bt

set ERROR=0

set CSODIR=Data\Shaders
if exist %CSODIR%\*.cso del %CSODIR%\*.cso
if exist %NAME%.exe del %NAME%.exe

%HLSLC% /D VS_IMGUI /E VertexMain /Fo %CSODIR%\Gui.vs.cso /T vs_5_1 %NAME%.hlsl & if ERRORLEVEL 1 (set ERROR=1 & goto :end)
%HLSLC% /D PS_IMGUI /E PixelMain /Fo %CSODIR%\Gui.ps.cso /T ps_5_1 %NAME%.hlsl & if ERRORLEVEL 1 (set ERROR=1 & goto :end)
%HLSLC% /D VS_DISPLAY_CANVAS /E VertexMain /Fo %CSODIR%\DisplayCanvas.vs.cso /T vs_5_1 %NAME%.hlsl & if ERRORLEVEL 1 (set ERROR=1 & goto :end)
%HLSLC% /D PS_DISPLAY_CANVAS /E PixelMain /Fo %CSODIR%\DisplayCanvas.ps.cso /T ps_5_1 %NAME%.hlsl & if ERRORLEVEL 1 (set ERROR=1 & goto :end)
%HLSLC% /D VS_SAND /E VertexMain /Fo %CSODIR%\Sand.vs.cso /T vs_5_1 %NAME%.hlsl & if ERRORLEVEL 1 (set ERROR=1 & goto :end)
%HLSLC% /D PS_SAND /E PixelMain /Fo %CSODIR%\Sand.ps.cso /T ps_5_1 %NAME%.hlsl & if ERRORLEVEL 1 (set ERROR=1 & goto :end)

if not exist External.pch (cl %CFLAGS% /c /YcExternal.h External.cpp)
cl %CFLAGS% /YuExternal.h Main.cpp /link %LFLAGS% External.obj kernel32.lib user32.lib gdi32.lib /out:%NAME%.exe
if ERRORLEVEL 1 (set ERROR=1)
if exist Main.obj del Main.obj
if "%1" == "run" if exist %NAME%.exe %NAME%.exe

:end
exit /b %ERROR%
