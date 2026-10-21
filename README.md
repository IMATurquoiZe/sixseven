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

## Jouer

Au lancement, un menu propose trois modes :

- **Solo** : contre l'IA.
- **Héberger une partie** : le jeu écoute sur le port TCP `53000` et affiche l'adresse à communiquer. Le joueur qui héberge est celui du bas ; la partie démarre dès qu'un adversaire se connecte.
- **Rejoindre une partie** : saisir `adresse` ou `adresse:port`, puis Entrée.

En réseau local, il suffit que l'hôte donne son adresse IP locale. Via Internet, l'hôte doit rediriger le port `53000` (TCP) de sa box vers sa machine. Pour passer au pli, à la manche suivante ou rejouer, un clic gauche dans le vide (ni sur une carte, ni sur le bouton) suffit. Échap revient au menu.

L'hôte détient seul l'état de la partie et valide les coups ; le client ne reçoit que ce qu'il a le droit de voir (sa main, pas celle de l'adversaire ni les points de manche avant la fin de la manche).
