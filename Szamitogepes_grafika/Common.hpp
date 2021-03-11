#pragma once

#pragma warning (disable: 26451)
#pragma warning (disable: 26495)

#define GLEW_STATIC
#define GLM_FORCE_RADIANS

#include <gl\glew.h>
#include "GLFW\glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "stb_image.h"