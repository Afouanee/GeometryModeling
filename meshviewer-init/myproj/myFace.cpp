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
    myHalfedge* he = adjacent_halfedge;
    myVector3D v1, v2;

    v1.dX = he->next->source->point->X - he->source->point->X;
    v1.dY = he->next->source->point->Y - he->source->point->Y;
    v1.dZ = he->next->source->point->Z - he->source->point->Z;

    he = he->next;
    v2.dX = he->next->source->point->X - he->source->point->X;
    v2.dY = he->next->source->point->Y - he->source->point->Y;
    v2.dZ = he->next->source->point->Z - he->source->point->Z;

    normal->crossproduct(v1, v2);
    normal->normalize();
}