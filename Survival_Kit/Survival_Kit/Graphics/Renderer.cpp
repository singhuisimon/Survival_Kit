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

	inline void load_basic_primitives(std::vector<gam300::MeshGL>& ms, std::vector<gam300::MeshData>& md) {

		gam300::MeshData cd = gam300::Shape::make_cube();
		gam300::MeshData pd = gam300::Shape::make_plane();
		gam300::MeshData sd = gam300::Shape::make_sphere();

		md.push_back(cd);
		md.push_back(pd);
		md.push_back(sd);

		gam300::MeshGL c = gam300::Shape::upload_mesh_data(cd);
		gam300::MeshGL p = gam300::Shape::upload_mesh_data(pd);
		gam300::MeshGL s = gam300::Shape::upload_mesh_data(sd);

		ms.push_back(std::move(c));
		ms.push_back(std::move(p));
		ms.push_back(std::move(s));
	}
	
}

namespace gam300 {

	namespace {
		int  selected_texture = 0;
		bool textureMode = false;
		bool isPBR = false;
	}

	inline void test_poll() {
		
		// Enable choosing of mesh
		if (IM.isKeyPressed(GLFW_KEY_1)) {
			selected_texture = 0;
		}

		if (IM.isKeyPressed(GLFW_KEY_2)) {
			selected_texture = 1;
		}

		if (IM.isKeyPressed(GLFW_KEY_3)) {
			selected_texture = 2;
		}

		// Testing texture
		if (IM.isKeyPressed(GLFW_KEY_9)) {
			textureMode = false;
		}
		if (IM.isKeyPressed(GLFW_KEY_0)) {
			textureMode = true;
		}

		// Choosing BlinnPhong or PBR
		if (IM.isKeyPressed(GLFW_KEY_B)) {
			isPBR = false;
		}
		if (IM.isKeyPressed(GLFW_KEY_N)) {
			isPBR = true;
		}

	}

}

#pragma endregion

namespace gam300 {

	// On first load, setup some simple stuff
	void Renderer::setup() {

		// Temporary functions, used for testing only
		test_load_shaders(m_shader_storage);

		// Load a set of basic primitives: Cube, Plane, Sphere
		load_basic_primitives(m_mesh_storage, m_mesh_data_storage);

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

#pragma region TEST_TO_SEE_TEXTURE_PASS_TEMP

		{
			RenderPass stub_pass
			{
				.pass_name = "Stub Pass",
				.fbo_handle = 0,
				.shdpgm_handle = 0

			};

			m_passes.push_back(stub_pass);
		}

#pragma endregion


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

#pragma region TEXTURE_LOAD_TEMP
		{
			// Temporarily load textures 
			auto mouse_tex = Texture::load_from_file(getAssetFilePath("Textures/mouse_kenny.png"), TextureDesc(false, false, true));
			if (mouse_tex->valid()) {
				t_testing_textures.push_back(std::move(*mouse_tex));
			}

			auto rabbit_tex = Texture::load_from_file(getAssetFilePath("Textures/rabbit_kenny.png"), TextureDesc(false, false, true));
			if (rabbit_tex->valid()) {
				t_testing_textures.push_back(std::move(*rabbit_tex));
			}

			auto squirrel_tex = Texture::load_from_file(getAssetFilePath("Textures/squirrel_kenny.png"), TextureDesc(false, false, true));
			if (squirrel_tex->valid()) {
				t_testing_textures.push_back(std::move(*squirrel_tex));
			}
		}
#pragma endregion


#pragma region MATERIAL_LOAD_TEMP
		{
			Material mat1 = Material(glm::vec3(0.3f, 0.5f, 0.9f), glm::vec3(0.3f, 0.5f, 0.9f), glm::vec3(0.8f, 0.8f, 0.8f), 100.0f);
			Material mat2 = Material(glm::vec3(0.9f, 0.5f, 0.3f), glm::vec3(0.9f, 0.5f, 0.3f), glm::vec3(0.8f, 0.8f, 0.8f), 100.0f);
			t_testing_material.emplace_back(mat1);
			t_testing_material.emplace_back(mat2);
		}
#pragma endregion


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

		//ZoneScoped;
		TracyGpuZone("RenderFrame");

		test_poll();

		for (const auto& pass : m_passes) {

			TracyGpuZone("RenderPass");

			beginFrame(pass);
			draw(pass, draw_items, active_cam, light);
			endFrame(pass);
		}
	}

	void Renderer::draw(RenderPass const& pass, std::span<const DrawItem> draw_items, Camera3D& active_cam, Light& light) {

		ZoneScoped;
		TracyGpuZone("DrawMeshes");

		auto& prog = m_shader_storage[pass.shdpgm_handle];

#pragma region CAMERA_CONTROLS_TEMP
		{
			// Temporary input for cursor to move camera
			if (IM.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {

				// MOUSE: Camera control with left mouse button held down
				if (IM.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
					double mouseDeltaX = IM.getMouseDeltaX();
					double mouseDeltaY = IM.getMouseDeltaY();

					if (std::abs(mouseDeltaX) > 0.1 || std::abs(mouseDeltaY) > 0.1) {
						// INCREASE SENSITIVITY
						float extraSensitivity = 500.0f;

						active_cam.cameraOnCursor(mouseDeltaX * extraSensitivity,
							mouseDeltaY * extraSensitivity,
							&prog);
					}
				}

				// Gather input for scrolling
				double scrollY_offset = IM.getScrollY();
				if (scrollY_offset != 0) {

					active_cam.cameraOnScroll(IM.getScrollY(), &prog);
				}
			}

			//Temporary input for light cursor
			if (IM.isKeyPressed(GLFW_KEY_L)) {
				//std::cout << IM.getMouseDeltaX() << std::endl;
				if (IM.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
					light.lightOnCursor(IM.getMouseDeltaX(), IM.getMouseDeltaY(), &prog);
				}
			}
		}
#pragma endregion

		prog.setUniform("V", active_cam.getLookAt());                // View transform
		prog.setUniform("P", active_cam.getPerspective());           // Perspective transform

		prog.setUniform("light.position", light.getLightPos());      // Position
		prog.setUniform("light.La", light.getLightAmbient());        // Ambient
		prog.setUniform("light.Ld", light.getLightDiffuse());        // Diffuse
		prog.setUniform("light.Ls", light.getLightSpecular());       // Specular


#pragma region SET_UNIFORM_TEMP
		if (textureMode) {
			glBindTextureUnit(0, t_testing_textures[selected_texture].handle());
			prog.setUniform("Texture2D", 0);
			prog.setUniform("isTexture", true);
		}
		else {
			prog.setUniform("isTexture", false);
		}

		if (isPBR) {
			prog.setUniform("isPBR", true);
		}
		else {
			prog.setUniform("isPBR", false);
		}
#pragma endregion

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

#pragma region TESTING
			size_t material_handle = static_cast<size_t>(item.m_material_handle);
			Material& test_material = t_testing_material[material_handle];
#pragma endregion

			// Temporary transformations
			prog.setUniform("M", item.m_model_to_world_transform); // Model transform
			prog.setUniform("material.Ka", test_material.getMaterialAmbient());
			prog.setUniform("material.Kd", test_material.getMaterialDiffuse());
			prog.setUniform("material.Ks", test_material.getMaterialSpecular());
			prog.setUniform("material.shininess", test_material.getMaterialShininess());

			size_t mesh_handle = static_cast<size_t>(item.m_mesh_handle);
			m_mesh_storage[mesh_handle].vao.bind();

			GLenum  primitive  = m_mesh_storage[mesh_handle].primitive_type;
			GLsizei draw_count = m_mesh_storage[mesh_handle].draw_count;
			GLenum  index_type = m_mesh_storage[mesh_handle].index_type;

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