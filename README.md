# Geometry Modeling — Éditeur de maillages 3D 📐

> **Visualiseur et éditeur de maillages 3D en C++/OpenGL basé sur une structure half-edge : subdivision, simplification, manipulation de la géométrie.**

![C++](https://img.shields.io/badge/C++-14b8a6?style=flat-square)
![Type](https://img.shields.io/badge/ESIEE-555?style=flat-square)
[![Portfolio](https://img.shields.io/badge/Portfolio-afouanee.dev-14b8a6?style=flat-square)](https://afouanee.dev/projects/opengl-3d)

## ✨ Aperçu
Projet ESIEE de modélisation géométrique : un outil interactif de visualisation et de traitement de maillages 3D développé en C++ avec OpenGL. Le maillage est représenté par une structure de données **half-edge**, qui permet de naviguer efficacement entre sommets, arêtes et faces. L'application offre une palette d'opérations de géométrie algorithmique (subdivision, simplification, découpe) accessibles via un menu contextuel.

## 🚀 Fonctionnalités
- **Structure half-edge** : sommets, arêtes orientées et faces (`myVertex`, `myHalfedge`, `myFace`).
- **Subdivision** : Catmull-Clark et Loop.
- **Simplification** de maillage (`simplify()`) et triangulation (`triangulate()`).
- **Édition fine** : découpe et contraction d'arêtes et de faces (split / contraction).
- **Calculs géométriques** : `computeNormals()`, normalisation du maillage.
- **Rendu** : affichage des silhouettes, des arêtes vives (crease) et des normales.
- **Import / export** : ouverture et écriture de fichiers OBJ, sélection via une boîte de dialogue native (NFD), menu contextuel GLUT (clic droit).

## 🛠️ Stack technique
- **Langage** : C++
- **Bibliothèques / frameworks** : OpenGL, GLEW, FreeGLUT, GLM, NFD (Native File Dialog, intégré au dépôt)
- **Outils** : compilation manuelle (voir ci-dessous)

## ▶️ Lancer le projet
```text
Build : aucun fichier de build n'est commité (pas de CMake / Makefile / .sln).
La compilation nécessite de lier manuellement GLEW, FreeGLUT, GLM et NFD,
puis de compiler meshviewer-init/myproj/main.cpp et les sources myMesh / my*.
```

## 📂 Structure
```
meshviewer-init/myproj/
├── main.cpp           # point d'entrée + menu GLUT
├── myMesh.{h,cpp}     # maillage half-edge + opérations (subdivision, simplify, ...)
├── myHalfedge / myVertex / myFace   # primitives de la structure half-edge
├── myPoint3D / myVector3D           # géométrie 3D
└── helperFunctions.h  # fonctions utilitaires
```

---
🔗 **Fiche projet** : [afouanee.dev/projects/opengl-3d](https://afouanee.dev/projects/opengl-3d)
👤 **Auteur** : Afouane MOUHAMAD — [Portfolio](https://afouanee.dev) · [LinkedIn](https://linkedin.com/in/afouane-mouhamad)
