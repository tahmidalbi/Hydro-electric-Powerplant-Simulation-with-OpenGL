@echo off
rem run.bat - starts the program (builds it first if needed)
setlocal
set PATH=C:\msys64\mingw64\bin;%PATH%
if not exist build\hydro.exe call build.bat
xcopy /E /I /Y /Q shaders build\shaders >nul
start "" build\hydro.exe %*
endlocal
