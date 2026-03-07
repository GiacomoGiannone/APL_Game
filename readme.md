# APL Game

## Se ti trovi su macOs
    Trovi tutti gli eseguibili nella cartella build_mac
## Se ti trovi su Windows
    Trovi tutti gli eseguibili nella cartella buildWindows

### Ti basta allora clonare il repo e avviare gli eseguibili nelle cartelle in base alla piattaforma in cui ti trovi

## Se, invece, vuoi compilare: 

## Compilazione su macOS

### Prerequisiti
- **CMake**: `brew install cmake`
- **.NET 9 SDK**: scarica da https://dotnet.microsoft.com/download/dotnet/9.0 oppure `brew install dotnet`

### Build automatica
Esegui lo script dalla cartella del progetto:
```bash
chmod +x build_mac.sh
./build_mac.sh
```
Lo script:
1. Verifica i prerequisiti
2. Installa il workload MAUI se mancante
3. Scarica e configura SFML
4. Compila il gioco C++ e la Dashboard C#

### Esecuzione
**Gioco (C++):**
```bash
cd build_mac
./APL_Game
```

**Dashboard (C#):**
```bash
cd Cs/Dashboard
dotnet run -f net9.0-maccatalyst
```

### Nota importante
Prima di eseguire, abilita i permessi per il Terminale:
1. Apri **Impostazioni di sistema**
2. Vai su **Privacy e Sicurezza**
3. Seleziona **Monitoraggio Input**
4. Abilita i permessi per **Terminale**

Senza questo permesso, l'applicazione potrebbe non ricevere correttamente l'input da tastiera.

---

## Compilazione su Windows

### Prerequisiti
- **Visual Studio 2022** con workload "Sviluppo di applicazioni desktop con C++"
- **.NET 9 SDK**

### Build
1. Esegui `configure_windows.bat` per scaricare SFML
2. Apri `build/APL_Game.sln` con Visual Studio
3. Compila in modalità Release

Per la Dashboard C#, apri `Cs/Dashboard/Dashboard.sln` con Visual Studio.