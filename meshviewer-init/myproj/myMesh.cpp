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

		myPoint3D midpoint(
			(A->point->X + B->point->X) * 0.5,
			(A->point->Y + B->point->Y) * 0.5,
			(A->point->Z + B->point->Z) * 0.5
		);
		*(A->point) = midpoint;

		myHalfedge* candidate_originof_from_B_edges = nullptr;
		for (myHalfedge* he : halfedges) {
			if (he && he->source == B) {
				he->source = A;
				if (he != minEdge && he != minEdge->twin) {
					candidate_originof_from_B_edges = he;
				}
			}
		}

		std::set<myFace*> faces_to_delete_set;
		if (minEdge->adjacent_face) faces_to_delete_set.insert(minEdge->adjacent_face);
		if (minEdge->twin && minEdge->twin->adjacent_face) faces_to_delete_set.insert(minEdge->twin->adjacent_face);

		for (myFace* f : faces) {
			if (faces_to_delete_set.count(f)) continue;

			std::set<myVertex*> uniqueVerticesInFace;
			myHalfedge* start_he = f->adjacent_halfedge;
			if (!start_he) { faces_to_delete_set.insert(f); continue; }

			myHalfedge* current_he = start_he;
			bool faceIsValid = true;
			int edgeCount = 0;
			int max_loop_count = vertices.size() + 5;

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
				faces_to_delete_set.insert(f);
			}
		}

		std::set<myHalfedge*> halfedges_to_delete_set;
		halfedges_to_delete_set.insert(minEdge);
		if (minEdge->twin) halfedges_to_delete_set.insert(minEdge->twin);

		for (myFace* f_del : faces_to_delete_set) {
			myHalfedge* h = f_del->adjacent_halfedge;
			if (!h) continue;
			do {
				halfedges_to_delete_set.insert(h);
				h = h->next;
			} while (h != f_del->adjacent_halfedge && h != nullptr);
		}

		if (A->originof == minEdge || A->originof == nullptr || halfedges_to_delete_set.count(A->originof)) {
			A->originof = nullptr;
			if (candidate_originof_from_B_edges && candidate_originof_from_B_edges->source == A && !halfedges_to_delete_set.count(candidate_originof_from_B_edges)) {
				A->originof = candidate_originof_from_B_edges;
			}
			if (!A->originof) {
				myHalfedge* he_around_A = minEdge->next;
				if (he_around_A && !halfedges_to_delete_set.count(he_around_A) && he_around_A->source == A) {
					A->originof = he_around_A;
				}
				else {
					he_around_A = (minEdge->twin && minEdge->twin->next) ? minEdge->twin->next : nullptr;
					if (he_around_A && !halfedges_to_delete_set.count(he_around_A) && he_around_A->source == A) {
						A->originof = he_around_A;
					}
				}
			}
			if (!A->originof) {
				for (myHalfedge* he_iter : halfedges) {
					if (he_iter->source == A && !halfedges_to_delete_set.count(he_iter)) {
						A->originof = he_iter;
						break;
					}
				}
			}
		}

		for (myHalfedge* he_iter : halfedges) {
			if (halfedges_to_delete_set.count(he_iter)) {
				continue;
			}
			if (he_iter->twin && halfedges_to_delete_set.count(he_iter->twin)) {
				he_iter->twin = nullptr;
			}
		}

		for (myHalfedge* he_del : halfedges_to_delete_set) {
			auto it = std::find(halfedges.begin(), halfedges.end(), he_del);
			if (it != halfedges.end()) {
				halfedges.erase(it);
			}
			delete he_del;
		}
		myHalfedge* twin_of_min_edge = minEdge->twin;
		minEdge = nullptr;
		if (twin_of_min_edge) {}

		for (myFace* f_del : faces_to_delete_set) {
			auto it = std::find(faces.begin(), faces.end(), f_del);
			if (it != faces.end()) {
				faces.erase(it);
			}
			delete f_del;
		}

		auto it_v = std::find(vertices.begin(), vertices.end(), B);
		if (it_v != vertices.end()) {
			vertices.erase(it_v);
		}
		delete B;
		B = nullptr;

	}

	std::cout << "Simplification terminée." << std::endl;
	computeNormals();
	checkMesh();
}