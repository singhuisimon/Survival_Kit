
#include "ResourceManager.h"
#include "../Resource/ResourceData.h"
#include "LogManager.h"

namespace gam300 {

	ResourceManager::ResourceManager() {
		setType("ResourceManager");
		m_resource_mgr = std::make_unique<xresource::mgr>();
		m_paths = std::make_unique<ResourcePaths>();
	}

	//get 
	ResourceManager& ResourceManager::getInstance() {
		static ResourceManager instance;
		return instance;
	}

	//startUP
	int ResourceManager::startUp() {

		//call parent
		if (Manager::startUp()) {
			return -1;
		}
        try {
            // Initialize the xresource manager
            m_resource_mgr->Initiallize(10000); // Support up to 10,000 resources

            // Set this ResourceManager as user data for the xresource manager
            // This allows loaders to access ResourceManager methods
            m_resource_mgr->setUserData(this, false);

            // Initialize directory structure using ResourcePaths
            if (!m_paths->initializeDirectories()) {
                LM.writeLog("ResourceManager::startUp() - Failed to initialize directories");
                return -1;
            }

            // Set the root path for xresource_mgr (convert to wide string)
            std::string descriptor_root = m_paths->getDescriptorRootPath();
            std::wstring wide_descriptor_path(descriptor_root.begin(), descriptor_root.end());
            m_resource_mgr->setRootPath(std::move(wide_descriptor_path));

            LM.writeLog("ResourceManager::startUp() - Resource Manager started successfully");
            LM.writeLog("ResourceManager::startUp() - Descriptor root: %s", descriptor_root.c_str());
            LM.writeLog("ResourceManager::startUp() - Intermediate root: %s", m_paths->getIntermediateRootPath().c_str());
            LM.writeLog("ResourceManager::startUp() - Compiled root: %s", m_paths->getCompiledRootPath().c_str());

            return 0;
        }
        catch (const std::exception& e) {
            LM.writeLog("ResourceManager::startUp() - Exception: %s", e.what());
            return -1;
        }

	}

	void ResourceManager::shutDown() {

        LM.writeLog("ResourceManager::shutDown() - ResourceManager shutting down");

        m_descriptor_cache.clear();
		m_resource_mgr.reset();
		m_paths.reset();

		Manager::shutDown();
	}

    ResourceProperties* ResourceManager::getResourceProperties(const xresource::full_guid& guid, ResourceType type) {

        //check cache first
        auto cache = m_descriptor_cache.find(guid);

        //return it if found
        if (cache != m_descriptor_cache.end()) {
            return cache->second.get();
        }

        //load the descriptor file
        std::string descriptor_path = m_paths->getDescriptorFilePath(guid, type);

        //if file does not exits
        if (!m_paths->fileExists(descriptor_path)) {
            LM.writeLog("ResourceManager::getResourceProperties() - Descriptor file not found: %s", descriptor_path.c_str());
            return nullptr;
        }

        //TODO:: Implement descriptor file parsing
        return nullptr;
    }

    //texture load and release 

    //template for loadResource
    template<>
    TextureResource* ResourceManager::loadResource<TextureResource>(const xresource::full_guid& guid) {

        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<TextureResource*>(resource);

    }

    //release texture resource
    template<>
    void ResourceManager::releaseResource<TextureResource>(xresource::full_guid& guid) {

        m_resource_mgr->ReleaseRef(guid);

    }

    //mesh load 
    template<>
    MeshResource* ResourceManager::loadResource<MeshResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        return static_cast<MeshResource*>(resource);
    }

    //mesh release 
    template<>
    void ResourceManager::releaseResource<MeshResource>(xresource::full_guid& guid) {
        //ResourceGUID::mesh_guid mesh_guid;
        //mesh_guid.m_Instance = guid.m_Instance;

        ////m_resource_mgr->ReleaseRef(mesh_guid);
        //
        ////update the full guid with any changes
        //guid.m_Instance = mesh_guid.m_Instance;

        m_resource_mgr->ReleaseRef(guid);
    }


    //Material Load
    template<>
    MaterialResource* ResourceManager::loadResource<MaterialResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<MaterialResource*>(resource);
    }

    //Material Release
    template<>
    void ResourceManager::releaseResource<MaterialResource>(xresource::full_guid& guid) {
        m_resource_mgr->ReleaseRef(guid);
    }

    //load audio
    template<>
    AudioResource* ResourceManager::loadResource<AudioResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<AudioResource*>(resource);
    }

    //release audio
    template<>
    void ResourceManager::releaseResource<AudioResource>(xresource::full_guid& guid) {
        m_resource_mgr->ReleaseRef(guid);
    }

    //shader load
    template<>
    ShaderResource* ResourceManager::loadResource<ShaderResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<ShaderResource*>(resource);
    }

    //shader release
    template<>
    void ResourceManager::releaseResource<ShaderResource>(xresource::full_guid& guid) {

        m_resource_mgr->ReleaseRef(guid);
    }
     
}// end of namespace gam300