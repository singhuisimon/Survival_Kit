#include "../Graphics/Renderer.h"
#include "../Manager/InputManager.h"


#pragma region NAMESPACE

namespace {

	// Testing values
	constexpr int width = 640, height = 480;

	inline std::vector<gam300::ShaderProgram> loadShaderPrograms(std::vector<std::pair<std::string, std::string>> shaders) {

		std::vector<gam300::ShaderProgram> shadersStorage;

		for (auto const& file : shaders) {
			// Create the shader files vector with types 
			std::vector<std::pair<GLenum, std::string>> shader_files;
			shader_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, file.first));
			shader_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, file.second));

			// Create new shader program
			gam300::ShaderProgram shader_program;

			// Use Graphics_Manager to compile the shader
			if (!shader_program.compileShader(shader_files)) {
				throw std::runtime_error("failed to compile shaders");
			}

			// Insert shader program into vector
			shadersStorage.emplace_back(shader_program);
		}

		return shadersStorage;
	}

	inline void test_load_shaders(std::vector<gam300::ShaderProgram>& shd) {

		std::string vertex_obj_path{ gam300::getAssetFilePath("Shaders/survival_kit_obj.vert")   };
		std::string fragment_obj_path{ gam300::getAssetFilePath("Shaders/survival_kit_obj.frag") };

		// Pair vertex and fragment shader files
		std::vector<std::pair<std::string, std::string>> shader_files{
			std::make_pair(vertex_obj_path, fragment_obj_path),
		};

		shd = loadShaderPrograms(shader_files);
	}

	inline void test_load_mesh(std::unordered_map<gam300::u32, gam300::MeshGL>& ms) {

		gam300::MeshData cd = gam300::Shape::make_cube();
		gam300::MeshData pd = gam300::Shape::make_plane();
		gam300::MeshData sd = gam300::Shape::make_sphere();

		gam300::MeshGL c = gam300::Shape::upload_mesh_data(cd);
		gam300::MeshGL p = gam300::Shape::upload_mesh_data(pd);
		gam300::MeshGL s = gam300::Shape::upload_mesh_data(sd);

		ms.emplace(0, std::move(c));
		ms.emplace(1, std::move(p));
		ms.emplace(2, std::move(s));
	}

	inline void test_setup_fbo(std::optional<gam300::FrameBuffer>& fbo, GLuint& tex, int w, int h) {

		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo->handle()));

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		fbo->attach_color(GL_COLOR_ATTACHMENT0, tex);

		GLuint rboDepth;
		glCreateRenderbuffers(1, &rboDepth);
		glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT24, w, h);
		fbo->attach_renderbuffer(GL_DEPTH_ATTACHMENT, rboDepth);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}

#pragma endregion

namespace gam300 {

	// On first load, setup some simple stuff
	void Renderer::setup() {

		// Temporary functions, used for testing only
		test_load_shaders(m_shader_storage);
		test_load_mesh(m_tmp_mesh_storage);

		auto tmp = FrameBuffer::create();

		if (tmp->valid()) {
			d_fbo = std::move(tmp);
		}

		test_setup_fbo(d_fbo, d_imgui_texture, width, height);

		LM.writeLog("Renderer::setup this=%p\n", (void*)this);
	}

	void Renderer::beginFrame() {

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearDepth(1.0f);

		glBindFramebuffer(GL_FRAMEBUFFER, d_fbo->handle());

		glViewport(0, 0, width, height);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::render_frame(std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light) {

		beginFrame();
		draw(draw_items, active_cam, light);
		endFrame();

	}

	void Renderer::draw(std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light) {

		m_shader_storage[0].programUse();

		for (const auto& item : draw_items) {
			
			// Temporary transformations
			m_shader_storage[0].setUniform("M", item.m_model_to_world_transform); // Model transform
			m_shader_storage[0].setUniform("V", active_cam.getLookAt());          // View transform
			m_shader_storage[0].setUniform("P", active_cam.getPerspective());     // Perspective transform

			m_shader_storage[0].setUniform("light.position", light.getLightPos());      // Position
			m_shader_storage[0].setUniform("light.La", light.getLightAmbient());        // Ambient
			m_shader_storage[0].setUniform("light.Ld", light.getLightDiffuse());        // Diffuse
			m_shader_storage[0].setUniform("light.Ls", light.getLightSpecular());       // Specular

			u32 mesh_handle = item.m_mesh_handle;
			m_tmp_mesh_storage[mesh_handle].vao.bind();

			GLenum primitive = m_tmp_mesh_storage[mesh_handle].primitive_type;
			GLuint draw_count = m_tmp_mesh_storage[mesh_handle].draw_count;
			GLenum index_type = m_tmp_mesh_storage[mesh_handle].index_type;

			glDrawElements(primitive, draw_count, index_type, NULL);
			glBindVertexArray(0);
		}
	}

	void Renderer::endFrame() {
		m_shader_storage[0].programFree();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


}