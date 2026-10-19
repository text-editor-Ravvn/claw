@echo off
setlocal
rem Build with an installed C compiler; choose cl.exe (MSVC) or gcc (MinGW-w64).
where cl >nul 2>nul
if %errorlevel%==0 goto msvc
where gcc >nul 2>nul
if %errorlevel%==0 goto mingw
echo No supported C compiler found.
echo Use a Visual Studio Developer Command Prompt for MSVC, or install MinGW-w64.
exit /b 1
:msvc
cl /nologo /std:c11 /O2 /W4 /Iinclude src\main.c src\terminal.c src\renderer.c /Fe:claw-splash.exe
if errorlevel 1 exit /b 1
del *.obj 2>nul
exit /b 0
:mingw
gcc -O2 -std=c11 -Wall -Wextra -Iinclude -o claw-splash.exe src\main.c src\terminal.c src\renderer.c
if errorlevel 1 exit /b 1
exit /b 0
