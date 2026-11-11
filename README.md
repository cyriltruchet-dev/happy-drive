# Happy Drive Preamps

Plugin VST3 / AU / Standalone : préampli de saturation harmonique, esprit console API.

## Paramètres
| Potard | Rôle |
|---|---|
| Entrée / Sortie | ±12 dB |
| Drive harmonique | quantité de saturation (0 % = signal propre) |
| Paires | niveau des harmoniques H2, H4, H6… (chaleur) |
| Impaires | niveau des harmoniques H3, H5, H7… (mordant) |
| Mix | dose d'harmoniques ajoutée au signal d'origine |
| Bypass | contournement (latence conservée) |

## Compiler
Prérequis : CMake ≥ 3.22, un compilateur C++17 (Xcode, Visual Studio 2022 ou GCC/Clang), Git.

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

JUCE 8.0.4 est téléchargé automatiquement. Le VST3 est copié dans le dossier système
des plugins à la fin de la compilation (AU sur macOS aussi).

## DSP
Entrée → sur-échantillonnage 4x (FIR, latence annoncée à l'hôte) →
signal propre + [harmoniques impaires tanh + harmoniques paires x²] → passe-haut 25 Hz → Mix →
sous-échantillonnage → Sortie. Les réglages sont lissés pour éviter les craquements.
