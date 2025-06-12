#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"
#include <set>
  

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
	bool error_found = false;

	// Vérifier que chaque demi-arête a un twin cohérent
	for (unsigned int i = 0; i < halfedges.size(); i++) {
		myHalfedge* he = halfedges[i];
		if (!he->twin) {
			std::cout << "Erreur : demi-arête " << i << " sans twin." << std::endl;
			error_found = true;
		}
		else if (he->twin->twin != he) {
			std::cout << "Erreur : demi-arête " << i << " et son twin ne correspondent pas." << std::endl;
			error_found = true;
		}
	}

	// Vérifier que les faces ont au moins 3 demi-arêtes et que next/prev sont cohérents
	for (unsigned int i = 0; i < faces.size(); i++) {
		myFace* f = faces[i];
		if (!f->adjacent_halfedge) {
			std::cout << "Erreur : face " << i << " sans demi-arête adjacente." << std::endl;
			error_found = true;
			continue;
		}

		myHalfedge* start = f->adjacent_halfedge;
		myHalfedge* he = start;
		int count = 0;

		do {
			if (!he->next) {
				std::cout << "Erreur : demi-arête dans face " << i << " sans next." << std::endl;
				error_found = true;
				break;
			}
			if (he->next->prev != he) {
				std::cout << "Erreur : incohérence next-prev dans face " << i << std::endl;
				error_found = true;
			}
			he = he->next;
			count++;
		} while (he != start);

		if (count < 3) {
			std::cout << "Erreur : face " << i << " a moins de 3 demi-arêtes." << std::endl;
			error_found = true;
		}
	}

	// Vérifier que chaque vertex a une demi-arête d'origine
	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (!vertices[i]->originof) {
			std::cout << "Attention : vertex " << i << " sans demi-arête d'origine." << std::endl;
		}
	}

	if (!error_found) {
		std::cout << "CheckMesh OK : pas d'erreur détectée." << std::endl;
	}
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


void myMesh::subdivisionCatmullClark() {
	std::map<myFace*, myPoint3D> facePoints;
	std::map<std::pair<myVertex*, myVertex*>, myPoint3D> edgePoints;
	std::map<myVertex*, myPoint3D> vertexPoints;

	for (myFace* f : faces) {
		int count = 0;

		myHalfedge* h = f->adjacent_halfedge;
		myHalfedge* start = h;
		myPoint3D sum(0, 0, 0);

		do {
			sum += *(h->source->point);
			count++;
			h = h->next;
		} while (h != start);
		facePoints[f] = sum / count;
	}

	for (myHalfedge* h : halfedges) {
		myVertex* v1 = h->source;
		myVertex* v2 = h->next->source;

		auto key = std::make_pair(std::min(v1, v2), std::max(v1, v2));
		if (edgePoints.count(key)) continue;

		myPoint3D p1 = *(v1->point);
		myPoint3D p2 = *(v2->point);

		if (h->adjacent_face && h->twin && h->twin->adjacent_face)
			edgePoints[key] = (p1 + p2 + facePoints[h->adjacent_face] + facePoints[h->twin->adjacent_face]) * 0.25;
		else
			edgePoints[key] = (p1 + p2) * 0.5;
	}

	for (myVertex* v : vertices) {
		if (!v->originof) {
			vertexPoints[v] = *(v->point);
			continue;
		}

		myHalfedge* h = v->originof;
		myHalfedge* start = h;

		int k = 0;
		myPoint3D sumF(0, 0, 0);
		myPoint3D sumE(0, 0, 0);

		do {
			if (h->adjacent_face) sumF += facePoints[h->adjacent_face];
			myPoint3D mid = (*(h->source->point) + *(h->next->source->point)) * 0.5;
			sumE += mid;
			k++;

			h = h->twin ? h->twin->next : nullptr;
		} while (h && h != start);

		myPoint3D F = sumF / k;
		myPoint3D R = sumE / k;
		myPoint3D P = *(v->point);

		vertexPoints[v] = (F + R * 2.0 + P * (k - 3)) / k;
	}

	myMesh newMesh;

	std::map<myFace*, myVertex*> faceVerts;
	std::map<std::pair<myVertex*, myVertex*>, myVertex*> edgeVerts;
	std::map<myVertex*, myVertex*> vertexVerts;

	for (std::map<myFace*, myPoint3D>::iterator it = facePoints.begin(); it != facePoints.end(); ++it) {
		myFace* f = it->first;
		myPoint3D pt = it->second;

		myVertex* v = new myVertex();
		v->point = new myPoint3D(pt);
		faceVerts[f] = v;
		newMesh.vertices.push_back(v);
	}


	for (std::map<std::pair<myVertex*, myVertex*>, myPoint3D>::iterator it = edgePoints.begin(); it != edgePoints.end(); ++it) {
		std::pair<myVertex*, myVertex*> e = it->first;
		myPoint3D pt = it->second;

		myVertex* v = new myVertex();
		v->point = new myPoint3D(pt);
		edgeVerts[e] = v;
		newMesh.vertices.push_back(v);
	}


	for (std::map<myVertex*, myPoint3D>::iterator it = vertexPoints.begin(); it != vertexPoints.end(); ++it) {
		myVertex* v = it->first;
		myPoint3D pt = it->second;

		myVertex* vNew = new myVertex();
		vNew->point = new myPoint3D(pt);
		vertexVerts[v] = vNew;
		newMesh.vertices.push_back(vNew);
	}

	for (myFace* f : faces) {
		std::vector<myHalfedge*> boundary;
		myHalfedge* h = f->adjacent_halfedge;
		myHalfedge* start = h;
		do {
			boundary.push_back(h);
			h = h->next;
		} while (h != start);

		int k = boundary.size();

		for (int i = 0; i < k; ++i) {
			myHalfedge* he_i = boundary[i];
			myHalfedge* he_prev = boundary[(i - 1 + k) % k];

			myVertex* v = he_i->source;
			myVertex* v_corner = vertexVerts[v];
			auto key_i = std::make_pair(std::min(he_i->source, he_i->next->source), std::max(he_i->source, he_i->next->source));
			auto key_prev = std::make_pair(std::min(he_prev->source, he_prev->next->source), std::max(he_prev->source, he_prev->next->source));

			myVertex* v_ei = edgeVerts[key_i];
			myVertex* v_eprev = edgeVerts[key_prev];
			myVertex* v_f = faceVerts[f];

			myHalfedge* h0 = new myHalfedge();
			myHalfedge* h1 = new myHalfedge();
			myHalfedge* h2 = new myHalfedge();
			myHalfedge* h3 = new myHalfedge();

			h0->source = v_ei;
			h1->source = v_f;
			h2->source = v_eprev;
			h3->source = v_corner;

			h0->next = h1; h1->next = h2; h2->next = h3; h3->next = h0;
			h0->prev = h3; h1->prev = h0; h2->prev = h1; h3->prev = h2;

			myFace* newF = new myFace();
			newF->adjacent_halfedge = h0;

			h0->adjacent_face = newF;
			h1->adjacent_face = newF;
			h2->adjacent_face = newF;
			h3->adjacent_face = newF;

			if (!v_corner->originof) v_corner->originof = h3;
			if (!v_ei->originof) v_ei->originof = h0;
			if (!v_f->originof) v_f->originof = h1;
			if (!v_eprev->originof) v_eprev->originof = h2;

			newMesh.faces.push_back(newF);
			newMesh.halfedges.push_back(h0);
			newMesh.halfedges.push_back(h1);
			newMesh.halfedges.push_back(h2);
			newMesh.halfedges.push_back(h3);
		}
	}

	newMesh.buildTwins();

	newMesh.computeNormals();
	
	clear();
	
	*this = std::move(newMesh);
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



void myMesh::simplify() {
	if (halfedges.empty()) {
		std::cout << "Maillage vide, pas de simplification." << std::endl;
		return;
	}
	int numPhysicalEdges = halfedges.size() / 2;
	int collapsesToAttempt = numPhysicalEdges / 20;
	if (collapsesToAttempt == 0 && numPhysicalEdges > 4) {
		collapsesToAttempt = 1;
	}

	for (int i = 0; i < collapsesToAttempt; ++i) {
		if (vertices.size() <= 3 || halfedges.size() < 6) {
			std::cout << "Maillage trop petit, simplification arrêtée." << std::endl;
			break;
		}

		myHalfedge* minEdge = nullptr;
		double minDistSq = std::numeric_limits<double>::max();

		// Trouver l'arête la plus courte
		for (myHalfedge* he : halfedges) {
			if (!he || !he->twin || !he->source || !he->twin->source || !he->source->point || !he->twin->source->point) continue;
			if (he->source == he->twin->source) continue;

			myPoint3D* p1 = he->source->point;
			myPoint3D* p2 = he->twin->source->point;
			double dx = p1->X - p2->X;
			double dy = p1->Y - p2->Y;
			double dz = p1->Z - p2->Z;
			double currentDistSq = dx * dx + dy * dy + dz * dz;

			if (currentDistSq < minDistSq) {
				minDistSq = currentDistSq;
				minEdge = he;
			}
		}

		if (!minEdge) {
			std::cout << "Aucune arête à effondrer." << std::endl;
			break;
		}

		myVertex* A = minEdge->source;
		myVertex* B = minEdge->twin->source;

		if (!A || !B || !A->point || !B->point) {
			std::cerr << "Erreur: Sommet nul dans minEdge." << std::endl;
			continue;
		}

		// Mettre à jour la position du sommet A au milieu de l'arête
		myPoint3D midpoint(
			(A->point->X + B->point->X) * 0.5,
			(A->point->Y + B->point->Y) * 0.5,
			(A->point->Z + B->point->Z) * 0.5
		);
		*(A->point) = midpoint;

		// Transférer toutes les arêtes originaires de B vers A
		for (myHalfedge* he : halfedges) {
			if (he && he->source == B) {
				he->source = A;
			}
		}

		// Identifier les faces qui deviennent invalides après la contraction (ex : avec des sommets dupliqués)
		std::set<myFace*> faces_to_delete;
		if (minEdge->adjacent_face) faces_to_delete.insert(minEdge->adjacent_face);
		if (minEdge->twin && minEdge->twin->adjacent_face) faces_to_delete.insert(minEdge->twin->adjacent_face);

		for (myFace* f : faces) {
			if (faces_to_delete.count(f)) continue;

			std::set<myVertex*> uniqueVerticesInFace;
			myHalfedge* start_he = f->adjacent_halfedge;
			if (!start_he) {
				faces_to_delete.insert(f);
				continue;
			}

			myHalfedge* current_he = start_he;
			bool faceIsValid = true;
			int edgeCount = 0;
			int max_loop_count = vertices.size() + 5; // limite pour éviter boucle infinie

			do {
				if (!current_he || !current_he->source) { faceIsValid = false; break; }
				if (!uniqueVerticesInFace.insert(current_he->source).second) { faceIsValid = false; break; }
				edgeCount++;
				current_he = current_he->next;
				if (edgeCount > max_loop_count) { faceIsValid = false; break; }
			} while (current_he != start_he && current_he != nullptr);

			if (current_he == nullptr && start_he != nullptr) faceIsValid = false;
			if (edgeCount < 3) faceIsValid = false;

			if (!faceIsValid) {
				faces_to_delete.insert(f);
			}
		}

		// Supprimer les faces invalides et leurs demi-arêtes
		std::set<myHalfedge*> halfedges_to_delete;
		for (myFace* f_del : faces_to_delete) {
			myHalfedge* h = f_del->adjacent_halfedge;
			if (h) {
				myHalfedge* start = h;
				do {
					halfedges_to_delete.insert(h);
					h = h->next;
				} while (h != start && h != nullptr);
			}
		}

		// Supprimer les demi-arêtes sélectionnées
		for (myHalfedge* h_del : halfedges_to_delete) {
			// Supprimer le twin de la demi-arête s'il existe
			if (h_del->twin) h_del->twin->twin = nullptr;

			auto it = std::find(halfedges.begin(), halfedges.end(), h_del);
			if (it != halfedges.end()) halfedges.erase(it);

			delete h_del;
		}

		// Supprimer les faces invalides
		for (myFace* f_del : faces_to_delete) {
			auto it = std::find(faces.begin(), faces.end(), f_del);
			if (it != faces.end()) faces.erase(it);
			delete f_del;
		}

		// Supprimer le sommet B (car fusionné avec A)
		auto itv = std::find(vertices.begin(), vertices.end(), B);
		if (itv != vertices.end()) vertices.erase(itv);
		delete B;

		computeNormals();
		checkMesh();
	}
}


void myMesh::buildTwins()
{
	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> edgeMap;

	for (myHalfedge* he : halfedges)
	{
		myVertex* from = he->source;
		myVertex* to = he->next->source;

		auto key = std::make_pair(from, to);
		auto twinKey = std::make_pair(to, from);

		edgeMap[key] = he;

		if (edgeMap.count(twinKey)) {
			he->twin = edgeMap[twinKey];
			edgeMap[twinKey]->twin = he;
		}
	}
}