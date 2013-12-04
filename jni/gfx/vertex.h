/*
 * vertex.h
 *
 *  Created on: 2013/12/04
 *      Author: Michael
 */

#ifndef VERTEX_H_
#define VERTEX_H_

#include <GLES/gl.h>


struct vertex{
    GLfloat x, y, z;
    GLfloat r, g, b;
};

struct vertex_rgb{
	GLfloat r, g, b;
};


struct vec2{

	float x;
	float y;

};

#endif /* VERTEX_H_ */