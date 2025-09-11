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
		GLuint GetVAOId() const noexcept;

		GLenum    primitive_type;
		GLuint    draw_count;

		VAO       vao;
		VBO       vbo;
		VBO       ebo;

	private:
		VertexSoA vertices;



	};

}


#endif // !__SHAPE_H__
