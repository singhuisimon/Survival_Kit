#include "../Graphics/Renderer.h"
#include "../Manager/InputManager.h"

#include <glm-0.9.9.8/glm/gtx/matrix_decompose.hpp>

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

		std::string vertex_debug_path{ gam300::getAssetFilePath("Shaders/debug.vert") };
		std::string fragment_debug_path{ gam300::getAssetFilePath("Shaders/debug.frag") };

		// Pair vertex and fragment shader files
		std::vector<std::pair<std::string, std::string>> shader_files{
			std::make_pair(vertex_obj_path, fragment_obj_path),
			std::make_pair(vertex_debug_path, fragment_debug_path)
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

	inline void test_poll() {
		
	}
}

#pragma endregion

namespace gam300 {

	// On first load, setup some simple stuff
	void Renderer::setup() {

		// Temporary functions, used for testing only
		test_load_shaders(m_shader_storage);
		test_load_mesh(m_tmp_mesh_storage);

		// Create a framebuffer and configure it's settings
		auto fp_fbo = FrameBuffer::create();
		if (fp_fbo.has_value()) {
			m_framebuffers.push_back(std::move(*fp_fbo));
		}
		else {
			LM.writeLog("Renderer::setup() - Failed to create framebuffer!");
		}

		// Allocate storage for a texture on the GPU, this texture will be attached to the framebuffer
		auto fp_tex = Texture::alloc_storage_on_gpu(width, height);
		if (fp_tex.has_value()) {
			m_textures.push_back(std::move(*fp_tex));
		}
		else {
			LM.writeLog("Renderer::setup() - Failed to allocate storage on the GPU!");
		}

		// Allocate extra attachments to the framebuffer

		// Testing
		GLuint rboDepth;
		glCreateRenderbuffers(1, &rboDepth);
		glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT24, width, height);
		// end testing

		auto& fpfbo_ = m_framebuffers[0];
		auto& fptex_ = m_textures[0];

		fpfbo_.attach_color(GL_COLOR_ATTACHMENT0, static_cast<GLuint>(fptex_.handle()));
		fpfbo_.attach_depth(GL_DEPTH_ATTACHMENT, rboDepth);

		// Create a render pass for that framebuffer
		RenderPass first_pass
		{
			.pass_name = "First Pass",
			.fbo_handle = 0,
			.shdpgm_handle = 0

			// Leave the rest as default settings
		};

		// Register the pass with the renderer
		m_passes.push_back(first_pass);

		RenderPass debug_pass
		{
			.pass_name = "Debug Pass",
			.fbo_handle = 0,
			.shdpgm_handle = 1,
			.clear_color = false,
			.clear_depth = false,
			.depth_write = false,
			.culling = false,
			.passtype = PassType::DEBUG
		};

		m_passes.push_back(debug_pass);
	}

	void Renderer::beginFrame(RenderPass const& pass) {

		auto& fbo = m_framebuffers[pass.fbo_handle];
		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(fbo.handle()));

		auto& viewport = pass.view_port;
		glViewport(viewport.x, viewport.y, viewport.z, viewport.w);

		pass.depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glDepthMask(pass.depth_write ? GL_TRUE : GL_FALSE);

		if (pass.culling) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);      
			glFrontFace(GL_CCW);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		if (pass.blending) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
		}
		else {
			glDisable(GL_BLEND);
		}

		GLbitfield clear_mask = 0;

		if (pass.clear_color) {
			glClearColor(pass.clear_color_value.r,
				pass.clear_color_value.g,
				pass.clear_color_value.b,
				pass.clear_color_value.a);
			clear_mask |= GL_COLOR_BUFFER_BIT;
		}

		if (pass.clear_depth) {
			// If depth writes are disabled *before* clear, enable them just for clearing
			glDepthMask(GL_TRUE);
			clear_mask |= GL_DEPTH_BUFFER_BIT;
		}

		// Finally clear whatever bits were requested
		if (clear_mask != 0) {
			glClear(clear_mask);
		}

		glDepthMask(pass.depth_write ? GL_TRUE : GL_FALSE);

		auto& prog = m_shader_storage[pass.shdpgm_handle];
		prog.programUse();
	}

	void Renderer::render_frame(std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light) {

		for (const auto& pass : m_passes) {
			beginFrame(pass);
			draw(pass, draw_items, active_cam, light);
			endFrame(pass);
		}
	}

	void Renderer::draw(RenderPass const& pass, std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light) {

		auto& prog = m_shader_storage[pass.shdpgm_handle];

		prog.setUniform("V", active_cam.getLookAt());                // View transform
		prog.setUniform("P", active_cam.getPerspective());           // Perspective transform

		prog.setUniform("light.position", light.getLightPos());      // Position
		prog.setUniform("light.La", light.getLightAmbient());        // Ambient
		prog.setUniform("light.Ld", light.getLightDiffuse());        // Diffuse
		prog.setUniform("light.Ls", light.getLightSpecular());       // Specular

		for (const auto& item : draw_items) {
			
			if (pass.passtype == PassType::DEBUG) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.f, -1.f);
				glLineWidth(1.0f);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			// Temporary transformations
			prog.setUniform("M", item.m_model_to_world_transform); // Model transform
			prog.setUniform("material.Ka", test_material.getMaterialAmbient());
			prog.setUniform("material.Kd", test_material.getMaterialDiffuse());
			prog.setUniform("material.Ks", test_material.getMaterialSpecular());
			prog.setUniform("material.shininess", test_material.getMaterialShininess());

			u32 mesh_handle = item.m_mesh_handle;
			m_tmp_mesh_storage[mesh_handle].vao.bind();

			GLenum  primitive  = m_tmp_mesh_storage[mesh_handle].primitive_type;
			GLsizei draw_count = m_tmp_mesh_storage[mesh_handle].draw_count;
			GLenum  index_type = m_tmp_mesh_storage[mesh_handle].index_type;

			glDrawElements(primitive, draw_count, index_type, NULL);
			glBindVertexArray(0);
		}
	}

	void Renderer::endFrame(RenderPass const& pass) {
		auto& prog = m_shader_storage[pass.shdpgm_handle];
		prog.programFree();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}