#pragma once

#pragma warning( disable : 4127 )



#include <glad/glad.h>


#include <GLFW/glfw3.h> // GLFW helper library




#include <vector>
#include "gl_util.hpp"

/*
constexpr GLuint VBO_POSITION_ATTRIB_INDEX = 0;
constexpr GLuint VBO_TEX_COORD_ATTRIB_INDEX = 2;
constexpr GLuint VBO_NORMAL_ATTRIB_INDEX = 1;
constexpr GLuint VBO_COLOR_ATTRIB_INDEX = 3;

constexpr GLuint VBO_SLOT0_ATTRIB_INDEX = 4;
constexpr GLuint VBO_SLOT1_ATTRIB_INDEX = 5;
constexpr GLuint VBO_SLOT2_ATTRIB_INDEX = 6;
constexpr GLuint VBO_SLOT3_ATTRIB_INDEX = 7;
*/

typedef std::vector<GLushort> UshortVector;
typedef std::vector<GLfloat> FloatVector;
typedef std::vector<GLuint> UintVector;
