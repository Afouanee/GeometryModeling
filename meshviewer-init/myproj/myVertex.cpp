#include "myVertex.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0,1.0,1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}




void myVertex::computeNormal()
{
    if (!originof) return;

    normal->dX = 0.0; normal->dY = 0.0; normal->dZ = 0.0;

    myHalfedge* he = originof;
    myHalfedge* start = he;

    int nbfaces = 0;
    do
    {
        if (!he || !he->adjacent_face || !he->adjacent_face->normal) break;

        normal->dX += he->adjacent_face->normal->dX;
        normal->dY += he->adjacent_face->normal->dY;
        normal->dZ += he->adjacent_face->normal->dZ;

        nbfaces++;
        if (!he->twin || !he->twin->next) break;
        he = he->twin->next;

    } while (he != start);

    if (nbfaces > 0)
    {
        normal->dX /= nbfaces;
        normal->dY /= nbfaces;
        normal->dZ /= nbfaces;
    }

    normal->normalize();
}
