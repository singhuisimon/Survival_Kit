#include "DescriptorFileManager.h"
#include "LogManager.h"
#include "../Utility/AssetPath.h"
#include <filesystem>
#include <fstream>
#include <sstream>


namespace gam300 {

	DescriptorFileManager::DescriptorFileManager() {
		setType("DescriptorFileManager");

		//get the descriptor rooth path relative to the assets
		m_descriptors_root_path = getAssetsPath() + "Descriptors/";
	}
	
	//get the singleton instance
	DescriptorFileManager& DescriptorFileManager::getInstance() {
		static DescriptorFileManager instance;
		return instance;
	}

	int DescriptorFileManager::startUp() {

		//call partent setup, if failed, return
		if (Manager::startUp()) return -1;

		//create the descriptor directory structure if it does not exit yet
		try {
			createDirectoryIfNotExists(m_descriptors_root_path);
			createDirectoryIfNotExists(m_descriptors_root_path + "Textures/");
			createDirectoryIfNotExists(m_descriptors_root_path + "/Meshes");
			createDirectoryIfNotExists(m_descriptors_root_path + "/Materials");
			createDirectoryIfNotExists(m_descriptors_root_path + "/Audio");
		}
		catch (const std::exception& e) {
			LM.writeLog("DescriptorFileManager::startUp() - Failed to create directories: %s",
				e.what());
			return -1;
		}

		//Log startup
		LM.writeLog("DescriptorFileManager::startUp() - DescriptorFileManager started successfully");
		LM.writeLog("DescriptorFileManager::startUp() - Descriptors root path: %s", m_descriptors_root_path.c_str());

		return 0;
	}

	void DescriptorFileManager::shutDown() {

		LM.writeLog("DescriptorFileManager::shutDown() - Shutting down Descriptor File Manager");

		//clear all loaded descriptors
		m_loaded_descriptors.clear();

		//manager shutdown
		Manager::shutDown();
	}


	xresource::full_guid DescriptorFileManager::createDescriptor(ResourceType resource_type,
		const std::string& intermediate_file_path,
		const std::string& resource_name) {

		//generate GUID
		xresource::instance_guid instance_guid = xresource::instance_guid::GenerateGUIDCopy();
	/*	xresource::instance_guid instance_guid; 
		instance_guid.GenerateGUID(); */

		//generate type GUID based on resource type string

		xresource::type_guid type_guid = xresource::type_guid::GenerateGUIDCopy(resourceTypeToString(resource_type).c_str());

		//xresource::type_guid type_guid; 
		//type_guid.GenerateGUID(resourceTypeToString(resource_type).c_str());


		xresource::full_guid full_guid; 
		full_guid.m_Instance = instance_guid;
		full_guid.m_Type = type_guid;

		//create the descriptor properties based on type
		std::unique_ptr<DescriptorProperties> properties; 
		switch (resource_type) {
			case ResourceType::TEXTURE: 
				properties = std::make_unique<TextureDescriptorProperties>();
				break;

			case ResourceType::MESH: 
				properties = std::make_unique<MeshDescriptorProperties>();
				break;

			default: 
				properties = std::make_unique<DescriptorProperties>();
				break;

		}

		//set common properties
		properties->resourceName = resource_name;
		properties->intermediateFilePath = intermediate_file_path;
		properties->resourceType = resource_type;

		
		//create the descriptor file
		DescriptorFile descriptorFile;
		descriptorFile.guid = full_guid;
		descriptorFile.filePath = getDescriptorFilePath(full_guid, resource_type);
		descriptorFile.properties = std::move(properties);

		//save the descriptor
		if (!saveDescriptor(descriptorFile)) {
			LM.writeLog("DescriptorFileManager::createDescriptor() - Failed to save descriptor");
			return xresource::full_guid{}; //return with empty GUID on failure (or shall i return the said guid)
		}

		// Store in cache for future access
		std::string guid_str = std::to_string(full_guid.m_Instance.m_Value);
		m_loaded_descriptors[guid_str] = std::move(descriptorFile);

		LM.writeLog("DescriptorFileManager::createDescriptor() - Created descriptor with GUID: %llu",
			full_guid.m_Instance.m_Value);

		return full_guid;
	}

	//load descriptor file
	const DescriptorFile* DescriptorFileManager::loadDescriptor(const xresource::full_guid& guid) {

		//get the guid str 
		std::string guid_str = std::to_string(guid.m_Instance.m_Value);

		//check if it's already loaded
		auto it = m_loaded_descriptors.find(guid_str);
		if (it != m_loaded_descriptors.end()) {
			return &it->second;
		}

		//determin resource type and load from file
		for (int i = 0; i <= static_cast<int>(ResourceType::AUDIO); ++i) {
			ResourceType type = static_cast<ResourceType>(i);
			std::string file_path = getDescriptorFilePath(guid, type);

			if (std::filesystem::exists(file_path)) {
				DescriptorFile descriptor;
				descriptor.guid = guid;
				descriptor.filePath = file_path;
				descriptor.properties = readPropertiesFromFile(file_path);

				if (descriptor.properties) {
					auto result = m_loaded_descriptors.emplace(guid_str, std::move(descriptor));
					if (result.second) {
						return &result.first->second;
					}
				}
				break;
			}
		}

		LM.writeLog("DescriptorFileManager::loadDescriptor() - Failed to load descriptor with GUID: %llu",
			guid.m_Instance.m_Value);
		return nullptr;
	}



	//save descriptor file
	bool DescriptorFileManager::saveDescriptor(const DescriptorFile& descriptor) {

		if (!descriptor.properties) {
			LM.writeLog("DescriptorFileManager::saveDescriptor() - Descriptor has no properties");
			return false;
		}

		//ensure directory exits
		std::string dir = std::filesystem::path(descriptor.filePath).parent_path().string();
		if (!createDirectoryIfNotExists(dir)) {
			LM.writeLog("DescriptorFileManager::saveDescriptor() - Failed to create directory: %s", dir.c_str());
			return false;
		}

		return writePropertiesToFile(descriptor.filePath, *descriptor.properties);
	}


	// Set the root path for descriptor files
	void DescriptorFileManager::setDescriptorsRootPath(const std::string& path) {
		m_descriptors_root_path = path;
		if (!m_descriptors_root_path.empty() && m_descriptors_root_path.back() != '/') {
			m_descriptors_root_path += '/';
		}
	}

	// Get the root path for descriptor files
	const std::string& DescriptorFileManager::getDescriptorsRootPath() const {
		return m_descriptors_root_path;
	}


	/******************INTERNAL HELPER METHODS ***********************************/

	std::string DescriptorFileManager::resourceTypeToString(ResourceType resource_type) const {

		switch (resource_type) {
		case ResourceType::TEXTURE:
			return "Texture";

		case ResourceType::MESH:
			return "Mesh";

		case ResourceType::MATERIAL:
			return "Material";

		case ResourceType::AUDIO:
			return "Audio";

		default:
			return "Unknown";

		}

	}

	ResourceType DescriptorFileManager::stringToResourceType(const std::string& type_str) const {
		if (type_str == "Texture") return ResourceType::TEXTURE;
		if (type_str == "Mesh") return ResourceType::MESH;
		if (type_str == "Material") return ResourceType::MATERIAL;
		if (type_str == "Audio") return ResourceType::AUDIO;
		return ResourceType::UNKNOWN;
	}

	// Get the full path to a descriptor file
	std::string DescriptorFileManager::getDescriptorFilePath(const xresource::full_guid& guid, ResourceType type) const {
		std::string type_folder = getResourceTypeFolder(type);

		// Create directory structure based on GUID (first 2 digits of hex representation)
		uint64_t guid_val = guid.m_Instance.m_Value;
		std::stringstream ss;
		ss << std::hex << std::uppercase << guid_val;
		std::string hex_str = ss.str();

		// Pad with zeros if needed
		while (hex_str.length() < 16) {
			hex_str = "0" + hex_str;
		}

		std::string sub_dir1 = hex_str.substr(0, 2);
		std::string sub_dir2 = hex_str.substr(2, 2);

		std::string file_name = hex_str + ".desc";

		return m_descriptors_root_path + type_folder + "/" + sub_dir1 + "/" + sub_dir2 + "/" + file_name;
	}

	// Get the resource type folder name
	std::string DescriptorFileManager::getResourceTypeFolder(ResourceType type) const {
		return resourceTypeToString(type);
	}

	// Create directory if it doesn't exist
	bool DescriptorFileManager::createDirectoryIfNotExists(const std::string& path) const {
		try {
			if (!std::filesystem::exists(path)) {
				return std::filesystem::create_directories(path);
			}
			return true;
		}
		catch (const std::exception& e) {
			LM.writeLog("DescriptorFileManager::createDirectoryIfNotExists() - Exception: %s", e.what());
			return false;
		}
	}

	/******************INTERNAL HELPER METHODS END ***********************************/

		//for saving the descriptor to a file
	bool DescriptorFileManager::writePropertiesToFile(const std::string& file_path, const DescriptorProperties& properties) const {

		try {
			std::ofstream file(file_path);
			if (!file.is_open()) {
				LM.writeLog("DescriptorFileManager::writePropertiesToFile() - Failed to open file: %s", file_path.c_str());
				return false;
			}

			//write common properties
			file << "ResourceName=" << properties.resourceName << "\n";
			file << "IntermediateFilePath=" << properties.intermediateFilePath << "\n"
				<< "ResourceType=" << resourceTypeToString(properties.resourceType) << "\n";

			//write tags
			file << "Tags=";
			for (size_t i = 0; i < properties.tags.size(); ++i) {
				file << properties.tags[i];
				if (i < properties.tags.size() - 1) file << ",";
			}
			file << "\n";

			//write type-specific properties
			switch (properties.resourceType) {
			case ResourceType::TEXTURE:

				break;
			case ResourceType::MESH:

				break;

			default:
				break;
			}

			file.close();
			return true;

		}
		catch (const std::exception& e) {
			LM.writeLog("DescriptorFileManager::writePropertiesToFile() - Exception: %s", e.what());
			return false;
		}

	}

	// Read properties from file
	std::unique_ptr<DescriptorProperties> DescriptorFileManager::readPropertiesFromFile(const std::string& file_path) const {
		try {
			std::ifstream file(file_path);
			if (!file.is_open()) {
				LM.writeLog("DescriptorFileManager::readPropertiesFromFile() - Failed to open file: %s", file_path.c_str());
				return nullptr;
			}

			// Read resource type first to determine what type of properties to create
			std::string line;
			ResourceType resource_type = ResourceType::UNKNOWN;
			std::string resource_name, intermediate_file_path;
			std::vector<std::string> tags;

			while (std::getline(file, line)) {
				if (line.find("ResourceType=") == 0) {
					resource_type = stringToResourceType(line.substr(13));
					break;
				}
			}

			// Reset file to beginning
			file.clear();
			file.seekg(0);

			// Create appropriate properties object
			std::unique_ptr<DescriptorProperties> properties;
			switch (resource_type) {
			case ResourceType::TEXTURE:
				properties = readTextureProperties(file);
				break;
			case ResourceType::MESH:
				properties = readMeshProperties(file);
				break;
			default:
				properties = std::make_unique<DescriptorProperties>();
				break;
			}

			if (!properties) {
				return nullptr;
			}

			// Read common properties
			file.clear();
			file.seekg(0);
			while (std::getline(file, line)) {
				if (line.find("ResourceName=") == 0) {
					properties->resourceName = line.substr(13);
				}
				else if (line.find("IntermediateFilePath=") == 0) {
					properties->intermediateFilePath = line.substr(21);
				}
				else if (line.find("Tags=") == 0) {
					std::string tags_str = line.substr(5);
					std::stringstream ss(tags_str);
					std::string tag;
					while (std::getline(ss, tag, ',')) {
						properties->tags.push_back(tag);
					}
				}
			}

			properties->resourceType = resource_type;
			file.close();
			return properties;
		}
		catch (const std::exception& e) {
			LM.writeLog("DescriptorFileManager::readPropertiesFromFile() - Exception: %s", e.what());
			return nullptr;
		}
	}

	// Write texture-specific properties
	void DescriptorFileManager::writeTextureProperties(std::ofstream& file, const TextureDescriptorProperties& props) const {
		file << "MaxWidth=" << props.maxWidth << "\n";
		file << "MaxHeight=" << props.maxHeight << "\n";
		file << "CompressionFormat=" << props.compressionFormat << "\n";
		file << "SRGB=" << (props.srgb ? "true" : "false") << "\n";
	}
	// Write mesh-specific properties
	void DescriptorFileManager::writeMeshProperties(std::ofstream& file, const MeshDescriptorProperties& props) const {
		file << "ScaleFactor=" << props.scaleFactor << "\n";
		file << "OptimizeVertices=" << (props.optimizeVertices ? "true" : "false") << "\n";
		file << "GenerateNormals=" << (props.generateNormal ? "true" : "false") << "\n";
		file << "GenerateTangents=" << (props.generate_tangents ? "true" : "false") << "\n";
		file << "AnimationImportMode=" << props.animation_import_mode << "\n";
	}


	// Read texture-specific properties
	std::unique_ptr<TextureDescriptorProperties> DescriptorFileManager::readTextureProperties(std::ifstream& file) const {
		auto props = std::make_unique<TextureDescriptorProperties>();

		std::string line;
		while (std::getline(file, line)) {
			if (line.find("MaxWidth=") == 0) {
				props->maxWidth = std::stoi(line.substr(9));
			}
			else if (line.find("MaxHeight=") == 0) {
				props->maxHeight = std::stoi(line.substr(10));
			}
			else if (line.find("CompressionFormat=") == 0) {
				props->compressionFormat = line.substr(18);
			}
			else if (line.find("SRGB=") == 0) {
				props->srgb = (line.substr(5) == "true");
			}
		}

		return props;
	}

	// Read mesh-specific properties
	std::unique_ptr<MeshDescriptorProperties> DescriptorFileManager::readMeshProperties(std::ifstream& file) const {
		auto props = std::make_unique<MeshDescriptorProperties>();

		std::string line;
		while (std::getline(file, line)) {
			if (line.find("ScaleFactor=") == 0) {
				props->scaleFactor = std::stof(line.substr(12));
			}
			else if (line.find("OptimizeVertices=") == 0) {
				props->optimizeVertices = (line.substr(17) == "true");
			}
			else if (line.find("GenerateNormals=") == 0) {
				props->generateNormal = (line.substr(16) == "true");
			}
			else if (line.find("GenerateTangents=") == 0) {
				props->generate_tangents = (line.substr(17) == "true");
			}
			else if (line.find("AnimationImportMode=") == 0) {
				props->animation_import_mode = line.substr(20);
			}
		}

		return props;
	}


}// end of namespace gam300

