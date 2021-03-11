#include "Subdiv.hpp"

using namespace std;

Mesh _mesh;

Mesh loadMesh(string fileName)
{
	Mesh mesh;
	ifstream stream(fileName);
	string str;

	getline(stream, str);
	getline(stream, str);
	getline(stream, str);
	getline(stream, str);
	stream >> str;

	while (str == "v")
	{
		Mesh::Vertex temp;
		GLfloat x, y, z;
		stream >> x >> y >> z;
		temp.position = glm::vec3(x, y, z);
		mesh.vertices.push_back(temp);
		stream >> str;
	}

	while (str == "vn")
	{
		glm::vec3 temp;
		GLfloat x, y, z;
		stream >> x >> y >> z;
		temp = glm::vec3(x, y, z);
		mesh.normals.push_back(temp);
		stream >> str;
	}

	getline(stream, str);
	getline(stream, str);
	stream >> str;

	while (str == "f")
	{
		stream >> str;
		mesh.indices.push_back(atoi(str.substr(0, str.find_first_of('/')).c_str()) - 1);
		mesh.vertices[mesh.indices[mesh.indices.size() - 1]].normal = mesh.normals[atoi(str.substr(str.find_last_of('/') + 1, str.length() - 1).c_str()) - 1];
		stream >> str;
		mesh.indices.push_back(atoi(str.substr(0, str.find_first_of('/')).c_str()) - 1);
		mesh.vertices[mesh.indices[mesh.indices.size() - 1]].normal = mesh.normals[atoi(str.substr(str.find_last_of('/') + 1, str.length() - 1).c_str()) - 1];
		stream >> str;
		mesh.indices.push_back(atoi(str.substr(0, str.find_first_of('/')).c_str()) - 1);
		mesh.vertices[mesh.indices[mesh.indices.size() - 1]].normal = mesh.normals[atoi(str.substr(str.find_last_of('/') + 1, str.length() - 1).c_str()) - 1];
		stream >> str;
	}

	stream.close();

	for (int i = 0; i < mesh.vertices.size(); i++)
	{
		Mesh::VertexSubdiv vertexSubdiv;
		vertexSubdiv.position = mesh.vertices[i].position;
		mesh.verticesSubdiv.push_back(vertexSubdiv);
	}

	for (int i = 0; i < mesh.indices.size(); i += 3)
	{
		Mesh::Face face;
		face.vertices[0] = mesh.indices[i];
		face.vertices[1] = mesh.indices[i + 1];
		face.vertices[2] = mesh.indices[i + 2];
		mesh.faces.push_back(face);
	}

	mesh.halfEdge.reserve(1000000);

	for (int i = 0; i < mesh.faces.size(); i++)
	{
		Mesh::HalfEdge halfEdge;
		halfEdge.v1 = mesh.faces[i].vertices[0];
		halfEdge.v2 = mesh.faces[i].vertices[1];
		halfEdge.vertex = &mesh.verticesSubdiv[mesh.faces[i].vertices[1]];
		halfEdge.face = &mesh.faces[i];

		Mesh::HalfEdge halfEdge2find;
		halfEdge2find.v1 = mesh.faces[i].vertices[1];
		halfEdge2find.v2 = mesh.faces[i].vertices[0];
		halfEdge2find.pair = NULL;

		vector<Mesh::HalfEdge>::iterator pair = find(mesh.halfEdge.begin(), mesh.halfEdge.end(), halfEdge2find);

		if (pair != mesh.halfEdge.end())
		{
			halfEdge.pair = &(*pair);
		}
		mesh.halfEdge.push_back(halfEdge);
		mesh.verticesSubdiv[mesh.faces[i].vertices[0]].edge = &mesh.halfEdge[mesh.halfEdge.size() - 1];

		//============================================================================================

		halfEdge.v1 = mesh.faces[i].vertices[1];
		halfEdge.v2 = mesh.faces[i].vertices[2];
		halfEdge.vertex = &mesh.verticesSubdiv[mesh.faces[i].vertices[2]];
		halfEdge.face = &mesh.faces[i];
		halfEdge.pair = NULL;

		halfEdge2find.v1 = mesh.faces[i].vertices[2];
		halfEdge2find.v2 = mesh.faces[i].vertices[1];
		halfEdge2find.pair = NULL;

		pair = find(mesh.halfEdge.begin(), mesh.halfEdge.end(), halfEdge2find);

		if (pair != mesh.halfEdge.end())
		{
			halfEdge.pair = &(*pair);
		}

		mesh.halfEdge.push_back(halfEdge);
		mesh.verticesSubdiv[mesh.faces[i].vertices[1]].edge = &mesh.halfEdge[mesh.halfEdge.size() - 1];

		//============================================================================================

		halfEdge.v1 = mesh.faces[i].vertices[2];
		halfEdge.v2 = mesh.faces[i].vertices[0];
		halfEdge.vertex = &mesh.verticesSubdiv[mesh.faces[i].vertices[0]];
		halfEdge.face = &mesh.faces[i];
		halfEdge.pair = NULL;
		halfEdge2find.v1 = mesh.faces[i].vertices[0];
		halfEdge2find.v2 = mesh.faces[i].vertices[2];
		halfEdge2find.pair = NULL;

		pair = find(mesh.halfEdge.begin(), mesh.halfEdge.end(), halfEdge2find);

		if (pair != mesh.halfEdge.end())
		{
			halfEdge.pair = &(*pair);
		}

		mesh.halfEdge.push_back(halfEdge);
		mesh.verticesSubdiv[mesh.faces[i].vertices[2]].edge = &mesh.halfEdge[mesh.halfEdge.size() - 1];

		mesh.halfEdge[mesh.halfEdge.size() - 3].next = &mesh.halfEdge[mesh.halfEdge.size() - 2];
		mesh.halfEdge[mesh.halfEdge.size() - 2].next = &mesh.halfEdge[mesh.halfEdge.size() - 1];
		mesh.halfEdge[mesh.halfEdge.size() - 1].next = &mesh.halfEdge[mesh.halfEdge.size() - 3];

		if (mesh.halfEdge[mesh.halfEdge.size() - 3].pair != NULL)
		{
			mesh.halfEdge[mesh.halfEdge.size() - 3].pair->pair = &mesh.halfEdge[mesh.halfEdge.size() - 3];
		}

		if (mesh.halfEdge[mesh.halfEdge.size() - 2].pair != NULL)
		{
			mesh.halfEdge[mesh.halfEdge.size() - 2].pair->pair = &mesh.halfEdge[mesh.halfEdge.size() - 2];
		}

		if (mesh.halfEdge[mesh.halfEdge.size() - 1].pair != NULL)
		{
			mesh.halfEdge[mesh.halfEdge.size() - 1].pair->pair = &mesh.halfEdge[mesh.halfEdge.size() - 1];
		}

		mesh.faces[i].edge = &mesh.halfEdge[mesh.halfEdge.size() - 3];
	}

	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);
	glGenVertexArrays(1, &mesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Mesh::Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const void*)(sizeof(glm::vec3)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return mesh;
}

void saveMesh(string fileName)
{
	ofstream stream(fileName+".obj");
	
	stream << "o finom" << endl << endl;

	for (int i = 0; i < _mesh.vertices.size(); i++)
	{
		stream << "v " << _mesh.vertices[i].position.x << " " << _mesh.vertices[i].position.y << " " << _mesh.vertices[i].position.z << endl;
	}

	stream << endl;
	
	for (int i = 0; i < _mesh.faces.size(); i++)
	{
		stream << "f " << _mesh.faces[i].vertices[0]+1 << " " << _mesh.faces[i].vertices[1]+1 << " " << _mesh.faces[i].vertices[2]+1 << endl;
	}
	
	stream << endl;

	stream.close();
}

void subdivideLoop()
{
	int count2 = _mesh.verticesSubdiv.size();

	vector<Mesh::VertexSubdiv> newVertices;
	newVertices = _mesh.verticesSubdiv;
	vector<Mesh::HalfEdge> newEdges;
	vector<Mesh::Face> newFaces;
	int count = newVertices.size();

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		_mesh.halfEdge[h].used = false;
	}

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		if (!_mesh.halfEdge[h].used)
		{
			glm::vec3 u = (glm::vec3(_mesh.halfEdge[h].vertex->position) + glm::vec3(_mesh.halfEdge[h].pair->vertex->position)) * 3.0f / 8.0f + (glm::vec3(_mesh.halfEdge[h].next->vertex->position) + glm::vec3(_mesh.halfEdge[h].pair->next->vertex->position)) / 8.0f;

			Mesh::VertexSubdiv temp;
			temp.position = u;
			temp.normal = glm::vec3(0, 0, 0);
			temp.edge = NULL;
			newVertices.push_back(temp);
			_mesh.halfEdge[h].used = true;
			_mesh.halfEdge[h].pair->used = true;
		}
	}

	vector<glm::vec3> newPositions;

	for (int i = 0; i < count2; i++)
	{
		int n = 0;
		Mesh::HalfEdge* start = _mesh.verticesSubdiv[i].edge;
		Mesh::HalfEdge* current = _mesh.verticesSubdiv[i].edge;
		glm::vec3 s = { 0,0,0 };

		current = current->next->next->pair;
		s += current->next->vertex->position;
		n++;

		while (start != current)
		{
			current = current->next->next->pair;
			s += current->next->vertex->position;
			n++;
		}

		float beta = pow((3.0f / 8.0f + cos(2 * glm::pi<float>() / n) / 4.0f), 2);
		float alpha = (5.0f / 8.0f - beta) / n;

		newPositions.push_back((1 - n * alpha) * newVertices[i].position + alpha * s);
	}

	for (int i = 0; i < newPositions.size(); i++)
	{
		newVertices[i].position = newPositions[i];
	}

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		_mesh.halfEdge[h].used = false;
	}

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		if (!_mesh.halfEdge[h].used)
		{
			_mesh.halfEdge[h].pair->vertex = &newVertices[count];
			_mesh.halfEdge[h].vertex = &newVertices[count];
			_mesh.halfEdge[h].used = true;
			_mesh.halfEdge[h].pair->used = true;
			count++;
		}
	}

	newFaces.clear();

	for (int i = 0; i < _mesh.faces.size(); i++)
	{
		Mesh::Face face;
		Mesh::VertexSubdiv* v0 = &newVertices[_mesh.faces[i].vertices[0]];
		Mesh::VertexSubdiv* v1 = &newVertices[_mesh.faces[i].vertices[1]];
		Mesh::VertexSubdiv* v2 = &newVertices[_mesh.faces[i].vertices[2]];
		Mesh::VertexSubdiv* j0 = &newVertices[4];
		Mesh::VertexSubdiv* j1 = &newVertices[5];
		Mesh::VertexSubdiv* j2 = &newVertices[6];

		j0 = _mesh.faces[i].edge->vertex;
		j1 = _mesh.faces[i].edge->next->vertex;
		j2 = _mesh.faces[i].edge->next->next->vertex;

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j0));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j1));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j2));

		newFaces.push_back(face);

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *v0));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j0));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j2));

		newFaces.push_back(face);

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *v1));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j1));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j0));

		newFaces.push_back(face);

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *v2));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j2));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j1));

		newFaces.push_back(face);
	}

	_mesh.verticesSubdiv = newVertices;
	_mesh.faces = newFaces;
}

void subdivideKobbelt()
{
	int count2 = _mesh.verticesSubdiv.size();
	vector<Mesh::VertexSubdiv> newVertices;
	newVertices = _mesh.verticesSubdiv;
	vector<Mesh::HalfEdge> newEdges;
	vector<Mesh::Face> newFaces;
	int count = newVertices.size();
	vector<glm::vec3> newPositions;

	for (int i = 0; i < count2; i++)
	{
		int n = 0;
		Mesh::HalfEdge* start = _mesh.verticesSubdiv[i].edge;
		Mesh::HalfEdge* current = _mesh.verticesSubdiv[i].edge;
		glm::vec3 s = { 0,0,0 };

		current = current->next->next->pair;
		s += current->next->vertex->position;
		n++;

		while (start != current)
		{
			current = current->next->next->pair;
			s += current->next->vertex->position;
			n++;
		}

		float beta = pow((3.0f / 8.0f + cos(2 * glm::pi<float>() / n) / 4.0f), 2);
		float alpha = (5.0f / 8.0f - beta) / n;

		newPositions.push_back((1 - n * alpha) * newVertices[i].position + alpha * s);
	}

	for (int i = 0; i < newPositions.size(); i++)
	{
		newVertices[i].position = newPositions[i];
	}

	for (int i = 0; i < _mesh.faces.size(); i++)
	{
		glm::vec3 u = (_mesh.vertices[_mesh.faces[i].vertices[0]].position + _mesh.vertices[_mesh.faces[i].vertices[1]].position + _mesh.vertices[_mesh.faces[i].vertices[2]].position) / glm::vec3(3, 3, 3);
		Mesh::VertexSubdiv temp;
		temp.normal = glm::vec3(0, 0, 0);
		temp.edge = NULL;
		temp.position = u;
		newVertices.push_back(temp);
		_mesh.faces[i].centerPoint = newVertices.size() - 1;
	}

	newFaces.clear();

	for (int i = 0; i < _mesh.halfEdge.size(); i++)
	{
		Mesh::Face face;

		face.vertices[0] = _mesh.halfEdge[i].v2;
		face.vertices[1] = _mesh.halfEdge[i].face->centerPoint;
		face.vertices[2] = _mesh.halfEdge[i].pair->face->centerPoint;

		newFaces.push_back(face);
	}

	_mesh.verticesSubdiv = newVertices;
	_mesh.faces = newFaces;
}

void subdivideButterfly()
{
	vector<Mesh::VertexSubdiv> newVertices;
	newVertices = _mesh.verticesSubdiv;
	vector<Mesh::HalfEdge> newEdges;

	vector<Mesh::Face> newFaces;
	int count = newVertices.size();

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		_mesh.halfEdge[h].used = false;
	}

	float w = 1.0 / 8.0;

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		if (!_mesh.halfEdge[h].used)
		{
			glm::vec3 u = (0.5f * (_mesh.halfEdge[h].vertex->position + _mesh.halfEdge[h].pair->vertex->position)) + (w * (_mesh.halfEdge[h].next->vertex->position + _mesh.halfEdge[h].pair->next->vertex->position)) -
				((w / 2.0f) * (_mesh.halfEdge[h].next->pair->next->vertex->position + _mesh.halfEdge[h].next->next->pair->next->vertex->position + _mesh.halfEdge[h].pair->next->pair->next->vertex->position + _mesh.halfEdge[h].pair->next->next->pair->next->vertex->position));

			Mesh::VertexSubdiv temp;
			temp.position = u;
			temp.normal = glm::vec3(0, 0, 0);
			temp.edge = NULL;
			newVertices.push_back(temp);
			_mesh.halfEdge[h].used = true;
			_mesh.halfEdge[h].pair->used = true;
		}
	}

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		_mesh.halfEdge[h].used = false;
	}

	for (int h = 0; h < _mesh.halfEdge.size(); h++)
	{
		if (!_mesh.halfEdge[h].used)
		{
			_mesh.halfEdge[h].pair->vertex = &newVertices[count];
			_mesh.halfEdge[h].vertex = &newVertices[count];
			_mesh.halfEdge[h].used = true;
			_mesh.halfEdge[h].pair->used = true;
			count++;
		}
	}

	newFaces.clear();

	for (int i = 0; i < _mesh.faces.size(); i++)
	{
		Mesh::Face face;
		Mesh::VertexSubdiv* v0 = &newVertices[_mesh.faces[i].vertices[0]];
		Mesh::VertexSubdiv* v1 = &newVertices[_mesh.faces[i].vertices[1]];
		Mesh::VertexSubdiv* v2 = &newVertices[_mesh.faces[i].vertices[2]];
		Mesh::VertexSubdiv* j0 = &newVertices[4];
		Mesh::VertexSubdiv* j1 = &newVertices[5];
		Mesh::VertexSubdiv* j2 = &newVertices[6];

		j0 = _mesh.faces[i].edge->vertex;
		j1 = _mesh.faces[i].edge->next->vertex;
		j2 = _mesh.faces[i].edge->next->next->vertex;

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j0));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j1));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j2));

		newFaces.push_back(face);

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *v0));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j0));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j2));

		newFaces.push_back(face);

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *v1));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j1));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j0));

		newFaces.push_back(face);

		face.vertices[0] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *v2));
		face.vertices[1] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j2));
		face.vertices[2] = abs(newVertices.begin() - find(newVertices.begin(), newVertices.end(), *j1));

		newFaces.push_back(face);
	}

	_mesh.verticesSubdiv = newVertices;
	_mesh.faces = newFaces;
}