#pragma warning (disable: 26451)
#pragma warning (disable: 26495)

#include "Shader.hpp"
#include "Subdiv.hpp"

#include <algorithm>
#include <memory>
#include <ctime>
#include <cmath>

#define _USE_MATH_DEFINES

using namespace std;

bool isKeyPressedK = false;
bool isKeyPressedL = false;
bool isKeyPressedB = false;
bool isKeyPressedM = false;
bool isKeyPressedF = false;
bool displayMesh = false;

glm::mat4 trans2;

struct UniformDataModel
{
	glm::mat4 m_modelView;
	glm::mat4 m_view;
	glm::mat4 m_normal;
	glm::mat4 m_mvp;
	glm::mat4 m_rotate;
};

array<glm::mat4, 1> _models =
{
	glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(1.0f)),
};

static GLuint _uboModel = 0, _uboMaterial = 0, _uboLight = 0;
static int _width = 1366, _height = 768;
static GLuint _vboCube = 0, _iboCube = 0, _vaoCube = 0;
static GLFWwindow* _window = NULL;

/** Frissítések közötti minimális idő. */
static double _updateDeltaTime = 1.0 / 60.0;

extern Mesh _mesh;

static GLuint _program = 0;
static GLuint boundingbox = 2;

static glm::mat4 _view, _projection;

static glm::vec3 _eye = glm::vec3(0.0f, 0.0f, 3.0f), _forward = glm::vec3(0.0f), _right = glm::vec3(0.0f), _up = glm::vec3(0.0f);

static GLfloat _phi = glm::radians(90.0f), _theta = glm::radians(270.0f);

static GLfloat _moveSpeed = 2.0f, _turnSpeed = glm::radians(3.0f);

static double _mouseX = -1.0, _mouseY = -1.0;

GLuint vbo, ibo, vao;

float radians = 0;

//=====================================================================================================================

void computeCameraMatrices()
{
	_forward = glm::vec3(
		glm::cos(_theta) * glm::sin(_phi),
		glm::cos(_phi),
		glm::sin(_theta) * glm::sin(_phi)
	);
	_right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), _forward));
	_up = glm::normalize(glm::cross(_forward, _right));

	_view = glm::lookAt(_eye, _eye + _forward, glm::vec3(0.0f, 1.0f, 0.0f));
	_projection = glm::perspective(glm::radians(55.0f), (float)_width / (float)_height, 0.01f, 20.0f);
}

//=====================================================================================================================

void initScene()
{
	computeCameraMatrices();

	/** Betöltjük a rajzoláshoz használandó shader programot. */
	_program = loadProgram("shader");
	boundingbox = loadProgram("bounds");

	/** Betöltjük a mesht. */
	_mesh = loadMesh("test3.obj");
	cout << "Loading done." << endl;

	glGenBuffers(1, &_uboModel);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glGenVertexArrays(1, &vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(Mesh::Vertex), _mesh.vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(GLuint), _mesh.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	/** Konfiguráljuk a VAO-t. */
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (const void*)(sizeof(glm::vec3)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void cleanUpScene()
{
	/** Töröljük a submesheket. */

	if (_mesh.vao != 0)
	{
		glDeleteVertexArrays(1, &_mesh.vao);
	}

	if (_mesh.vbo != 0)
	{
		glDeleteBuffers(1, &_mesh.vbo);
	}

	if (_mesh.ibo != 0)
	{
		glDeleteBuffers(1, &_mesh.ibo);
	}

	/** Töröljük a shader programot. */
	glDeleteProgram(_program);
}

void renderScene()
{
	glClearColor(0.2f, 0.6f, 0.3f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUseProgram(_program);

	for (size_t i = 0; i < _models.size(); ++i)
	{
		UniformDataModel modelData;
		modelData.m_modelView = _view * _models[i];
		modelData.m_view = _view;
		modelData.m_normal = glm::inverseTranspose(modelData.m_modelView);
		modelData.m_mvp = _projection * modelData.m_modelView;
		modelData.m_rotate = trans2;
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _uboModel);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformDataModel), &modelData, GL_STREAM_DRAW);

		glBindVertexArray(_mesh.vao);
		glPointSize(10);
		glDrawElements(GL_TRIANGLES, _mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
	}

	glBindVertexArray(0);

	//===========================================================================

	if (displayMesh)
	{
		glUseProgram(boundingbox);
		glBindVertexArray(_mesh.vao);

		glLineWidth(4);
		glPointSize(5);

		for (size_t i = 0; i < _models.size(); ++i)
		{
			glm::mat4 mvp = _projection * _view * _models[i];
			glm::vec3 boundsColor = glm::vec3(0.0f, 1.0f, 0.0f);
			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
			glUniform3fv(1, 1, glm::value_ptr(boundsColor));

			glDrawElements(GL_POINTS, _mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
		}

		glBindVertexArray(0);

		//===========================================================================

		glLineWidth(3);
		glPointSize(10);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(boundingbox);

		for (size_t i = 0; i < _models.size(); ++i)
		{
			UniformDataModel modelData;
			modelData.m_modelView = _view * _models[i];
			modelData.m_view = _view;
			modelData.m_normal = glm::inverseTranspose(modelData.m_modelView);
			modelData.m_mvp = _projection * modelData.m_modelView;
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, _uboModel);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformDataModel), &modelData, GL_STREAM_DRAW);

			glm::vec3 boundsColor = glm::vec3(1.0f, 0.0f, 0.0f);
			glUniform3fv(1, 1, glm::value_ptr(boundsColor));

			glBindVertexArray(_mesh.vao);

			glDrawElements(GL_TRIANGLES, _mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
		}
		glBindVertexArray(0);
	}
}

void updateScene(double delta)
{
	radians += 0.5f;
	glm::mat4 trans = glm::mat4(1.0f);
	trans2 = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
	_models[0] = _models[0] * trans;
	trans2 = glm::rotate(trans2, glm::radians(radians), glm::vec3(0.0f, 1.0f, 0.0f));

	/** Escape billentyű. */
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(_window, true);
	}

	/** Mozgás kezelése. */
	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		_eye += _forward * _moveSpeed * (float)delta;
	}

	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		_eye -= _forward * _moveSpeed * (float)delta;
	}

	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		_eye += _right * _moveSpeed * (float)delta;
	}

	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		_eye -= _right * _moveSpeed * (float)delta;
	}

	if (glfwGetKey(_window, GLFW_KEY_M) == GLFW_PRESS)
	{
		if (!isKeyPressedM)
		{
			displayMesh = !displayMesh;
			isKeyPressedM = true;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_F) == GLFW_PRESS)
	{
		if (!isKeyPressedF)
		{
			saveMesh("finomitott");
			isKeyPressedF = true;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_L) == GLFW_PRESS)
	{
		if (!isKeyPressedL)
		{
			subdivideLoop();

			_mesh.loadSubdivData();

			glBindBuffer(GL_ARRAY_BUFFER, _mesh.vbo);
			glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(Mesh::Vertex), _mesh.vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh.ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(GLuint), _mesh.indices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			isKeyPressedL = true;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_K) == GLFW_PRESS)
	{
		if (!isKeyPressedK)
		{
			subdivideKobbelt();

			_mesh.loadSubdivData();

			glBindBuffer(GL_ARRAY_BUFFER, _mesh.vbo);
			glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(Mesh::Vertex), _mesh.vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh.ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(GLuint), _mesh.indices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			isKeyPressedK = true;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_B) == GLFW_PRESS)
	{
		if (!isKeyPressedB)
		{
			subdivideButterfly();

			_mesh.loadSubdivData();

			glBindBuffer(GL_ARRAY_BUFFER, _mesh.vbo);
			glBufferData(GL_ARRAY_BUFFER, _mesh.vertices.size() * sizeof(Mesh::Vertex), _mesh.vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh.ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _mesh.indices.size() * sizeof(GLuint), _mesh.indices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			isKeyPressedB = true;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_L) == GLFW_RELEASE && isKeyPressedL)
	{
		isKeyPressedL = false;
	}

	if (glfwGetKey(_window, GLFW_KEY_K) == GLFW_RELEASE && isKeyPressedK)
	{
		isKeyPressedK = false;
	}

	if (glfwGetKey(_window, GLFW_KEY_B) == GLFW_RELEASE && isKeyPressedB)
	{
		isKeyPressedB = false;
	}

	if (glfwGetKey(_window, GLFW_KEY_M) == GLFW_RELEASE && isKeyPressedM)
	{
		isKeyPressedM = false;
	}

	if (glfwGetKey(_window, GLFW_KEY_F) == GLFW_RELEASE && isKeyPressedF)
	{
		isKeyPressedF = false;
	}

	/** Forgás kezelése. */
	double mouseX, mouseY;
	glfwGetCursorPos(_window, &mouseX, &mouseY);

	if (_mouseX != -1.0)
	{
		float deltaX = (float)(mouseX - _mouseX);
		float deltaY = (float)(mouseY - _mouseY);

		_phi += _turnSpeed * delta * deltaY;
		_theta += _turnSpeed * delta * deltaX;
	}

	_mouseX = mouseX;
	_mouseY = mouseY;

	/** Újraszámítjuk a kamera mátrixokat. */
	computeCameraMatrices();
}

//=====================================================================================================================

void glfwErrorFunc(int errorCode, const char* errorStr)
{
	cerr << "GLFW error: " << errorStr << endl;
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
	/** Eltároljuk az ablakunk méreteit. */
	_width = width;
	_height = height;

	/** Frissítjük a rendereléshez használt területet. */
	glViewport(0, 0, width, height);

	/** Frissítjük a kamera mátrixot, mivel megváltozott az ablak méretaránya. */
	computeCameraMatrices();
}

//=====================================================================================================================

int main(int argc, char** argv)
{
	/** Próbáljuk meg felállítani a GLFW-t. */
	if (!glfwInit())
	{
		cerr << "Failed to initialize GLFW!" << endl;
		return -1;
	}

	/** A kívánt OpenGL profil és verzió (4.3 Core profil) */
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);

	/** Hiba callback függvény. */
	glfwSetErrorCallback(glfwErrorFunc);

	/** Próbáljuk meg létrehozni az ablakunkat. */
	_window = glfwCreateWindow(_width, _height, "Szamitogepes grafika beadando", NULL, NULL);

	if (!_window)
	{
		cerr << "Failed to create a GLFW window!" << endl;
		glfwTerminate();
		return -2;
	}

	/** Kapcsoljuk ki az egérkurzort. */
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/** Callback függvények az egyéb eseményekhez. */
	glfwSetFramebufferSizeCallback(_window, resizeCallback);

	/** Válasszuk ki az ablakunk OpenGL kontextusát, hogy használhassuk. */
	glfwMakeContextCurrent(_window);

	/** Inicializáljuk a GLEW-ot, hogy elérhetővé váljanak az OpenGL függvények. */
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();

	if (glewError != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW, reason: " << glewGetErrorString(glewError) << endl;
		glfwTerminate();
		return -3;
	}

	/** Inicializáljuk az stb_image-et. */
	stbi_set_flip_vertically_on_load(1);

	/** Inicializáljuk a jelenetünket. */
	initScene();

	/** Előző frissítés időpontja. */
	double lastTime = glfwGetTime();

	/** Fő ciklus. */
	while (!glfwWindowShouldClose(_window))
	{
		/** Jelenlegi idő. */
		double currentTime = glfwGetTime();

		/** Eltelt idő. */
		double deltaTime = currentTime - lastTime;

		/** Először az eseményeket kezeljük. */
		glfwPollEvents();

		/** Ha elég idő eltelt, frissítünk és rajzolunk. */
		if (deltaTime > _updateDeltaTime)
		{
			/** Jelenet frissítése (input kezelés, alkalmazáslogika). */
			updateScene(deltaTime);

			/** Jelenet kirajzolása. */
			renderScene();
			glfwSwapBuffers(_window);

			/** Eltároljuk a mostani időpontot, mint a legutolsó frissítés idejét. */
			lastTime = currentTime;
		}
	}

	/** Szabaduljunk meg a már feleslegessé vált objektumoktól. */
	cleanUpScene();

	/** Állítsuk le a GLFW-t. */
	glfwTerminate();

	return 0;
}