#pragma once

#include "Common.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <array>

using namespace std;

string loadTextFile(const string& fileName);

GLuint loadShader(const string& fileName, GLenum type);

GLuint loadProgram(const string& baseName);