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

    if (!normal)
        normal = new myVector3D();

    normal->clear();

    myHalfedge* start = originof;
    myHalfedge* current = start;
    do {
        if (current->adjacent_face && current->adjacent_face->normal) {
            *normal = *normal + *(current->adjacent_face->normal);
        }

        current = (current->twin) ? current->twin->next : nullptr;
    } while (current && current != start);

    normal->normalize();
}

