# Geometry Modeling — Éditeur de maillages 3D 📐

> **Visualiseur et éditeur de maillages 3D en C++/OpenGL basé sur une structure half-edge : subdivision, simplification, manipulation de la géométrie.**

![C++](https://img.shields.io/badge/C++-14b8a6?style=flat-square)
![Type](https://img.shields.io/badge/ESIEE-555?style=flat-square)
[![Portfolio](https://img.shields.io/badge/Portfolio-afouanee.dev-14b8a6?style=flat-square)](https://afouanee.dev/projects/opengl-3d)

## ✨ Aperçu
Projet ESIEE de modélisation géométrique (cours *Geometry Modelling*, énoncés TD inclus dans `TD/`) : un outil interactif de visualisation et de traitement de maillages 3D développé en C++ avec OpenGL. Le maillage est représenté par une structure de données **half-edge**, qui permet de naviguer efficacement entre sommets, arêtes et faces. L'application offre une palette d'opérations de géométrie algorithmique (subdivision, simplification) accessibles via un menu contextuel GLUT.

## 🚀 Fonctionnalités
- **Structure half-edge** : sommets, arêtes orientées et faces (`myVertex`, `myHalfedge`, `myFace`), reconstruction des twins (`buildTwins()`) et vérification de cohérence du maillage (`checkMesh()`).
- **Subdivision de Catmull-Clark** (`subdivisionCatmullClark()`) : implémentation complète (face points / edge points / vertex points) qui reconstruit un nouveau maillage half-edge à chaque passage.
- **Simplification** de maillage (`simplify()`) par effondrement itératif de l'arête la plus courte, avec nettoyage des faces devenues dégénérées.
- **Triangulation** (`triangulate()`).
- **Édition fine** : découpe d'arêtes/faces (`splitEdge`, `splitFaceTRIS`, `splitFaceQUADS`).
- **Calculs géométriques** : `computeNormals()`, normalisation du maillage (`normalize()`).
- **Rendu** : affichage du maillage plein/filaire, des sommets, des silhouettes et des normales (shading Gouraud/plat via shaders GLSL).
- **Import** : ouverture de fichiers OBJ via une boîte de dialogue native (NFD), menu contextuel GLUT (clic droit).

> ℹ️ **Loop subdivision** : une entrée de menu *"Loop subdivision"* (`MENU_LOOP`) existe dans `helperFunctions.h`, mais aucun `case MENU_LOOP` ne la traite dans `main.cpp` et aucune fonction `subdivisionLoop`/équivalente n'existe dans `myMesh`. À ce stade, seule la subdivision de Catmull-Clark est réellement implémentée ; Loop reste un point d'entrée UI non câblé (comme plusieurs autres entrées du menu : *Inflate/Smoothen* pour Smoothen, *Contract edge/face*, *Undo*, *Write to File*, *Generate/Cut Mesh*, *Crease*, *Select Face*).

## 🛠️ Stack technique
- **Langage** : C++ (Visual Studio / MSVC — `myHalfedge.h` inclut `<tchar.h>`, spécifique à Windows).
- **Bibliothèques** : OpenGL, [GLEW](http://glew.sourceforge.net/), [FreeGLUT](http://freeglut.sourceforge.net/), [GLM](https://github.com/g-truc/glm), [NFD (Native File Dialog)](https://github.com/btzy/nativefiledialog-extended).
- **Shaders** : le code référence `shaders/light.vert.glsl` et `shaders/light.frag.glsl` (chargés au runtime), qui **ne sont pas présents dans ce dépôt** — à recréer/récupérer pour que le rendu fonctionne.

## ▶️ Compiler le projet
Aucun fichier de build n'est commité (pas de `.sln`/`.vcxproj`, pas de `CMakeLists.txt`, pas de `Makefile`). Le seul indice de toolchain est l'usage de `<tchar.h>`, qui suggère un projet Visual Studio sous Windows. Pour compiler ce code il faut donc recréer un projet et lier manuellement les dépendances.

### Windows (Visual Studio + vcpkg, recommandé)
```powershell
# Installer les dépendances via vcpkg
vcpkg install glew freeglut glm nativefiledialog-extended --triplet x64-windows

# Créer un projet Visual Studio (Application console ou Win32) intégrant vcpkg
# (vcpkg integrate install), puis ajouter au projet tous les fichiers de
# meshviewer-init/myproj/ (main.cpp, myMesh.cpp, myFace.cpp, myHalfedge.cpp,
# myPoint3D.cpp, myVector3D.cpp, myVertex.cpp) et leurs headers.
#
# Ajouter les libs au linker : opengl32.lib, glew32.lib, freeglut.lib, nfd.lib
# Créer un dossier shaders/ à côté de l'exécutable avec light.vert.glsl et light.frag.glsl
```
Si le paquet vcpkg s'appelle `nativefiledialog-extended`, adapter les `#include "NFD/nfd.h"` du code aux headers réellement installés (l'API historique de NFD a changé de nom entre versions).

### Linux (à titre indicatif — code non testé hors Windows)
```bash
sudo apt install libglew-dev freeglut3-dev libglm-dev
# NFD n'a pas de paquet standard : cloner et compiler
# https://github.com/btzy/nativefiledialog-extended (ou l'original mlabbe/nativefiledialog)
g++ meshviewer-init/myproj/*.cpp -o meshviewer \
    -lGLEW -lglut -lGL -lGLU -lnfd $(pkg-config --cflags --libs gtk+-3.0)
```
Le code utilise `<tchar.h>` (Windows uniquement) dans `myHalfedge.h` : ce fichier doit être adapté (retiré ou remplacé) pour compiler sous Linux/macOS.

### Étapes communes à toute plateforme
1. Récupérer/écrire `shaders/light.vert.glsl` et `shaders/light.frag.glsl` (référencés par `helperFunctions.h` mais absents du dépôt).
2. Vérifier que les headers NFD utilisés (`#include "NFD/nfd.h"`) correspondent à la version de la bibliothèque installée.
3. Lancer l'exécutable puis clic droit dans la fenêtre pour accéder au menu contextuel (import OBJ, subdivision, simplification, affichage).

## 📂 Structure
```
meshviewer-init/myproj/
├── main.cpp              # point d'entrée, boucle GLUT, menu contextuel (clic droit)
├── helperFunctions.h     # setup OpenGL/shaders, buffers, construction du menu GLUT
├── myMesh.{h,cpp}        # maillage half-edge + algorithmes (Catmull-Clark, simplify, triangulate, split, checkMesh, buildTwins)
├── myHalfedge.{h,cpp}    # demi-arête (source, face adjacente, next/prev/twin)
├── myVertex.{h,cpp}      # sommet (position, normale, half-edge d'origine)
├── myFace.{h,cpp}        # face (half-edge adjacente, normale)
├── myPoint3D.{h,cpp}     # point 3D (opérations géométriques de base)
└── myVector3D.{h,cpp}    # vecteur 3D

TD/                        # énoncés de TD du cours (PDF)
Capture d'ecran/            # captures et vidéos de démonstration (subdivision, simplification, triangulation, normales)
```
