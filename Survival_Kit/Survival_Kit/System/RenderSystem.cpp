#include "RenderSystem.h"
#include "../Component/ComponentView.h"
#include "../Graphics/SharedGraphics.h"
#include "../Graphics/ShaderProgram.h"

// anonymous namespace
namespace {

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

	inline void test_load_shaders() {

		std::string vertex_obj_path{ "..\\Survival_Kit\\Assets\\Shaders\\survival_kit_obj.vert" };
		std::string fragment_obj_path{ "..\\Survival_Kit\\Assets\\Shaders\\survival_kit_obj.frag" };

		// Pair vertex and fragment shader files
		std::vector<std::pair<std::string, std::string>> shader_files{
			std::make_pair(vertex_obj_path, fragment_obj_path),
		};
		
		loadShaderPrograms(shader_files);
	}

	inline void test_load_mesh(std::unordered_map<std::string, gam300::MeshGL>& ms) {

		gam300::MeshData cd = gam300::Shape::make_cube();
		gam300::MeshData pd = gam300::Shape::make_plane();
		gam300::MeshData sd = gam300::Shape::make_sphere();

		gam300::MeshGL c = gam300::Shape::upload_mesh_data(cd);
		gam300::MeshGL p = gam300::Shape::upload_mesh_data(pd);
		gam300::MeshGL s = gam300::Shape::upload_mesh_data(sd);

		ms.emplace("cube",   std::move(c));
		ms.emplace("plane",  std::move(p));
		ms.emplace("sphere", std::move(s));
	}

}

namespace gam300 {

	RenderSystem::RenderSystem() : ComponentSystem<Transform3D, RenderComponent>("RenderSystem") {
		set_priority(102);
	}

	bool RenderSystem::init(SystemManager&) {
		LM.writeLog("RenderSystem::init() - Render System Initialized");

#pragma region TEST_FUNC
		test_load_shaders();
		test_load_mesh(m_tmp_mesh_storage);
#pragma endregion

		return true;
	}

	void RenderSystem::update(float dt) {

		(void)dt;

		for (EntityID entity_id : m_entities) {
			process_entity(entity_id);
		}

		configurePipelineState();

		draw();
	}

	void RenderSystem::shutdown() {
		LM.writeLog("RenderSystem::shutdown() - Render System shut down");
	}

	void RenderSystem::process_entity(EntityID entity_id) {


	}


}