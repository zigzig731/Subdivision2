#include "Shader.hpp"

using namespace std;

/** Sztring puffer a shader fordítás loghoz. */
static array<GLchar, 4096> s_logBuffer;

string loadTextFile(const string& fileName)
{
	string result;

	ifstream fstream(fileName);

	if (fstream.good())
	{
		stringstream inputStream;
		inputStream << fstream.rdbuf();
		result = inputStream.str();
	}

	return result;
}

GLuint loadShader(const string& fileName, GLenum type)
{
	/** Töltsük be a kódját. */
	string source = loadTextFile(fileName);

	/** Ha nem létezik, akkor adjunk vissza nullát. */
	if (source.empty())
	{
		return 0;
	}

	/** Hozzuk létre a shader objektumunk. */
	GLuint shader = glCreateShader(type);

	const GLchar* sourcePtr[] = { source.c_str() };

	/** Rendeljük a betöltött kódot a shader objektumhoz. */
	glShaderSource(shader, 1, sourcePtr, NULL);

	/** Végezetül fordítsuk le a shaderhez tartozó kódot. */
	glCompileShader(shader);

	GLint status;

	/** Lekérjük a fordítás eredményét. */
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	/** Ha a fordítás sikertelen, írjuk ki a hiba okát. */
	if (status == GL_FALSE)
	{
		glGetShaderInfoLog(shader, s_logBuffer.size(), NULL, s_logBuffer.data());
		cout << fileName << ": " << s_logBuffer.data() << endl;
	}

	return shader;
}

GLuint loadProgram(const string& baseName)
{
	/** Töltsük be a shadereket. */
	GLuint shaders[] =
	{
		loadShader(baseName + ".tes", GL_TESS_EVALUATION_SHADER),
		loadShader(baseName + ".tcs", GL_TESS_CONTROL_SHADER),
		loadShader(baseName + ".vs", GL_VERTEX_SHADER),
		loadShader(baseName + ".gs", GL_GEOMETRY_SHADER),
		loadShader(baseName + ".fs", GL_FRAGMENT_SHADER),
		loadShader(baseName + ".cs", GL_COMPUTE_SHADER),
	};

	/** Ezek után hozzuk létre az õket összefogó shader programot. */
	GLuint program = glCreateProgram();

	/** Csatoljuk a shadereket. */
	for (int i = 0; i < 6; ++i)
	{
		if (shaders[i] != 0)
		{
			glAttachShader(program, shaders[i]);
		}
	}

	/** Végezetül linkeljük a shader programunkat, hogy használni tudjuk. */
	glLinkProgram(program);

	GLint status;

	/** Lekérjük a linkelés eredményét. */
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	/** Ha a linkelés sikertelen, írjuk ki a hiba okát. */
	if (status == GL_FALSE)
	{
		glGetProgramInfoLog(program, s_logBuffer.size(), NULL, s_logBuffer.data());
		cout << baseName << ": " << s_logBuffer.data() << endl;
	}

	/**  A program sikeresen linkelve lett, így leválaszthatjuk a shadereket a programról, majd törölhetjük őket. */
	for (int i = 0; i < 6; ++i)
	{
		if (shaders[i] != 0)
		{
			glDetachShader(program, shaders[i]);
			glDeleteShader(shaders[i]);
		}
	}

	/** Visszaadjuk a kész programot. */
	return program;
}