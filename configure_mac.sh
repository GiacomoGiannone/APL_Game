#!/bin/bash
echo "Configurazione progetto macOS..."

# Verifica Homebrew e SFML
if ! command -v brew &> /dev/null; then
    echo "Homebrew non trovato!"
    echo "Installa da: https://brew.sh"
    exit 1
fi

if ! brew list sfml &> /dev/null; then
    echo "SFML non trovato!"
    echo "Installa con: brew install sfml"
    exit 1
fi

echo "SFML trovato tramite Homebrew"

# Configura CMake
cd "$(dirname "$0")"
rm -rf build
mkdir build
cd build

echo "Configuro CMake..."
cmake .. -G "Unix Makefiles"

if [ $? -eq 0 ]; then
    echo ""
    echo "Configurazione completata!"
    echo ""
    echo "Per compilare:"
    echo "make -j4"
else
    echo ""
    echo "ERRORE nella configurazione!"
fi