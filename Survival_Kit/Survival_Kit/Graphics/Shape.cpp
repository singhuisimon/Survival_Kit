#include "../Graphics/Shape.h"
#include "../Manager/GraphicsManager.h"
#include <string>

namespace gam300 {

	// Self contained shaders
	std::string vtx_shd // Vertex Shader
	{
		R"(#version 450 core
			layout(location = 0) in vec3 aVertexPosition;
			layout(location = 1) in vec3 aVertexColor;
			layout(location = 0) out vec3 vColor;
			void main()
			{
				gl_Position = vec4(aVertexPosition, 1.0);
				vColor = aVertexColor;
			}
			)"
	};

	std::string frag_shd // Fragment Shader
	{
		R"( #version 450 core
			layout (location=0) in vec3 vInterpColor;
			layout (location=0) out vec4 fFragColor;
			void main () 
			{
				fFragColor = vec4(vInterpColor, 1.0);
			}
			)"
	};

	void Cube::init() {

		vertices.positions = {
            // +X face
			{+0.5f,-0.5f,-0.5f}, {+0.5f,-0.5f,+0.5f}, {+0.5f,+0.5f,+0.5f}, {+0.5f,+0.5f,-0.5f},
			// -X face
			{-0.5f,-0.5f,+0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f,+0.5f,-0.5f}, {-0.5f,+0.5f,+0.5f},
			// +Y face
			{-0.5f,+0.5f,-0.5f}, {+0.5f,+0.5f,-0.5f}, {+0.5f,+0.5f,+0.5f}, {-0.5f,+0.5f,+0.5f},
			// -Y face
			{-0.5f,-0.5f,+0.5f}, {+0.5f,-0.5f,+0.5f}, {+0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f},
			// +Z face
			{+0.5f,-0.5f,+0.5f}, {-0.5f,-0.5f,+0.5f}, {-0.5f,+0.5f,+0.5f}, {+0.5f,+0.5f,+0.5f},
			// -Z face
			{-0.5f,-0.5f,-0.5f}, {+0.5f,-0.5f,-0.5f}, {+0.5f,+0.5f,-0.5f}, {-0.5f,+0.5f,-0.5f}
		};

		vertices.colors = {

			// +X → red
			{1,0,0}, {1,0,0}, {1,0,0}, {1,0,0},
			// -X → green
			{0,1,0}, {0,1,0}, {0,1,0}, {0,1,0},
			// +Y → blue
			{0,0,1}, {0,0,1}, {0,0,1}, {0,0,1},
			// -Y → yellow
			{1,1,0}, {1,1,0}, {1,1,0}, {1,1,0},
			// +Z → magenta
			{1,0,1}, {1,0,1}, {1,0,1}, {1,0,1},
			// -Z → cyan
			{0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}
		};

		// Create buffers
		vbo.create();
		
		// Calculate how much size we need to allocate for vbo

		// Calculate values for ease of packing
		GLsizei position_data_offset = 0;
		GLsizei position_attribute_size = sizeof(glm::vec3);
		GLsizei position_data_size = position_attribute_size * static_cast<GLsizei>(vertices.positions.size());

		GLsizei color_data_offset = position_data_size;
		GLsizei color_attribute_size = sizeof(glm::vec3);
		GLsizei color_data_size = color_attribute_size * static_cast<GLsizei>(vertices.colors.size());

		GLsizei buffer_size = position_data_size + color_data_size;

		// Wrapper for named buffer storage
		vbo.storage(buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

		// Load data into sub buffer		
		vbo.sub_data(position_data_offset, position_data_size, vertices.positions.data());
		vbo.sub_data(color_data_offset, color_data_size, vertices.colors.data());

		// Set up the VAO
		vao.create();

		// Bind the vertex array for the position
		vao.enable_attrib(0);
		vao.bind_vertex_buffer(0, vbo, position_data_offset, position_attribute_size);
		vao.attrib_format(0, 3, GL_FLOAT, false, 0);
		vao.attrib_binding(0, 0);
		glBindVertexArray(0);

		// Bind the vertex array for the colors
		vao.enable_attrib(1);
		vao.bind_vertex_buffer(1, vbo, color_data_offset, color_attribute_size);
		vao.attrib_format(1, 3, GL_FLOAT, false, 0);
		vao.attrib_binding(1, 1);
		glBindVertexArray(0);

		std::vector<GLushort> indices = {
			//// +X (base 0):   0,2,1,3
			//0, 2, 1, 3,
			//// bridge to -X
			//3, 4, 4,
			//// -X (base 4):   4,6,5,7
			//6, 5, 7,
			//// bridge to +Y
			//7, 8, 8,
			//// +Y (base 8):   8,10,9,11
			//10, 9, 11,
			//// bridge to -Y
			//11, 12, 12,
			//// -Y (base 12):  12,14,13,15
			//14, 13, 15,
			//// bridge to +Z
			//15, 16, 16,
			//// +Z (base 16):  16,18,17,19
			//18, 17, 19,
			//// bridge to -Z
			//19, 20, 20,
			//// -Z (base 20):  20,22,21,23
			//22, 21, 23
			 
			// KENNY TESTING:
			// front (0..3)
			0,1,2,  0,2,3,
			// back (4..7)  (note the swap to keep CCW from *outside*)
			4,6,5,  4,7,6,
			// left (8..11)
			8,9,10,  8,10,11,
			// right (12..15)
			12,14,13,  12,15,14,
			// bottom (16..19)
			16,17,18,  16,18,19,
			// top (20..23)
			20,22,21,  20,23,22
		};
		
		// Create an element buffer object to transfer topology
		ebo.create();
		ebo.storage(sizeof(GLushort)* indices.size(), reinterpret_cast<GLvoid*>(indices.data()), GL_DYNAMIC_STORAGE_BIT);
		vao.bind_element_buffer(ebo);
		glBindVertexArray(0);

		primitive_type = GL_TRIANGLES;
		draw_count	   = indices.size();
	}

	GLuint Cube::GetVAOId() const noexcept {

		return vao.id();
	}
}