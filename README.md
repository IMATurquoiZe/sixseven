# sixseven

Jeu de cartes en C++17 / SFML 2.x.

## Compilation

### Linux

```sh
sudo apt install libsfml-dev cmake g++   # ou l'équivalent de votre distribution
cmake -S . -B build
cmake --build build
./build/sixseven
```

### Windows

Prérequis (à installer, par exemple `winget install Kitware.CMake` puis Visual Studio Build Tools) : CMake et un compilateur (Visual Studio avec « Développement Desktop en C++ », ou MinGW).
SFML n'a pas besoin d'être installé : s'il est introuvable, CMake le télécharge et le compile en statique (Git requis).

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\sixseven.exe     # MinGW : .\build\sixseven.exe
```

Les commandes se lancent depuis le dossier du projet. Le dossier `assets` est copié à côté de l'exécutable à la compilation, et le jeu peut être lancé depuis n'importe quel dossier.
