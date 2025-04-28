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
    if (originof == nullptr) {
        return;
    }

    myVector3D accumulatedNormal(0.0, 0.0, 0.0);
    myHalfedge* current = originof;
    myHalfedge* iter = current;

    do {
        myFace* adjacentFace = iter->adjacent_face;

        if (adjacentFace != nullptr) {
            if (adjacentFace->normal == nullptr || adjacentFace->normal->length() < 1e-6) {
                adjacentFace->computeNormal();
            }
            accumulatedNormal += *(adjacentFace->normal);
        }

        if (iter->twin == nullptr || iter->twin->next == nullptr) {
            break;
        }
        iter = iter->twin->next;

    } while (iter != current);

    if (accumulatedNormal.length() > 1e-6) {
        accumulatedNormal.normalize();
        *normal = accumulatedNormal;
    }
}
