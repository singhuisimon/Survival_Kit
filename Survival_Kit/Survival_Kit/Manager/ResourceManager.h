
#pragma once
#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__


#include "Manager.h"
#include "../Resource/ResourceTypes.h"
#include "../Resource/ResourceMetadata.h"
#include "../Resource/ResourcePaths.h"
#include "../include/xresource_mgr/xresource_mgr.h"
#include "../include/xresource_guid-main/source/xresource_guid.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

//define acronym for easier access
#define RM gam300::ResourceManager::getInstance()

namespace gam300 {

#if 0

	//register all loaders
	inline static xresource::loader_registration<ResourceGUID::texture_type_guid_v> texture_loader;
	inline static xresource::loader_registration<ResourceGUID::mesh_type_guid_v> mesh_loader;
	inline static xresource::loader_registration<ResourceGUID::material_type_guid_v> material_loader;
	inline static xresource::loader_registration<ResourceGUID::audio_type_guid_v> audio_loader;
	inline static xresource::loader_registration<ResourceGUID::shader_type_guid_v> shader_loader;

    // Forward declarations for resource data structures
// These represent the actual runtime data loaded into memory
    struct TextureData {
        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* data = nullptr;
        std::string format; // PNG, JPG, etc.

        ~TextureData() {
            if (data) {
                // Will use stb_image_free when we implement actual loading
                data = nullptr;
            }
        }
    };

    struct MeshData {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<float> tangents;
        std::string originalFormat; // FBX, OBJ, etc.
    };

    struct AudioData {
        std::vector<short> samples;
        int sampleRate = 44100;
        int channels = 2;
        int bitDepth = 16;
        float duration = 0.0f;
        std::string originalFormat; // WAV, MP3, etc.
    };

    struct ShaderData {
        std::string vertexSource;
        std::string fragmentSource;
        std::string geometrySource;
        std::vector<std::string> defines;
        bool compiled = false;
    };

    struct MaterialData {
        xresource::full_guid diffuseTexture;
        xresource::full_guid normalTexture;
        xresource::full_guid specularTexture;
        std::string shaderName;
        float shininess = 32.0f;
        float opacity = 1.0f;
        bool doubleSided = false;
    };

#endif

    //forward declarations for resource data structures
    struct TextureResource;
    struct MeshResource;
    struct MaterialResource;
    struct AudioResource;
    struct ShaderResource;

	class ResourceManager : public Manager {
	private: 
		ResourceManager();
		ResourceManager(ResourceManager const&);
		void operator=(ResourceManager const&) = delete;

		std::unique_ptr<xresource::mgr> m_resource_mgr;
		std::unique_ptr<ResourcePaths> m_paths;

		//cache for descriptor files to avoid repeated disk reads
		std::unordered_map<xresource::full_guid, std::unique_ptr<ResourceProperties>> m_descriptor_cache;

	public:
		static ResourceManager& getInstance();

		int startUp() override;

		void shutDown() override;

        /**
         * @brief Load a resource by GUID.
         * @tparam T Resource type (TextureResource, MeshResource, etc.)
         * @param guid The GUID of the resource to load.
         * @return Pointer to the loaded resource, or nullptr if loading failed.
         */
        template<typename T>
        T* loadResource(const xresource::full_guid& guid);

        /**
         * @brief Release a resource reference.
         * @tparam T Resource type.
         * @param guid The GUID of the resource to release.
         */
        template<typename T>
        void releaseResource(xresource::full_guid& guid);

        /**
         * @brief Get the ResourcePaths utility.
         * @return Reference to the ResourcePaths instance.
         */
        ResourcePaths& getPaths() { return *m_paths; }

        /**
         * @brief Get the xresource manager (for internal use by loaders).
         * @return Reference to the xresource manager.
         */
        xresource::mgr& getXResourceManager() { return *m_resource_mgr; }

        /**
         * @brief Get resource properties from descriptor file.
         * @param guid The resource GUID.
         * @param type The resource type.
         * @return Pointer to resource properties, or nullptr if not found.
         */
        ResourceProperties* getResourceProperties(const xresource::full_guid& guid, ResourceType type);

        /**
         * @brief Process end-of-frame cleanup.
         * @details Should be called once per frame to handle delayed resource cleanup.
         */
        void onEndFrame() { m_resource_mgr->OnEndFrameDelegate(); }
	};

    // Template specializations (declared here, defined in .cpp)
    template<>
    TextureResource* ResourceManager::loadResource<TextureResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<TextureResource>(xresource::full_guid& guid);

    template<>
    MeshResource* ResourceManager::loadResource<MeshResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<MeshResource>(xresource::full_guid& guid);

    template<>
    MaterialResource* ResourceManager::loadResource<MaterialResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<MaterialResource>(xresource::full_guid& guid);

    template<>
    AudioResource* ResourceManager::loadResource<AudioResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<AudioResource>(xresource::full_guid& guid);

    template<>
    ShaderResource* ResourceManager::loadResource<ShaderResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<ShaderResource>(xresource::full_guid& guid);


}// end of namespace gam300


#endif // __RESOURCE_MANAGER_H__