#include "myFace.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}

void myFace::computeNormal()
{
    // Si la face n'a pas d'arête associée, impossible de calculer sa normale
    if (adjacent_halfedge == nullptr) {
        return;
    }

    myVector3D accumulatedNormal(0.0, 0.0, 0.0);

    myHalfedge* currentEdge = adjacent_halfedge;
    myPoint3D* referencePoint = currentEdge->source->point;

    myHalfedge* edge1 = currentEdge;
    myHalfedge* edge2 = edge1->next;

    // Parcours des triangles formés autour du point de référence
    while (edge2->next != currentEdge)
    {
        // Points des sommets suivants
        myPoint3D* point1 = edge1->next->source->point;
        myPoint3D* point2 = edge2->next->source->point;

        // Création de deux vecteurs basés sur le point de référence
        myVector3D vecU = *point1 - *referencePoint;
        myVector3D vecV = *point2 - *referencePoint;

        // Le produit vectoriel donne une normale locale pour ce triangle
        myVector3D localNormal = vecU.crossproduct(vecV);

        // On ajoute cette normale au total
        accumulatedNormal += localNormal;

        // Avancer dans la liste des arêtes
        edge1 = edge2;
        edge2 = edge2->next;
    }

    // Normalisation pour obtenir une normale unitaire
    accumulatedNormal.normalize();

    // Affecter le résultat à la normale de la face
    *normal = accumulatedNormal;
}

