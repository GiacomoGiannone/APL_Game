@echo off
echo Configurazione progetto Windows...
echo.

set PROJECT_DIR=%~dp0
set SFML_PATH=C:\SFML-2.5.1

echo Verifico SFML...
if not exist "%SFML_PATH%\include\SFML\Graphics.hpp" (
    echo ERRORE: SFML non trovato in %SFML_PATH%
    echo.
    echo Scarica SFML 2.5.1 da:
    echo https://www.sfml-dev.org/download/sfml/2.5.1/
    echo.
    echo Estrai in C:\SFML-2.5.1
    pause
    exit /b 1
)

echo SFML trovato!
echo.

cd /d "%PROJECT_DIR%"
if exist build rmdir /s /q build
mkdir build
cd build

echo Configuro CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DSFML_ROOT="%SFML_PATH%"

if %errorlevel% equ 0 (
    echo.
    echo Configurazione completata!
    echo.
    echo Per compilare esegui:
    echo cmake --build . --config Release
) else (
    echo.
    echo ERRORE nella configurazione!
)

pause