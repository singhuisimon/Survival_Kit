/**
 * @file DescriptorEditor.h
 * @brief Declares the descriptor editing interface for the editor, for editing Descriptors
 * @author Wai Lwin Thit (100%)
 * @date 29/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#pragma once

#ifndef _DESCRIPTOR_EDITOR_H_
#define _DESCRIPTOR_EDITOR_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

//include the header files 
#include "AssetDatabase.h"
#include "AssetDescriptorGenerator.h"

namespace gam300 {
	
	/**
	* @brief Editable representation of descriptor data for the editor
	* @details This is a simplified, editor-friendly separate from the pipeline.
	*/
	struct EditableDescriptor {

		//Asset Record Data (read-only for editor, shown for reference)
		AssetId assetId= 0;
		std::string guid;
		std::string sourcePath;
		AssetType assetType = AssetType::Unknown;

		//Editable extras
		std::string displayName;
		std::string category;
		std::vector<std::string>tags;
		std::unordered_map<std::string, std::string> userProperties;

		//editable texture settings (only for texture assets)
		struct TextureSettings {
			std::string usageType = "COLOR"; //default would be color 
			std::string compression = "BC1";
			float quality = 0.8f; 
			bool generateMipmaps = true;
			bool srgb = true;
			std::vector<std::string> inputFiles;

			bool isValid() const;

		}textureSettings;

		AudioSettings audioSettings;
		MeshSettings meshSettings;
		ShaderSettings shaderSettings;

		//track the modification status
		bool isDirty = false;

		//validation
		bool isValid() const;
		std::vector<std::string>validate() const;

	};


	/**
	 * @class DescriptorEditor
	 * @brief Provides editor interface for reading/editing/saving descriptors.
	 * @details Handles the bridge between the asset pipeline and ImGui editor.
	 */
	class DescriptorEditor {
	public:
		DescriptorEditor(AssetDatabase& db, AssetDescriptorGenerator& gen);

		// =========== CORE API FOR EDITOR ===========

	   /**
		* @brief Load a descriptor for editing by AssetId.
		* @param id The asset's unique ID
		* @param outDescriptor Output parameter filled with editable data
		* @return True if descriptor was loaded successfully
		*/
		bool LoadDescriptor(AssetId id, EditableDescriptor& outDescriptor);

		/**
		 * @brief Load a descriptor by GUID string.
		 * @param guid Hex string of the GUID
		 * @param outDescriptor Output parameter filled with editable data
		 * @return True if descriptor was loaded successfully
		 */
		bool LoadDescriptorByGuid(const std::string& guid, EditableDescriptor& outDescriptor);

		/**
		 * @brief Update a single property in the descriptor.
		 * @param id The asset ID
		 * @param propertyPath Dot-separated path (e.g., "displayName", "textureSettings.quality")
		 * @param value The new value as a string (will be parsed based on type)
		 * @return True if update succeeded and was validated
		 */
		bool UpdateProperty(AssetId id, const std::string& propertyPath, const std::string& value);

		/**
		 * @brief Save the entire descriptor immediately to disk.
		 * @param descriptor The modified descriptor to save
		 * @return True if save succeeded
		 */
		bool SaveDescriptor(const EditableDescriptor& descriptor);

		/**
		* @brief Check if a descriptor has unsaved changes.
		* @param id The asset ID
		* @return True if there are uncommitted changes in memory
		*/
		bool IsDescriptorDirty(AssetId id) const;

		/**
		 * @brief Get the file path of a descriptor.
		 * @param id The asset ID
		 * @return Full path to Descriptor.txt file
		 */
		std::string GetDescriptorPath(AssetId id) const;

		// =========== VALIDATION ===========

	   /**
		* @brief Validate descriptor data before saving.
		* @param descriptor The descriptor to validate
		* @param outErrors Optional vector to collect error messages
		* @return True if descriptor is valid
		*/
		bool ValidateDescriptor(const EditableDescriptor& descriptor,
			std::vector<std::string>* outErrors = nullptr) const;

		/**
		 * @brief Register a custom validation callback.
		 * @param assetType The asset type this validator applies to
		 * @param validator Function that returns true if valid, false + error message if invalid
		 */
		using ValidationCallback = std::function<bool(const EditableDescriptor&, std::string& outError)>;
		void RegisterValidator(AssetType assetType, ValidationCallback validator);
		// =========== HELPERS ===========

	  /**
	   * @brief Get default values for a new descriptor of given type.
	   * @param assetType The type of asset
	   * @return A descriptor pre-filled with sensible defaults
	   */
		static EditableDescriptor CreateDefault(AssetType assetType);

		/**
		 * @brief Get list of valid options for a string property (e.g., compression types).
		 * @param propertyPath The property to get options for
		 * @return Vector of valid string values
		 */
		std::vector<std::string> GetPropertyOptions(const std::string& propertyPath) const;

	private:
		// Internal helpers
		bool ReadDescriptorFile(const std::string& filePath, EditableDescriptor& outDescriptor);
		bool WriteDescriptorFile(const std::string& filePath, const EditableDescriptor& descriptor);

		std::string ParseJsonValue(const std::string& json, const std::string& fieldPath);
		bool ParseJsonBool(const std::string& json, const std::string& fieldName);
		float ParseJsonFloat(const std::string& json, const std::string& fieldName);
		std::vector<std::string> ParseJsonArray(const std::string& json, const std::string& fieldName);

		//std::string BuildJsonFromDescriptor(const EditableDescriptor& descriptor);

		//bool ValidateTextureSettings(const EditableDescriptor& descriptor, std::string& outError) const;

		// References to pipeline components
		AssetDatabase& m_db;
		AssetDescriptorGenerator& m_descGen;

		// Custom validators per asset type
		std::unordered_map<AssetType, std::vector<ValidationCallback>> m_validators;

		// In-memory cache of loaded descriptors (for dirty tracking)
		std::unordered_map<AssetId, EditableDescriptor> m_loadedDescriptors;

		// Property options lookup
		std::unordered_map<std::string, std::vector<std::string>> m_propertyOptions;

		void InitializePropertyOptions();
	};
}


#endif // !_DESCRIPTOR_EDITOR_H_
