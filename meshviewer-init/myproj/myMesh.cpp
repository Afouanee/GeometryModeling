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
	// Copie la liste des faces existantes pour ne pas modifier l'original pendant l'itération
	std::vector<myFace*> initialFaces = faces;

	// Pour chaque face dans la liste copiée, on procède à la triangulation
	for (myFace* face : initialFaces)
	{
		triangulate(face);  // Appel de la fonction de triangulation pour chaque face
	}
}

bool myMesh::triangulate(myFace* face)
{
	// Vérifie si la face est valide et contient des half-edges adjacents
	if (!face || !face->adjacent_halfedge) return false;

	// Comptabilise le nombre de sommets de la face en suivant les half-edges
	int numVertices = 1;  // On commence à 1 car on compte le premier sommet
	myHalfedge* halfEdge = face->adjacent_halfedge->next;
	while (halfEdge != face->adjacent_halfedge) {
		numVertices++;
		halfEdge = halfEdge->next;
	}

	// Si la face est déjà un triangle (3 sommets ou moins), on n'a rien à faire
	if (numVertices <= 3) return false;

	// On commence à parcourir les sommets de la face pour les stocker
	std::vector<myVertex*> vertices;
	myHalfedge* startEdge = face->adjacent_halfedge;
	halfEdge = startEdge;
	do {
		vertices.push_back(halfEdge->source);  // Stocke le sommet du half-edge courant

		// Supprime proprement l'half-edge de la liste des half-edges
		auto it = std::find(halfedges.begin(), halfedges.end(), halfEdge);
		if (it != halfedges.end()) halfedges.erase(it);
		delete halfEdge;  // Libération de la mémoire

		halfEdge = halfEdge->next;  // Passe au half-edge suivant
	} while (halfEdge != startEdge);

	// Supprime la face originale de la liste des faces
	auto faceIt = std::find(faces.begin(), faces.end(), face);
	if (faceIt != faces.end()) {
		faces.erase(faceIt);
	}
	delete face;  // Libère la mémoire allouée pour la face

	// Map pour gérer les twin edges, utilisant un couple de sommets
	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> edgePairs;

	// Crée des triangles en partant du premier sommet
	for (size_t i = 1; i < vertices.size() - 1; ++i) {
		// Création d'une nouvelle face pour chaque triangle
		myFace* newFace = new myFace();

		// Création des trois half-edges pour le triangle
		myHalfedge* he1 = new myHalfedge();
		myHalfedge* he2 = new myHalfedge();
		myHalfedge* he3 = new myHalfedge();

		// Définition des sources des half-edges
		he1->source = vertices[0];
		he2->source = vertices[i];
		he3->source = vertices[i + 1];

		// Relie les half-edges entre eux (circuit fermé)
		he1->next = he2; he2->next = he3; he3->next = he1;
		he1->prev = he3; he2->prev = he1; he3->prev = he2;

		// Associe les half-edges à la face correspondante
		he1->adjacent_face = newFace;
		he2->adjacent_face = newFace;
		he3->adjacent_face = newFace;
		newFace->adjacent_halfedge = he1;

		// Ajoute la nouvelle face à la liste des faces
		faces.push_back(newFace);

		// Ajoute les half-edges et gère les twins
		auto addTwinEdge = [&](myHalfedge* halfEdge, myVertex* from, myVertex* to) {
			std::pair<myVertex*, myVertex*> key = std::make_pair(from, to);
			std::pair<myVertex*, myVertex*> twinKey = std::make_pair(to, from);

			// Si le twin edge existe, on les lie
			if (edgePairs.count(twinKey)) {
				myHalfedge* twin = edgePairs[twinKey];
				halfEdge->twin = twin;
				twin->twin = halfEdge;
			}

			edgePairs[key] = halfEdge;  // Ajoute l'edge à la map
			halfedges.push_back(halfEdge);  // Ajoute le half-edge à la liste globale
			};

		// Ajout des twin edges pour les trois half-edges du triangle
		addTwinEdge(he1, he1->source, he2->source);
		addTwinEdge(he2, he2->source, he3->source);
		addTwinEdge(he3, he3->source, he1->source);
	}

	// Retourne true après avoir triangulé avec succès
	return true;
}
