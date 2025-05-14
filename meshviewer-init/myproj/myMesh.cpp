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

// Retourne false si la face est déjà triangulaire, true sinon (triangulation effectuée)
bool myMesh::triangulate(myFace* face)
{
	if (!face) return false;

	// Collecte de tous les half-edges formant la boucle autour de la face
	std::vector<myHalfedge*> edgeLoop;
	myHalfedge* first = face->adjacent_halfedge;
	myHalfedge* current = first;

	do {
		edgeLoop.push_back(current);
		current = current->next;
	} while (current != first);

	// Si la face est déjà un triangle, pas besoin de trianguler
	if (edgeLoop.size() == 3)
		return false;

	// On garde la première origine pour former les triangles depuis ce sommet
	myVertex* origin = edgeLoop[0]->source;

	// On génère les triangles en utilisant un "fan triangulation" depuis 'origin'
	for (size_t i = 1; i < edgeLoop.size() - 1; ++i)
	{
		// Création d'une nouvelle face
		myFace* triangle = new myFace();
		faces.push_back(triangle);

		// Création des trois nouveaux half-edges pour cette face
		myHalfedge* e1 = new myHalfedge();
		myHalfedge* e2 = new myHalfedge();
		myHalfedge* e3 = new myHalfedge();

		// Attribution des sommets sources aux half-edges
		e1->source = origin;
		e2->source = edgeLoop[i]->source;
		e3->source = edgeLoop[i + 1]->source;

		// Connexion des half-edges entre eux pour former une boucle
		e1->next = e2; e2->next = e3; e3->next = e1;
		e1->prev = e3; e2->prev = e1; e3->prev = e2;

		// Affectation de la nouvelle face aux half-edges
		e1->adjacent_face = triangle;
		e2->adjacent_face = triangle;
		e3->adjacent_face = triangle;
		triangle->adjacent_halfedge = e1;

		// Ajout dans les conteneurs
		halfedges.push_back(e1);
		halfedges.push_back(e2);
		halfedges.push_back(e3);
		if (!origin->originof) origin->originof = e1;
		if (!e2->source->originof) e2->source->originof = e2;
		if (!e3->source->originof) e3->source->originof = e3;
	}

	// Nettoyage : suppression des anciens half-edges
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

	// Suppression de la face d'origine (non triangulaire)
	auto faceIt = std::find(faces.begin(), faces.end(), face);
	if (faceIt != faces.end())
		faces.erase(faceIt);
	delete face;

	// Mise à jour des twins : on connecte les half-edges opposés
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
