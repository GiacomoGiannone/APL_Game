#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEPS_DIR="$PROJECT_DIR/deps"
SFML_VERSION="2.6.1"

echo "=== Configurazione Mac (Fix Frameworks + Architecture) ==="

# 1. Rileva Architettura
ARCH_NAME=$(uname -m)
if [ "$ARCH_NAME" == "arm64" ]; then
    echo "Rilevato Mac Apple Silicon (arm64)"
    FILE_NAME="SFML-${SFML_VERSION}-macOS-clang-arm64.tar.gz"
    FOLDER_NAME="SFML-${SFML_VERSION}-macOS-clang-arm64"
    CMAKE_ARCH="arm64"
else
    echo "Rilevato Mac Intel (x86_64)"
    FILE_NAME="SFML-${SFML_VERSION}-macOS-clang.tar.gz"
    FOLDER_NAME="SFML-${SFML_VERSION}-macOS-clang"
    CMAKE_ARCH="x86_64"
fi
DOWNLOAD_URL="https://github.com/SFML/SFML/releases/download/${SFML_VERSION}/${FILE_NAME}"

# 2. Verifica CMake
if ! command -v cmake &> /dev/null; then
    echo "CMake non trovato. Installalo con: brew install cmake"
    exit 1
fi

# 3. Pulizia Totale e Download
mkdir -p "$DEPS_DIR"
cd "$DEPS_DIR"

if [ -d "SFML" ]; then
    echo "Rimuovo vecchia installazione per applicare fix Frameworks..."
    rm -rf "SFML"
fi

echo "Scaricando SFML..."
curl -L -O "$DOWNLOAD_URL"
tar -xzf "$FILE_NAME"
rm "$FILE_NAME"
mv "$FOLDER_NAME" "SFML"

# ========================================================
# FIX 1: Rinomina extlibs in Frameworks (Cruciale per il tuo errore)
# ========================================================
if [ -d "SFML/extlibs" ]; then
    echo "Fixing cartella Frameworks..."
    mv "SFML/extlibs" "SFML/Frameworks"
    
    # Copia di sicurezza: alcuni dylib cercano i framework dentro lib/
    cp -R "SFML/Frameworks/"* "SFML/lib/"
fi

# ========================================================
# FIX 2: Rimuovi Quarantena Apple (Evita che macOS blocchi i file)
# ========================================================
echo "Rimozione blocco sicurezza Apple (Quarantine)..."
# Usa || true per non bloccare lo script se non trova attributi
xattr -r -d com.apple.quarantine "SFML" || true

# ========================================================
# FIX 3: Patch C++ per Xcode 16 (Come prima)
# ========================================================
echo "Applicazione patch C++..."
TARGET_FILE="$DEPS_DIR/SFML/include/SFML/System/String.hpp"
cat <<EOF > sfml_patch.tmp
#include <SFML/System/Export.hpp>
// --- PATCH START ---
#include <string>
#include <cstring>
#include <ios>
namespace std {
  template<> struct char_traits<unsigned int> {
    typedef unsigned int char_type;
    typedef unsigned int int_type;
    typedef std::streamoff off_type;
    typedef std::streampos pos_type;
    typedef std::mbstate_t state_type;
    static void assign(char_type& a, const char_type& c) { a = c; }
    static char_type* assign(char_type* s, size_t n, char_type a) { for(size_t i=0; i<n; ++i) s[i]=a; return s; }
    static bool eq(const char_type& c1, const char_type& c2) { return c1 == c2; }
    static bool lt(const char_type& c1, const char_type& c2) { return c1 < c2; }
    static int compare(const char_type* s1, const char_type* s2, size_t n) { 
        for(size_t i=0; i<n; ++i) { if(s1[i]<s2[i]) return -1; if(s1[i]>s2[i]) return 1; } return 0; 
    }
    static size_t length(const char_type* s) { size_t i=0; while(s[i]!=0) ++i; return i; }
    static const char_type* find(const char_type* s, size_t n, const char_type& a) { 
        for(size_t i=0; i<n; ++i) if(s[i]==a) return s+i; return nullptr; 
    }
    static char_type* move(char_type* s1, const char_type* s2, size_t n) { return (char_type*)std::memmove(s1, s2, n*sizeof(char_type)); }
    static char_type* copy(char_type* s1, const char_type* s2, size_t n) { return (char_type*)std::memcpy(s1, s2, n*sizeof(char_type)); }
    static int_type not_eof(const int_type& c) { return (c==static_cast<int_type>(-1))?0:c; }
    static char_type to_char_type(const int_type& c) { return c; }
    static int_type to_int_type(const char_type& c) { return c; }
    static bool eq_int_type(const int_type& c1, const int_type& c2) { return c1==c2; }
    static int_type eof() { return static_cast<int_type>(-1); }
  };
}
// --- PATCH END ---
EOF
perl -i -ne 'BEGIN { local $/; open(F, "sfml_patch.tmp"); $p = <F>; close(F); } s|#include <SFML/System/Export.hpp>|$p|g; print;' "$TARGET_FILE"
rm sfml_patch.tmp

# 5. Configura e Compila
cd "$PROJECT_DIR"
rm -rf build
mkdir build
cd build

echo "Eseguendo CMake..."
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES=$CMAKE_ARCH

echo ""
echo "=== PRONTO! ==="
echo "Per avviare:"
echo "cd build"
echo "make -j4"
echo "./APL_Game"