#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"

using namespace std;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex*> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge*> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace*> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	vector<myHalfedge*>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
}


bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge** hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;
		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			cout << "v " << x << " " << y << " " << z << endl;
			myPoint3D* p = new myPoint3D(x, y, z);
			myVertex* v = new myVertex();
			v->point = p;
			vertices.push_back(v);

		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u) // read indices of vertices from a face into a container - it helps to access them later 
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);
			if (faceids.size() < 3) // ignore degenerate faces
				continue;

			hedges = new myHalfedge * [faceids.size()]; // allocate the array for storing pointers to half-edges
			for (unsigned int i = 0; i < faceids.size(); i++)
				hedges[i] = new myHalfedge(); // pre-allocate new half-edges

			myFace* f = new myFace(); // allocate the new face
			f->adjacent_halfedge = hedges[0]; // connect the face with incident edge

			for (unsigned int i = 0; i < faceids.size(); i++)
			{
				int iplusone = (i + 1) % faceids.size();
				int iminusone = (i - 1 + faceids.size()) % faceids.size();


				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];

				hedges[i]->source = vertices[faceids[i]];

				hedges[i]->adjacent_face = f;

				auto twin = make_pair(faceids[iplusone], faceids[i]);
				it = twin_map.find(twin);
				if (it != twin_map.end()) {
					hedges[i]->twin = it->second;
					it->second->twin = hedges[i];
				}
				else {
					twin_map[make_pair(faceids[i], faceids[iplusone])] = hedges[i];
				}

				vertices[faceids[i]]->originof = hedges[i]; // connect the vertex with incident edge

				halfedges.push_back(hedges[i]); // add the half-edge to the list of half-edges
			}

			faces.push_back(f); // add the face to the list of faces

		}
	}

	checkMesh();
	normalize();

	return true;
}



void myMesh::computeNormals()
{
	for (int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->normal) {
			vertices[i]->normal->clear();
		}
		else {
			vertices[i]->normal = new myVector3D();
		}
	}

	for (int i = 0; i < faces.size(); i++) {
		if (faces[i]) {
			faces[i]->computeNormal();
		}
	}

	for (int i = 0; i < vertices.size(); i++) {
		if (vertices[i]) {
			vertices[i]->computeNormal();
		}
	}
}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}


void myMesh::splitFaceTRIS(myFace* f, myPoint3D* p)
{
	/**** TODO ****/
}

void myMesh::splitEdge(myHalfedge* e1, myPoint3D* p)
{

	/**** TODO ****/
}

void myMesh::splitFaceQUADS(myFace* f, myPoint3D* p)
{
	/**** TODO ****/
}


void myMesh::subdivisionCatmullClark()
{
	/**** TODO ****/
}

void myMesh::triangulate()
{
	std::vector<myFace*> old_faces = faces;
	for (myFace* f : old_faces)
	{
		if (f != nullptr)
			triangulate(f);
	}
	linkTwinsIfMissing();
	checkMesh();
}

//return false if already triangle, true othewise.
bool myMesh::triangulate(myFace* f)
{
	if (!f) return false;

	std::vector<myHalfedge*> border;
	myHalfedge* he = f->adjacent_halfedge;

	// Collecte les sommets dans l'ordre
	do {
		border.push_back(he);
		he = he->next;
	} while (he != f->adjacent_halfedge);

	int num = halfedges.size();
	if (num <= 3) return false;


	std::vector<myVertex*> v;
	for (myHalfedge* he : border)
	{
		v.push_back(he->source);
	}

	for (myHalfedge* he : border)
	{
		if (he->source && he->source->originof == he)
			he->source->originof = nullptr;

		if (he->twin)
			he->twin->twin = nullptr;

		auto it = std::find(halfedges.begin(), halfedges.end(), he);
		if (it != halfedges.end()) halfedges.erase(it);
		delete he;
	}

	auto it = std::find(faces.begin(), faces.end(), f);
	if (it != faces.end()) faces.erase(it);
	delete f;

	myHalfedge* twin_tmp = nullptr;

	for (int i = 1; i < v.size() - 1; ++i)
	{
		myFace* newFace = new myFace();

		myHalfedge* he0 = new myHalfedge(); // v0 -> vi
		myHalfedge* he1 = new myHalfedge(); // vi -> vi+1
		myHalfedge* he2 = new myHalfedge(); // vi+1 -> v0

		he0->source = v[0];
		he1->source = v[i];
		he2->source = v[i + 1];

		he0->next = he1; he1->next = he2; he2->next = he0;
		he0->prev = he2; he1->prev = he0; he2->prev = he1;

		he0->adjacent_face = newFace;
		he1->adjacent_face = newFace;
		he2->adjacent_face = newFace;

		newFace->adjacent_halfedge = he0;

		// Gestion du twin entre triangles successifs (logique de fan)
		if (twin_tmp != nullptr)
		{
			he0->twin = twin_tmp;
			twin_tmp->twin = he0;
		}
		twin_tmp = he1; // Pour le prochain triangle

		// Ajout au mesh
		halfedges.push_back(he0);
		halfedges.push_back(he1);
		halfedges.push_back(he2);
		faces.push_back(newFace);
	}

	return true;
}
void myMesh::linkTwinsIfMissing()
{
	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> halfedge_pairs;

	for (myHalfedge* edge : halfedges)
	{
		// Vérification de validité
		if (!edge || !edge->next || !edge->source || !edge->next->source)
			continue;

		myVertex* start = edge->source;
		myVertex* end = edge->next->source;

		std::pair<myVertex*, myVertex*> direct = std::make_pair(start, end);
		std::pair<myVertex*, myVertex*> inverse = std::make_pair(end, start);

		auto it = halfedge_pairs.find(inverse);
		if (it != halfedge_pairs.end())
		{
			myHalfedge* twin_candidate = it->second;
			if (!edge->twin && !twin_candidate->twin) {
				edge->twin = twin_candidate;
				twin_candidate->twin = edge;
			}
		}
		else
		{
			halfedge_pairs[direct] = edge;
		}
	}
}
