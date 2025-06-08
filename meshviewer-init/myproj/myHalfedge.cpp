#include "myHalfedge.h"
#include "myPoint3D.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <cmath>

myHalfedge::myHalfedge(void)
{
	source = NULL; 
	adjacent_face = NULL; 
	next = NULL;  
	prev = NULL;  
	twin = NULL;  
}

void myHalfedge::copy(myHalfedge *ie)
{
/**** TODO ****/
	if (!ie) return;

	source = ie->source;
	adjacent_face = ie->adjacent_face;
	next = ie->next;
	prev = ie->prev;
	twin = ie->twin;
}

myHalfedge::~myHalfedge(void)
{
}


float myHalfedge::computeLength() const {
	if (!source || !next || !next->source || !source->point || !next->source->point)
		return 0.0f;

	float dx = source->point->X - next->source->point->X;
	float dy = source->point->Y - next->source->point->Y;
	float dz = source->point->Z - next->source->point->Z;

	return std::sqrt(dx * dx + dy * dy + dz * dz);
}
