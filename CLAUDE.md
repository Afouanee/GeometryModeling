# CLAUDE.md

Contexte projet pour Claude Code sur ce dépôt.

## Quoi
Projet ESIEE (cours *Geometry Modelling*, cf. `TD/`) : éditeur/visualiseur de maillages 3D en C++/OpenGL, structure de données **half-edge**. Menu contextuel GLUT (clic droit) pour import OBJ, subdivision, simplification, affichage (wireframe, normales, silhouettes).

Tout le code source vit dans `meshviewer-init/myproj/`.

## Ne pas essayer de compiler
Pas de système de build committé (pas de `.sln`/`.vcxproj`, `CMakeLists.txt` ni `Makefile`). Dépendances externes non vendorisées : GLEW, FreeGLUT, GLM, NFD. Les shaders `shaders/light.vert.glsl` / `shaders/light.frag.glsl` référencés dans `helperFunctions.h` sont absents du dépôt. Voir le README pour la démarche de reconstruction d'un projet de build (vcpkg conseillé sous Windows). Ne pas tenter de build dans cet environnement.

## Structure du code
| Fichier | Rôle |
|---|---|
| `main.cpp` | Point d'entrée, boucle GLUT (`glutMainLoop`), callback `menu()` qui traite les entrées du menu contextuel, callbacks souris/clavier, setup caméra/projection. |
| `helperFunctions.h` | Setup OpenGL (contexte, shaders, VAO/VBO), construction des buffers de rendu (`makeBuffers`), construction du menu GLUT (`glutCreateMenu`/`glutAddMenuEntry`), chargement des shaders GLSL. |
| `myMesh.{h,cpp}` | Classe centrale `myMesh` : conteneurs `vertices`/`halfedges`/`faces`, algorithmes de traitement de maillage (voir section Algorithmes). |
| `myHalfedge.{h,cpp}` | Demi-arête : `source` (sommet), `adjacent_face`, `next`, `prev`, `twin`. |
| `myVertex.{h,cpp}` | Sommet : `point` (position), `originof` (demi-arête d'origine), `normal`. |
| `myFace.{h,cpp}` | Face : `adjacent_halfedge`, `normal`. |
| `myPoint3D.{h,cpp}` / `myVector3D.{h,cpp}` | Primitives géométriques (point, vecteur), opérateurs arithmétiques. |

## Algorithmes implémentés (vérifiés dans le code)

- **Catmull-Clark** — `myMesh::subdivisionCatmullClark()` dans `myMesh.cpp` (~ligne 260-425). Implémentation complète : calcule les face points (moyenne des sommets de chaque face), les edge points (moyenne pondérée avec les face points adjacents, ou milieu si arête de bord), les vertex points (formule classique F/2k + R/2k + P(k-3)/k), puis reconstruit un maillage half-edge entièrement neuf (`myMesh newMesh`) avec `buildTwins()` pour retisser les jumelles. Remplace `*this` par le nouveau maillage en fin de fonction.
- **Loop subdivision — NON implémentée.** Une entrée de menu `"Loop subdivision"` / `MENU_LOOP` existe dans `helperFunctions.h` (menu GLUT) et dans l'enum `MENU` de `main.cpp`, mais **aucun `case MENU_LOOP` ne la traite** dans le `switch` de `menu()` (`main.cpp`), et il n'existe aucune fonction `subdivisionLoop` (ni équivalent) dans `myMesh`. Cliquer sur cette entrée de menu ne fait rien. Si Loop doit être ajoutée un jour : suivre le même schéma que Catmull-Clark (nouveau maillage half-edge reconstruit), avec les poids de sommet/arête propres au schéma triangulaire de Loop, et l'entrée switch manquante dans `main.cpp`.
- **Simplification half-edge** — `myMesh::simplify()` dans `myMesh.cpp` (~ligne 527-665). Effondrement itératif de l'arête la plus courte (recherche linéaire sur `halfedges` à chaque itération) : fusionne les deux sommets au milieu, réassigne les demi-arêtes issues de B vers A, détecte et supprime les faces devenues dégénérées (sommets dupliqués après fusion), nettoie les demi-arêtes orphelines et le sommet fusionné. Nombre de collapses par appel = `numPhysicalEdges / 20` (au moins 1 si le maillage est assez grand). Appelle `computeNormals()` et `checkMesh()` à chaque itération.
- **Triangulation** — `myMesh::triangulate()` / `triangulate(myFace*)` dans `myMesh.cpp` (~ligne 433-525).
- **Autres opérations sur le maillage** : `splitEdge`, `splitFaceTRIS`, `splitFaceQUADS` (découpe), `computeNormals`, `normalize`, `buildTwins` (reconstruction des jumelles half-edge par correspondance de paires de sommets), `checkMesh` (validation : twins cohérents, faces avec ≥3 arêtes, next/prev cohérents).

### Menu GLUT — entrées définies mais non câblées
L'enum `MENU` (`main.cpp`) et `helperFunctions.h` définissent plus d'entrées de menu que `main.cpp` n'en traite dans le `switch`. Entrées **sans** `case` correspondant (no-op actuellement) : `MENU_LOOP`, `MENU_DRAWCREASE`, `MENU_GENERATE`, `MENU_CUT`, `MENU_SELECTFACE`, `MENU_CONTRACTEDGE`, `MENU_CONTRACTFACE`, `MENU_SMOOTHEN`, `MENU_UNDO`, `MENU_WRITE`.
Entrées **câblées** (case existant) : `MENU_TRIANGULATE`, `MENU_SHADINGTYPE`, `MENU_DRAWMESH`, `MENU_DRAWMESHVERTICES`, `MENU_DRAWWIREFRAME`, `MENU_DRAWNORMALS`, `MENU_DRAWSILHOUETTE`, `MENU_SELECTCLEAR`, `MENU_SELECTEDGE`, `MENU_SELECTVERTEX`, `MENU_INFLATE`, `MENU_CATMULLCLARK`, `MENU_SPLITEDGE`, `MENU_SPLITFACE`, `MENU_OPENFILE`, `MENU_EXIT`, `MENU_SIMPLIFY`.

## Rendu / UI
- OpenGL "moderne" (VAO/VBO + shaders GLSL, pas de fixed pipeline), shaders chargés depuis `shaders/light.{vert,frag}.glsl` (absents du dépôt, à fournir).
- FreeGLUT pour la fenêtre, la boucle d'événements et le menu contextuel (clic droit).
- Sélection interactive du sommet/arête/face le plus proche du point cliqué (`closest_vertex`, `closest_edge`, `closest_face` dans `main.cpp`).
- Import de fichiers OBJ via NFD (boîte de dialogue native) ; pas d'export/write fonctionnel (`MENU_WRITE` non câblé malgré l'entrée de menu).

## Consignes pour toute modification future
- Ne pas renommer/supprimer les fichiers `my*.{h,cpp}` existants sans raison forte : convention de nommage du cours, reprise probable par un correcteur ESIEE.
- Si vous implémentez Loop subdivision : ajouter la fonction dans `myMesh.{h,cpp}` (signature `void subdivisionLoop();` par cohérence avec `subdivisionCatmullClark()`) et le `case MENU_LOOP:` dans `main.cpp`.
- Toute dépendance ajoutée (shaders, libs) doit être documentée dans le README — actuellement plusieurs éléments requis à la compilation/exécution ne sont pas dans le dépôt (voir README, section Stack technique).
