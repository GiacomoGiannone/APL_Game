#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "    APL Game - Build Script per macOS    "
echo "=========================================="
echo ""

# ==========================================
# 1. VERIFICA PREREQUISITI
# ==========================================
echo -e "${YELLOW}[1/5] Verifica prerequisiti...${NC}"

MISSING=0

# CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED} CMake non trovato${NC}"
    echo "  Installa con: brew install cmake"
    MISSING=1
else
    echo -e "${GREEN} CMake trovato$(cmake --version | head -1 | sed 's/cmake version /: /')${NC}"
fi

# .NET SDK
if ! command -v dotnet &> /dev/null; then
    echo -e "${RED} .NET SDK non trovato${NC}"
    echo "  Scarica da: https://dotnet.microsoft.com/download/dotnet/9.0"
    echo "  Oppure con Homebrew: brew install dotnet"
    MISSING=1
else
    DOTNET_VERSION=$(dotnet --version)
    if [[ "$DOTNET_VERSION" < "9.0" ]]; then
        echo -e "${RED} .NET SDK versione $DOTNET_VERSION trovata, richiesta >= 9.0${NC}"
        MISSING=1
    else
        echo -e "${GREEN} .NET SDK: $DOTNET_VERSION${NC}"
    fi
fi

if [ $MISSING -eq 1 ]; then
    echo ""
    echo -e "${RED}Installa i prerequisiti mancanti e riprova.${NC}"
    exit 1
fi

# ==========================================
# 2. VERIFICA/INSTALLA WORKLOAD MAUI
# ==========================================
echo ""
echo -e "${YELLOW}[2/5] Verifica workload .NET MAUI...${NC}"

if ! dotnet workload list 2>/dev/null | grep -q "maui"; then
    echo "Installazione workload MAUI (potrebbe richiedere sudo)..."
    dotnet workload install maui --skip-sign-check || {
        echo -e "${YELLOW}Provo con sudo...${NC}"
        sudo dotnet workload install maui --skip-sign-check
    }
fi
echo -e "${GREEN}✓ Workload MAUI disponibile${NC}"

# ==========================================
# 3. CONFIGURA E COMPILA C++ (SFML)
# ==========================================
echo ""
echo -e "${YELLOW}[3/5] Configurazione SFML per C++...${NC}"

cd "$PROJECT_DIR"

# Esegui configure_mac.sh se SFML non è presente
if [ ! -d "Cpp/deps/SFML" ]; then
    echo "SFML non trovato, eseguo configure_mac.sh..."
    chmod +x configure_mac.sh
    ./configure_mac.sh
else
    echo -e "${GREEN}✓ SFML già configurato${NC}"
fi

echo ""
echo -e "${YELLOW}[4/5] Compilazione progetto C++...${NC}"

# Build C++
mkdir -p build_mac
cd build_mac

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=$(uname -m)
cmake --build . --config Release -j$(sysctl -n hw.ncpu)

if [ -f "APL_Game" ] || [ -f "Release/APL_Game" ]; then
    echo -e "${GREEN}✓ Progetto C++ compilato con successo${NC}"
else
    echo -e "${RED}✗ Errore nella compilazione C++${NC}"
    exit 1
fi

# Copia assets se necessario
if [ -d "$PROJECT_DIR/Cpp/assets" ]; then
    cp -R "$PROJECT_DIR/Cpp/assets" . 2>/dev/null || true
fi

# ==========================================
# 4. COMPILA C# (Dashboard MAUI)
# ==========================================
echo ""
echo -e "${YELLOW}[5/5] Compilazione Dashboard C# (.NET MAUI)...${NC}"

cd "$PROJECT_DIR/Cs/Dashboard"

dotnet restore
dotnet build -c Release -f net9.0-maccatalyst

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Dashboard C# compilata con successo${NC}"
else
    echo -e "${RED}✗ Errore nella compilazione C#${NC}"
    exit 1
fi

# ==========================================
# RIEPILOGO FINALE
# ==========================================
echo ""
echo "=========================================="
echo -e "${GREEN}        BUILD COMPLETATA CON SUCCESSO     ${NC}"
echo "=========================================="
echo ""
echo "Per eseguire il gioco C++:"
echo "  cd $PROJECT_DIR/build_mac"
echo "  ./APL_Game"
echo ""
echo "Per eseguire la Dashboard C#:"
echo "  cd $PROJECT_DIR/Cs/Dashboard"
echo "  dotnet run -f net9.0-maccatalyst"
echo ""
echo -e "${YELLOW}NOTA: Prima di eseguire, abilita i permessi in:${NC}"
echo "  Impostazioni > Privacy e Sicurezza > Monitoraggio Input > Terminale"
echo ""
