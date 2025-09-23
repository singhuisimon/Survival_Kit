#include "../Graphics/Shape.h"
#include "../Manager/GraphicsManager.h"
#include "../Utility/MathUtils.h"

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

		MeshData make_sphere() {

			// Stack is the vertical/longitudinal axis
			// Sectors is the horizontal/lateral axis
			MeshData m;

			const int stacks  = 60;
			const int sectors = 50;

			const float radius = 1.0f;
			const float lengthInv = 1.0f / radius;

			float sector_factor = glm::two_pi<float>() / sectors;
			float stack_factor = glm::pi<float>() / stacks;

			float sector_angle, stack_angle; // sector angle is theta, stack angle is phi

			for (int i = 0; i <= stacks; ++i) {

				stack_angle = glm::pi<float>() / 2.0f - i * stack_factor;
				float xy = radius * std::cosf(stack_angle);
				float z  = radius * std::sinf(stack_angle);

				for (int j = 0; j <= sectors; ++j) {
						
					sector_angle = sector_factor * j;
					float x = xy * std::cosf(sector_angle);
					float y = xy * std::sinf(sector_angle);
					m.positions.push_back({ x,y,z });

					float nx = x * lengthInv;
					float ny = y * lengthInv;
					float nz = z * lengthInv;
					m.normals.push_back({ nx, ny, nz });

					m.colors.push_back(glm::vec3(0.5f));

					float s = static_cast<float>(j) / sectors;
					float t = static_cast<float>(i) / stacks;
					m.texcoords.push_back({ s, t });
				}
			}

			// Generate index buffer
			uint32_t v1, v2;
			for (int i = 0; i < stacks; ++i) {

				v1 = i *  (sectors + 1); // beginning of current stack
				v2 = v1 + (sectors + 1); // beginning of next stack

				for (int j = 0; j < sectors; ++j, ++v1, ++v2) {

					// 2 triangles per sectors excluding first and last stacks
					// v1 => v2 => v1 + 1
					if (i != 0) {

						m.indices.push_back(v1);
						m.indices.push_back(v2);
						m.indices.push_back(v1 + 1);
					}

					if (i != (stacks - 1)) {

						m.indices.push_back(v1 + 1);
						m.indices.push_back(v2);
						m.indices.push_back(v2 + 1);
					}

					
					// Future consideration for line indices.
				}

			}

			return m;
		}

		MeshGL upload_mesh_data(MeshData& mesh) {

			MeshGL mgl;

			const size_t N = mesh.positions.size();
			if (N == 0 || mesh.indices.empty()) throw std::runtime_error("Corrupt mesh, check mesh position and index values!");

			mgl.vbo.create();

			// Calculate how much size we need to allocate for vbo
			// Calculate values for ease of packing
			GLsizeiptr position_data_offset = 0;
			GLsizeiptr position_attribute_size = sizeof(glm::vec3);
			GLsizeiptr position_data_size = position_attribute_size * static_cast<GLsizeiptr>(mesh.positions.size());

			GLsizeiptr color_data_offset = position_data_size;
			GLsizeiptr color_attribute_size = sizeof(glm::vec3);
			GLsizeiptr color_data_size = color_attribute_size * static_cast<GLsizeiptr>(mesh.colors.size());

			GLsizeiptr buffer_size = position_data_size + color_data_size;

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

			// Bind the vertex array for the colors
			mgl.vao.enable_attrib(1);
			mgl.vao.bind_vertex_buffer(1, mgl.vbo, color_data_offset, color_attribute_size);
			mgl.vao.attrib_format(1, 3, GL_FLOAT, false, 0);
			mgl.vao.attrib_binding(1, 1);

			// Create an element buffer object to transfer topology
			mgl.ebo.create();
			mgl.ebo.storage(sizeof(uint32_t) * mesh.indices.size(), reinterpret_cast<GLvoid*>(mesh.indices.data()), GL_DYNAMIC_STORAGE_BIT);
			mgl.vao.bind_element_buffer(mgl.ebo);

			mgl.draw_count = mesh.indices.size();
			mgl.primitive_type = GL_TRIANGLES;

			return mgl;
		}
	}
}