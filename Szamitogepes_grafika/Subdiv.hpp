#pragma once

#include "Common.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <array>

using namespace std;

struct Mesh {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
	};

	struct VertexSubdiv;
	struct Face;
	struct HalfEdge;

	struct HalfEdge {
		HalfEdge* pair = NULL;
		HalfEdge* next = NULL;
		Face* face;
		VertexSubdiv* vertex; //VÉGPONT
		int v1, v2;
		bool used = false;

		bool operator==(HalfEdge b)
		{
			return b.v1 == this->v1 && b.v2 == this->v2;
		}
	};

	struct VertexSubdiv {
		glm::vec3 position;
		glm::vec3 normal;
		HalfEdge* edge; //KIINDULÓ

		bool operator==(VertexSubdiv b)
		{
			return b.position == this->position;
		}
	};

	struct Face {
		int vertices[3];
		HalfEdge* edge;
		int centerPoint;
	};

	GLuint vbo, vao, ibo;

	vector<glm::vec3> normals;
	vector<GLuint> Normalindices;

	vector<GLuint> indices;

	vector<Vertex> vertices;

	vector<VertexSubdiv> verticesSubdiv;
	vector<Face> faces;
	vector<HalfEdge> halfEdge;

	void loadSubdivData()
	{
		indices.clear();

		for (int i = 0; i < faces.size(); i++)
		{
			indices.push_back(faces[i].vertices[0]);
			indices.push_back(faces[i].vertices[1]);
			indices.push_back(faces[i].vertices[2]);

			glm::vec3 normal = glm::normalize(glm::cross(verticesSubdiv[faces[i].vertices[0]].position - verticesSubdiv[faces[i].vertices[1]].position, verticesSubdiv[faces[i].vertices[0]].position - verticesSubdiv[faces[i].vertices[2]].position));

			verticesSubdiv[faces[i].vertices[0]].normal = normal;
			verticesSubdiv[faces[i].vertices[1]].normal = normal;
			verticesSubdiv[faces[i].vertices[2]].normal = normal;
		}

		vertices.clear();

		for (int i = 0; i < verticesSubdiv.size(); i++)
		{
			Vertex vertex;
			vertex.position = verticesSubdiv[i].position;
			vertex.normal = verticesSubdiv[i].normal;
			vertices.push_back(vertex);
		}

		this->halfEdge.clear();

		for (int i = 0; i < this->faces.size(); i++)
		{
			Mesh::HalfEdge halfEdge;
			halfEdge.v1 = this->faces[i].vertices[0];
			halfEdge.v2 = this->faces[i].vertices[1];
			halfEdge.vertex = &this->verticesSubdiv[this->faces[i].vertices[1]];
			halfEdge.face = &this->faces[i];

			Mesh::HalfEdge halfEdge2find;
			halfEdge2find.v1 = this->faces[i].vertices[1];
			halfEdge2find.v2 = this->faces[i].vertices[0];
			halfEdge2find.pair = NULL;

			vector<Mesh::HalfEdge>::iterator pair = find(this->halfEdge.begin(), this->halfEdge.end(), halfEdge2find);

			if (pair != this->halfEdge.end())
			{
				halfEdge.pair = &(*pair);
			}

			this->halfEdge.push_back(halfEdge);
			this->verticesSubdiv[this->faces[i].vertices[0]].edge = &this->halfEdge[this->halfEdge.size() - 1];

			//============================================================================================

			halfEdge.v1 = this->faces[i].vertices[1];
			halfEdge.v2 = this->faces[i].vertices[2];
			halfEdge.vertex = &this->verticesSubdiv[this->faces[i].vertices[2]];
			halfEdge.face = &this->faces[i];
			halfEdge.pair = NULL;

			halfEdge2find.v1 = this->faces[i].vertices[2];
			halfEdge2find.v2 = this->faces[i].vertices[1];
			halfEdge2find.pair = NULL;

			pair = find(this->halfEdge.begin(), this->halfEdge.end(), halfEdge2find);

			if (pair != this->halfEdge.end())
			{
				halfEdge.pair = &(*pair);
			}

			this->halfEdge.push_back(halfEdge);
			this->verticesSubdiv[this->faces[i].vertices[1]].edge = &this->halfEdge[this->halfEdge.size() - 1];

			//============================================================================================

			halfEdge.v1 = this->faces[i].vertices[2];
			halfEdge.v2 = this->faces[i].vertices[0];
			halfEdge.vertex = &this->verticesSubdiv[this->faces[i].vertices[0]];
			halfEdge.face = &this->faces[i];
			halfEdge.pair = NULL;
			halfEdge2find.v1 = this->faces[i].vertices[0];
			halfEdge2find.v2 = this->faces[i].vertices[2];
			halfEdge2find.pair = NULL;

			pair = find(this->halfEdge.begin(), this->halfEdge.end(), halfEdge2find);

			if (pair != this->halfEdge.end())
			{
				halfEdge.pair = &(*pair);
			}

			this->halfEdge.push_back(halfEdge);
			this->verticesSubdiv[this->faces[i].vertices[2]].edge = &this->halfEdge[this->halfEdge.size() - 1];

			this->halfEdge[this->halfEdge.size() - 3].next = &this->halfEdge[this->halfEdge.size() - 2];
			this->halfEdge[this->halfEdge.size() - 2].next = &this->halfEdge[this->halfEdge.size() - 1];
			this->halfEdge[this->halfEdge.size() - 1].next = &this->halfEdge[this->halfEdge.size() - 3];

			if (this->halfEdge[this->halfEdge.size() - 3].pair != NULL)
			{
				this->halfEdge[this->halfEdge.size() - 3].pair->pair = &this->halfEdge[this->halfEdge.size() - 3];
			}

			if (this->halfEdge[this->halfEdge.size() - 2].pair != NULL)
			{
				this->halfEdge[this->halfEdge.size() - 2].pair->pair = &this->halfEdge[this->halfEdge.size() - 2];
			}

			if (this->halfEdge[this->halfEdge.size() - 1].pair != NULL)
			{
				this->halfEdge[this->halfEdge.size() - 1].pair->pair = &this->halfEdge[this->halfEdge.size() - 1];
			}

			this->faces[i].edge = &this->halfEdge[this->halfEdge.size() - 3];
		}
	}
};

Mesh loadMesh(string fileName);

void saveMesh(string fileName);

void subdivideLoop();

void subdivideKobbelt();

void subdivideButterfly();