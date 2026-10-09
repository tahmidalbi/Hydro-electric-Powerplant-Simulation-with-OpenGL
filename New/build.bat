@echo off
rem ===========================================================================
rem  build.bat - compiles the project with the MSYS2 MinGW-w64 g++ compiler.
rem  Result: build\hydro.exe  (shaders are copied next to it)
rem ===========================================================================
setlocal
set PATH=C:\msys64\mingw64\bin;%PATH%
if not exist build mkdir build

echo Compiling ...
g++ -std=c++17 -O2 -Iinclude -Isrc ^
    src\main.cpp src\App.cpp src\glad.c ^
    src\core\*.cpp src\render\*.cpp src\objects\*.cpp src\scenes\*.cpp src\ui\*.cpp ^
    -o build\hydro.exe -lglfw3 -lopengl32 -lgdi32
if errorlevel 1 (
    echo.
    echo BUILD FAILED
    exit /b 1
)
xcopy /E /I /Y /Q shaders build\shaders >nul
echo Done: build\hydro.exe
endlocal
