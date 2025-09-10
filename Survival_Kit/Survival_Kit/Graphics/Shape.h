// Utilizes vertex SoA to create meshes that are simple shapes

#pragma once

#ifndef __SHAPE_H__
#define __SHAPE_H__
#include "../Graphics/Vertex.h"
#include "../Graphics/GLResources.h"

namespace gam300 {

	class Cube {

	public:

		void init();
		void draw();

	private:
		VertexSoA vertices;
		VAO       vao;
		VBO       vbo;
		VBO       ebo;

		GLenum    primitive_type;
		GLuint    draw_count;
	};

}


#endif // !__SHAPE_H__
