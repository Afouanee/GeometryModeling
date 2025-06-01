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
	bool erreur = false;

	for (unsigned int i = 0; i < halfedges.size(); ++i) {
		myHalfedge* he = halfedges[i];
		if (he->twin == nullptr) {
			std::cout << "Erreur: demi-arête " << i << " n'a pas de twin." << std::endl;
			erreur = true;
		}
		else if (he->twin->twin != he) {
			std::cout << "Erreur: demi-arête " << i << " a un twin incohérent." << std::endl;
			erreur = true;
		}
	}

	for (unsigned int i = 0; i < halfedges.size(); ++i) {
		myHalfedge* he = halfedges[i];
		if (he->next == nullptr) {
			std::cout << "Erreur: demi-arête " << i << " n'a pas de next." << std::endl;
			erreur = true;
		}
		if (he->prev == nullptr) {
			std::cout << "Erreur: demi-arête " << i << " n'a pas de prev." << std::endl;
			erreur = true;
		}
	}

	for (unsigned int i = 0; i < faces.size(); ++i) {
		myFace* f = faces[i];
		if (f->adjacent_halfedge == nullptr) {
			std::cout << "Erreur: face " << i << " n'a pas de demi-arête adjacente." << std::endl;
			erreur = true;
			continue;
		}

		myHalfedge* start = f->adjacent_halfedge;
		myHalfedge* courant = start;
		int compteur = 0;
		do {
			if (courant == nullptr) {
				std::cout << "Erreur: face " << i << " a une boucle cassée." << std::endl;
				erreur = true;
				break;
			}
			courant = courant->next;
			compteur++;
			if (compteur > halfedges.size()) {
				std::cout << "Erreur: face " << i << " boucle trop longue." << std::endl;
				erreur = true;
				break;
			}
		} while (courant != start);
	}

	for (unsigned int i = 0; i < vertices.size(); ++i) {
		myVertex* v = vertices[i];
		if (v->originof == nullptr) {
			std::cout << "Attention: sommet " << i << " n'a pas de demi-arête d'origine." << std::endl;
		}
	}

	if (!erreur)
		std::cout << "Vérification terminée: pas d'erreur détectée." << std::endl;
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
			myVertex* v = new myVertex;
			v->point = p;
			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			vector<int> face_indices;
			while (myline >> u) { // read indices of vertices from a face into a container - it helps to access them later
				int vertex_index = atoi((u.substr(0, u.find("/"))).c_str()) - 1;
				face_indices.push_back(vertex_index);
			}

			myFace* face = new myFace();
			faces.push_back(face);

			vector<myHalfedge*> face_halfedges;
			for (size_t i = 0; i < face_indices.size(); ++i) {
				int current_index = face_indices[i];
				int next_index = face_indices[(i + 1) % face_indices.size()];

				myHalfedge* he = new myHalfedge();
				if (vertices[current_index]->originof == nullptr)
					vertices[current_index]->originof = he;
				he->source = vertices[current_index];
				he->adjacent_face = face;
				face_halfedges.push_back(he);

				pair<int, int> edge_key(current_index, next_index);
				twin_map[edge_key] = he;

				pair<int, int> twin_key(next_index, current_index);
				it = twin_map.find(twin_key);

				if (it != twin_map.end()) {
					he->twin = it->second;
					it->second->twin = he;
				}
			}

			for (size_t i = 0; i < face_halfedges.size(); ++i) {
				face_halfedges[i]->next = face_halfedges[(i + 1) % face_halfedges.size()];
				face_halfedges[i]->prev = face_halfedges[(i + face_halfedges.size() - 1) % face_halfedges.size()];
			}

			face->adjacent_halfedge = face_halfedges[0];
			halfedges.insert(halfedges.end(), face_halfedges.begin(), face_halfedges.end());
		}
	}

	checkMesh();
	normalize();
	for (myHalfedge* he : halfedges) {
		if (he->source && !he->source->originof)
			he->source->originof = he;
	}

	return true;
}


void myMesh::computeNormals()
{
	for (int i = 0; i < vertices.size(); i++) {
		if (vertices[i] && vertices[i]->originof && vertices[i]->originof->twin)
			vertices[i]->computeNormal();
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
	vector<myFace*> old_faces = faces;

	for (myFace* f : old_faces)
		triangulate(f);

	computeNormals();

}

bool myMesh::triangulate(myFace* face)
{
	if (!face) return false;

	std::vector<myHalfedge*> edgeLoop;
	myHalfedge* first = face->adjacent_halfedge;
	myHalfedge* current = first;

	do {
		edgeLoop.push_back(current);
		current = current->next;
	} while (current != first);

	if (edgeLoop.size() == 3)
		return false;

	myVertex* origin = edgeLoop[0]->source;

	for (size_t i = 1; i < edgeLoop.size() - 1; ++i)
	{
		myFace* triangle = new myFace();
		faces.push_back(triangle);

		myHalfedge* e1 = new myHalfedge();
		myHalfedge* e2 = new myHalfedge();
		myHalfedge* e3 = new myHalfedge();

		e1->source = origin;
		e2->source = edgeLoop[i]->source;
		e3->source = edgeLoop[i + 1]->source;

		e1->next = e2; e2->next = e3; e3->next = e1;
		e1->prev = e3; e2->prev = e1; e3->prev = e2;

		e1->adjacent_face = triangle;
		e2->adjacent_face = triangle;
		e3->adjacent_face = triangle;
		triangle->adjacent_halfedge = e1;

		halfedges.push_back(e1);
		halfedges.push_back(e2);
		halfedges.push_back(e3);
		if (!origin->originof) origin->originof = e1;
		if (!e2->source->originof) e2->source->originof = e2;
		if (!e3->source->originof) e3->source->originof = e3;
	}

	for (myHalfedge* oldEdge : edgeLoop)
	{
		if (oldEdge->source && oldEdge->source->originof == oldEdge)
			oldEdge->source->originof = nullptr;

		auto it = std::find(halfedges.begin(), halfedges.end(), oldEdge);
		if (it != halfedges.end())
			halfedges.erase(it);

		if (oldEdge->twin)
			oldEdge->twin->twin = nullptr;

		delete oldEdge;
	}

	auto faceIt = std::find(faces.begin(), faces.end(), face);
	if (faceIt != faces.end())
		faces.erase(faceIt);
	delete face;

	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> edgeMap;
	for (myHalfedge* he : halfedges) {
		edgeMap[{he->source, he->next->source}] = he;
	}
	for (myHalfedge* he : halfedges) {
		auto reverseKey = std::make_pair(he->next->source, he->source);
		if (edgeMap.count(reverseKey)) {
			he->twin = edgeMap[reverseKey];
		}
	}

	return true;
}


void myMesh::simplify()
{


}