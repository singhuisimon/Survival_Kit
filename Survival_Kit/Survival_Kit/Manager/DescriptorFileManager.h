/**
* @file DescriptorFileManager.h
* @brief Declaration of Descriptor File Manager for resource pipeline.
* @details Manages creation, reading, and writing of resource descriptor files with GUID naming
* @author Wai Lwin Thit
* @date 12/09/2025
*  Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/


#pragma once
#ifndef _DESCRIPTOR_FILE_MANAGER_H_
#define _DESCRIPTOR_FILE_MANAGER_H_

//add necessary headers 
#include "Manager.h"
#include "xresource_guid-main/source/xresource_guid.h"

//add necessary libraries 
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <fstream>

//define two letter acronym for easier access
#define DFM gam300::DescriptorFileManager::getInstance()

namespace gam300 {

	//resource types
	enum class ResourceType {
		TEXTURE = 0, 
		MESH,
		MATERIAL, 
		AUDIO,
		UNKNOWN
	};

	//base descriptor properties
	struct DescriptorProperties {

		std::string resourceName;
		std::string intermediateFilePath;
		std::vector<std::string> tags; //tags for categorizing
		ResourceType resourceType; 

		DescriptorProperties() : resourceType(ResourceType::UNKNOWN) {}

	};

	/**
	* @brief Texture-specific descriptor properties 
	*/
	struct TextureDescriptorProperties : public DescriptorProperties {
		int maxWidth; 
		int maxHeight; 
		std::string compressionFormat;
		bool srgb;

		TextureDescriptorProperties() : maxWidth(1024), maxHeight(1024), srgb(false) {
			resourceType = ResourceType::TEXTURE;
		}
	};

	/**
	* @brief Mesh-specific descriptor properties
	*/
	struct MeshDescriptorProperties : public DescriptorProperties {

	
		float scaleFactor;                 // Scale to apply during import
		bool optimizeVertices;             // Whether to optimize vertex data
		bool generateNormal;              // Whether to generate normals if missing
		bool generate_tangents;             // Whether to generate tangents
		std::string animation_import_mode;  // How to handle animations

		MeshDescriptorProperties() : scaleFactor(1.0f), optimizeVertices(true),
			generateNormal(true), generate_tangents(true),
			animation_import_mode("default") {
			resourceType = ResourceType::MESH;
		}
	};

	/**
	* @brief Structure for a complete descriptor file
	*/
	class DescriptorFile {

	public:

		xresource::full_guid guid;
		std::string filePath; 
		std::unique_ptr<DescriptorProperties> properties; //resource specific properties

		DescriptorFile() = default; 
		DescriptorFile(DescriptorFile&& other) noexcept : 
			guid(other.guid), filePath(std::move(other.filePath)),
			properties(std::move(other.properties)) { }

		DescriptorFile& operator=(DescriptorFile&& other) noexcept {
			if (this != &other) {
				guid = other.guid;
				filePath = std::move(other.filePath);
				properties = std::move(other.properties);
			}
			return *this;
		}
		// Delete copy constructor and copy assignment
		DescriptorFile(const DescriptorFile&) = delete;
		DescriptorFile& operator=(const DescriptorFile&) = delete;
	};

/**************************************************************************************/

	/**
	* @brief DescriptorFileManager
	* @details Handles creation, reading, writing, and management of resource descriptor files.
	*/
	class DescriptorFileManager : public Manager {
	private:
		DescriptorFileManager();                                // Private since a singleton.
		DescriptorFileManager(DescriptorFileManager const&);   // Don't allow copy.
		void operator=(DescriptorFileManager const&);           // Don't allow assignment.

		std::string m_descriptors_root_path;                   // Root path for descriptor files
		std::unordered_map<std::string, DescriptorFile> m_loaded_descriptors; // Cache of loaded descriptors (GUID as key)

		// Internal helper methods
		std::string getDescriptorFilePath(const xresource::full_guid& guid, ResourceType type) const;
		std::string getResourceTypeFolder(ResourceType type) const;
		std::string resourceTypeToString(ResourceType type) const;
		ResourceType stringToResourceType(const std::string& type_str) const;
		bool createDirectoryIfNotExists(const std::string& path) const;

		// Serialization helpers
		bool writePropertiesToFile(const std::string& file_path, const DescriptorProperties& properties) const;
		std::unique_ptr<DescriptorProperties> readPropertiesFromFile(const std::string& file_path) const;
		void writeTextureProperties(std::ofstream& file, const TextureDescriptorProperties& props) const;
		void writeMeshProperties(std::ofstream& file, const MeshDescriptorProperties& props) const;
		std::unique_ptr<TextureDescriptorProperties> readTextureProperties(std::ifstream& file) const;
		std::unique_ptr<MeshDescriptorProperties> readMeshProperties(std::ifstream& file) const;

	public: 
		static DescriptorFileManager& getInstance();
		int startUp() override;
		void shutDown() override;
	

		//create a new descriptor file
		xresource::full_guid createDescriptor(ResourceType resource_type,
			const std::string& intermediate_file_path,
			const std::string& resource_name);

		//load descriptor file
		const DescriptorFile* loadDescriptor(const xresource::full_guid& guid);

		//save descriptor file
		bool saveDescriptor(const DescriptorFile& descriptor);

		/**
		 * @brief Set the root path for descriptor files.
		 * @param path Root path for descriptors
		 */
		void setDescriptorsRootPath(const std::string& path);

		/**
		 * @brief Get the root path for descriptor files.
		 * @return Root path for descriptors
		 */
		const std::string& getDescriptorsRootPath() const;
	};

} // end of namespace gam300


#endif // !_DESCRIPTOR_FILE_MANAGER_H_
