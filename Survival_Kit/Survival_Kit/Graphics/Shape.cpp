#include "../Graphics/Shape.h"
#include "../Manager/GraphicsManager.h"
#include <string>

namespace gam300 {

	namespace Shape {

		MeshData make_cube() {

			MeshData m;

			m.positions = {

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

			m.normals = {

				// +X
				{+1,0,0}, {+1,0,0}, {+1,0,0}, {+1,0,0},
				// -X
				{-1,0,0}, {-1,0,0}, {-1,0,0}, {-1,0,0},
				// +Y
				{0,+1,0}, {0,+1,0}, {0,+1,0}, {0,+1,0},
				// -Y
				{0,-1,0}, {0,-1,0}, {0,-1,0}, {0,-1,0},
				// +Z
				{0,0,+1}, {0,0,+1}, {0,0,+1}, {0,0,+1},
				// -Z
				{0,0,-1}, {0,0,-1}, {0,0,-1}, {0,0,-1}

			};


			m.colors = {
				glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
				glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
				glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
				glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
				glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
				glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f)
			};

			m.texcoords = {
				// +X
				{0,0}, {1,0}, {1,1}, {0,1},
				// -X
				{0,0}, {1,0}, {1,1}, {0,1},
				// +Y
				{0,0}, {1,0}, {1,1}, {0,1},
				// -Y
				{0,0}, {1,0}, {1,1}, {0,1},
				// +Z
				{0,0}, {1,0}, {1,1}, {0,1},
				// -Z
				{0,0}, {1,0}, {1,1}, {0,1}
			};

			m.indices = {
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

			return m;
		}

		MeshData make_plane() {

			MeshData m;

			m.positions = {
				{-0.5f, 0, -0.5f},
				{+0.5f, 0, -0.5f},
				{+0.5f, 0, +0.5f},
				{-0.5f, 0, +0.5f}
			};

			m.normals = {
				{0, +1, 0},
				{0, +1, 0},
				{0, +1, 0},
				{0, +1, 0}
			};

			m.colors = {
				glm::vec3(0.5f),
				glm::vec3(0.5f),
				glm::vec3(0.5f),
				glm::vec3(0.5f)
			};

			m.texcoords = {
				{0, 0},
				{1, 0},
				{1, 1},
				{0, 1}
			};

			m.indices = {
				0, 1, 2,
				0, 2, 3
			};

			return m;
		}

		MeshGL upload_mesh_data(MeshData& mesh) {

			MeshGL mgl;

			mgl.vbo.create();

			// Calculate how much size we need to allocate for vbo
			// Calculate values for ease of packing
			GLsizei position_data_offset = 0;
			GLsizei position_attribute_size = sizeof(glm::vec3);
			GLsizei position_data_size = position_attribute_size * static_cast<GLsizei>(mesh.positions.size());

			GLsizei color_data_offset = position_data_size;
			GLsizei color_attribute_size = sizeof(glm::vec3);
			GLsizei color_data_size = color_attribute_size * static_cast<GLsizei>(mesh.colors.size());

			GLsizei buffer_size = position_data_size + color_data_size;

			// Wrapper for named buffer storage
			mgl.vbo.storage(buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

			// Load data into sub buffer		
			mgl.vbo.sub_data(position_data_offset, position_data_size, mesh.positions.data());
			mgl.vbo.sub_data(color_data_offset, color_data_size, mesh.colors.data());

			// Set up the VAO
			mgl.vao.create();

			// Bind the vertex array for the position
			mgl.vao.enable_attrib(0);
			mgl.vao.bind_vertex_buffer(0, mgl.vbo, position_data_offset, position_attribute_size);
			mgl.vao.attrib_format(0, 3, GL_FLOAT, false, 0);
			mgl.vao.attrib_binding(0, 0);
			glBindVertexArray(0);

			// Bind the vertex array for the colors
			mgl.vao.enable_attrib(1);
			mgl.vao.bind_vertex_buffer(1, mgl.vbo, color_data_offset, color_attribute_size);
			mgl.vao.attrib_format(1, 3, GL_FLOAT, false, 0);
			mgl.vao.attrib_binding(1, 1);
			glBindVertexArray(0);

			// Create an element buffer object to transfer topology
			mgl.ebo.create();
			mgl.ebo.storage(sizeof(GLushort) * mesh.indices.size(), reinterpret_cast<GLvoid*>(mesh.indices.data()), GL_DYNAMIC_STORAGE_BIT);
			mgl.vao.bind_element_buffer(mgl.ebo);
			glBindVertexArray(0);

			mgl.draw_count = mesh.indices.size();
			mgl.primitive_type = GL_TRIANGLES;

			return mgl;
		}
	}

}