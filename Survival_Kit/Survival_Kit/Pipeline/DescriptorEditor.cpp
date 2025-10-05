/**
 * @file DescriptorEditor.cpp
 * @brief Implements descriptor editing and validation features.
 * @author Wai Lwin Thit (100%)
 * @date 29/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iomanip>



#include "DescriptorEditor.h"
#include "../Manager/LogManager.h"

namespace fs = std::filesystem;

namespace gam300 {

    // =========== EditableDescriptor Methods ===========

    bool EditableDescriptor::TextureSettings::isValid() const {
        // Quality must be between 0 and 1
        if (quality < 0.0f || quality > 1.0f) return false;

        // Usage type must be valid (based on actual descriptor.txt file)
        static const std::vector<std::string> validUsage = {
            "COLOR", "NORMAL", "METALLIC", "ROUGHNESS", "AO", "EMISSIVE", "UI", "GENERIC"
        };
        if (std::find(validUsage.begin(), validUsage.end(), usageType) == validUsage.end()) {
            return false;
        }

        // Compression must be valid
        static const std::vector<std::string> validCompression = {
            "BC1", "BC3", "BC4", "BC5", "BC7", "None"
        };
        if (std::find(validCompression.begin(), validCompression.end(), compression) == validCompression.end()) {
            return false;
        }

        return true;
    }

    bool EditableDescriptor::isValid() const {
        if (assetId == 0) return false;
        if (displayName.empty()) return false;
        if (assetType == AssetType::Texture && !textureSettings.isValid()) return false;
        return true;
    }

    std::vector<std::string> EditableDescriptor::validate() const {
        std::vector<std::string> errors;

        if (assetId == 0) errors.push_back("Invalid asset ID");
        if (displayName.empty()) errors.push_back("Display name cannot be empty");
        if (category.empty()) errors.push_back("Category cannot be empty");

        // Texture-specific validation
        if (assetType == AssetType::Texture) {
            if (!textureSettings.isValid()) {
                errors.push_back("Invalid texture settings");
            }
            if (textureSettings.quality < 0.0f || textureSettings.quality > 1.0f) {
                errors.push_back("Quality must be between 0.0 and 1.0");
            }
        }

        return errors;
    }

    // =========== DescriptorEditor Implementation ===========

    DescriptorEditor::DescriptorEditor(AssetDatabase& db, AssetDescriptorGenerator& gen)
        : m_db(db), m_descGen(gen) {
        InitializePropertyOptions();
    }

    bool DescriptorEditor::LoadDescriptor(AssetId id, EditableDescriptor& outDescriptor) {
        // Find asset record
        const AssetRecord* rec = m_db.Find(id);
        if (!rec || !rec->valid) {
            LM.writeLog("DescriptorEditor - Asset ID %llu not found or invalid", id);
            return false;
        }

        // Get descriptor file path
        std::string descPath = m_descGen.DefaultDescPathForRecord(*rec);

        // Read and parse descriptor file
        if (!ReadDescriptorFile(descPath, outDescriptor)) {
            LM.writeLog("DescriptorEditor - Failed to read descriptor: %s", descPath.c_str());
            return false;
        }

        // Fill in asset record data
        outDescriptor.assetId = rec->id;
        std::ostringstream ss;
        ss << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << rec->id;
        outDescriptor.guid = ss.str();
        outDescriptor.sourcePath = rec->sourcePath;
        outDescriptor.assetType = rec->type;
        outDescriptor.isDirty = false;

        // Cache loaded descriptor for dirty tracking
        m_loadedDescriptors[id] = outDescriptor;

        return true;
    }

    bool DescriptorEditor::LoadDescriptorByGuid(const std::string& guid, EditableDescriptor& outDescriptor) {
        // Convert GUID string to AssetId
        AssetId id = 0;
        try {
            id = std::stoull(guid, nullptr, 16);
        }
        catch (...) {
            LM.writeLog("DescriptorEditor - Invalid GUID format: %s", guid.c_str());
            return false;
        }

        return LoadDescriptor(id, outDescriptor);
    }

    bool DescriptorEditor::UpdateProperty(AssetId id, const std::string& propertyPath,
        const std::string& value) {
        // Load current descriptor if not cached
        if (m_loadedDescriptors.find(id) == m_loadedDescriptors.end()) {
            EditableDescriptor desc;
            if (!LoadDescriptor(id, desc)) return false;
        }

        EditableDescriptor& desc = m_loadedDescriptors[id];

        // Parse property path and update
        if (propertyPath == "displayName") {
            desc.displayName = value;
        }
        else if (propertyPath == "category") {
            desc.category = value;
        }
        else if (propertyPath == "textureSettings.usageType") {
            desc.textureSettings.usageType = value;
        }
        else if (propertyPath == "textureSettings.compression") {
            desc.textureSettings.compression = value;
        }
        else if (propertyPath == "textureSettings.quality") {
            try {
                float q = std::stof(value);
                if (q < 0.0f || q > 1.0f) {
                    LM.writeLog("DescriptorEditor - Quality must be 0.0-1.0, got: %f", q);
                    return false;
                }
                desc.textureSettings.quality = q;
            }
            catch (...) {
                LM.writeLog("DescriptorEditor - Invalid float value: %s", value.c_str());
                return false;
            }
        }
        else if (propertyPath == "textureSettings.generateMipmaps") {
            desc.textureSettings.generateMipmaps = (value == "true" || value == "1");
        }
        else if (propertyPath == "textureSettings.srgb") {
            desc.textureSettings.srgb = (value == "true" || value == "1");
        }
        else {
            LM.writeLog("DescriptorEditor - Unknown property path: %s", propertyPath.c_str());
            return false;
        }

        // Validate after update
        std::vector<std::string> errors;
        if (!ValidateDescriptor(desc, &errors)) {
            LM.writeLog("DescriptorEditor - Validation failed after updating %s", propertyPath.c_str());
            for (const auto& err : errors) {
                LM.writeLog("  - %s", err.c_str());
            }
            return false;
        }

        desc.isDirty = true;

        // Immediate save
        return SaveDescriptor(desc);
    }

    bool DescriptorEditor::SaveDescriptor(const EditableDescriptor& descriptor) {
        // Validate before saving
        std::vector<std::string> errors;
        if (!ValidateDescriptor(descriptor, &errors)) {
            LM.writeLog("DescriptorEditor - Cannot save invalid descriptor");
            for (const auto& err : errors) {
                LM.writeLog("  - %s", err.c_str());
            }
            return false;
        }

        // Get descriptor path
        const AssetRecord* rec = m_db.Find(descriptor.assetId);
        if (!rec) {
            LM.writeLog("DescriptorEditor - Asset not found for save: %llu", descriptor.assetId);
            return false;
        }

        std::string descPath = m_descGen.DefaultDescPathForRecord(*rec);

        // Write to disk
        if (!WriteDescriptorFile(descPath, descriptor)) {
            LM.writeLog("DescriptorEditor - Failed to write descriptor: %s", descPath.c_str());
            return false;
        }

        // Update cache
        if (m_loadedDescriptors.find(descriptor.assetId) != m_loadedDescriptors.end()) {
            m_loadedDescriptors[descriptor.assetId] = descriptor;
            m_loadedDescriptors[descriptor.assetId].isDirty = false;
        }

        LM.writeLog("DescriptorEditor - Saved descriptor: %s", descPath.c_str());
        return true;
    }

    //bool DescriptorEditor::RevertDescriptor(AssetId id, EditableDescriptor& outDescriptor) {
    //    // Clear from cache to force reload
    //    m_loadedDescriptors.erase(id);

    //    // Reload from disk
    //    return LoadDescriptor(id, outDescriptor);
    //}

    bool DescriptorEditor::IsDescriptorDirty(AssetId id) const {
        auto it = m_loadedDescriptors.find(id);
        if (it == m_loadedDescriptors.end()) return false;
        return it->second.isDirty;
    }

    std::string DescriptorEditor::GetDescriptorPath(AssetId id) const {
        const AssetRecord* rec = m_db.Find(id);
        if (!rec) return "";
        return m_descGen.DefaultDescPathForRecord(*rec);
    }

    bool DescriptorEditor::ValidateDescriptor(const EditableDescriptor& descriptor,
        std::vector<std::string>* outErrors) const {
        // Basic validation
        std::vector<std::string> errors = descriptor.validate();

        // Run custom validators
        auto it = m_validators.find(descriptor.assetType);
        if (it != m_validators.end()) {
            for (const auto& validator : it->second) {
                std::string error;
                if (!validator(descriptor, error)) {
                    errors.push_back(error);
                }
            }
        }

        if (outErrors) {
            *outErrors = errors;
        }

        return errors.empty();
    }

    void DescriptorEditor::RegisterValidator(AssetType assetType, ValidationCallback validator) {
        m_validators[assetType].push_back(validator);
    }

    EditableDescriptor DescriptorEditor::CreateDefault(AssetType assetType) {
        EditableDescriptor desc;
        desc.assetType = assetType;
        desc.displayName = "New Asset";

        switch (assetType) {
        case AssetType::Texture:
            desc.category = "Texture";
            desc.textureSettings.usageType = "COLOR";  // Changed from "Albedo" to match actual descriptors
            desc.textureSettings.compression = "BC1";   // Changed from "BC7" to match actual descriptors
            desc.textureSettings.quality = 0.8f;        // Changed from 1.0 to match actual descriptors
            desc.textureSettings.generateMipmaps = true;
            desc.textureSettings.srgb = true;
            break;
        case AssetType::Mesh:
            desc.category = "Mesh";
            desc.meshSettings.outputFormat = "OGG";
            desc.meshSettings.includePos = true;
            desc.meshSettings.includeNormals = true;
            desc.meshSettings.includeColors = false;
            desc.meshSettings.includeTexCoords = true;
            desc.meshSettings.indexType = "UINT32";
            desc.meshSettings.scale = 1.0f;
            desc.meshSettings.optimizeVertices = true;
            desc.meshSettings.generateNormals = false;
            break;
        case AssetType::Material:
            desc.category = "Material";
            break;
        case AssetType::Shader:
            desc.category = "Shader";
            desc.shaderSettings.vertexShader = "";
            desc.shaderSettings.fragmentShader = "";
            desc.shaderSettings.outputFormat = "GLSL";
            desc.shaderSettings.targetAPI = "OPENGL";
            desc.shaderSettings.targetVersion = "460";
            desc.shaderSettings.optimizationLevel = "PERFORMANCE";
            desc.shaderSettings.stripDebugInfo = true;

            break;
        case AssetType::Audio:
            desc.category = "Audio";
            desc.audioSettings.outputFormat = "OGG";
            desc.audioSettings.compression = "VORBIS";
            desc.audioSettings.quality = 0.7f;
            desc.audioSettings.sampleRate = 44100;
            desc.audioSettings.channelMode = "STEREO";
            break;
            break;
        case AssetType::Scene:
            desc.category = "Scene";
            break;
        default:
            desc.category = "Unknown";
            break;
        }

        return desc;
    }

    std::vector<std::string> DescriptorEditor::GetPropertyOptions(const std::string& propertyPath) const {
        auto it = m_propertyOptions.find(propertyPath);
        if (it != m_propertyOptions.end()) {
            return it->second;
        }
        return {};
    }

    // =========== Private Helpers ===========

    bool DescriptorEditor::ReadDescriptorFile(const std::string& filePath,
        EditableDescriptor& outDescriptor) {
        // Check if file exists
        if (!fs::exists(filePath)) {
            LM.writeLog("DescriptorEditor - File not found: %s", filePath.c_str());
            return false;
        }

        // Read entire file
        std::ifstream file(filePath);
        if (!file.is_open()) {
            LM.writeLog("DescriptorEditor - Cannot open file: %s", filePath.c_str());
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json = buffer.str();

        // Parse JSON manually (simple parsing for known structure)
        outDescriptor.displayName = ParseJsonValue(json, "extras.displayName");
        outDescriptor.category = ParseJsonValue(json, "extras.category");
        outDescriptor.tags = ParseJsonArray(json, "extras.tags");

        // Parse texture settings if present
        if (json.find("textureSettings") != std::string::npos) {
            outDescriptor.textureSettings.usageType = ParseJsonValue(json, "textureSettings.usageType");
            outDescriptor.textureSettings.compression = ParseJsonValue(json, "textureSettings.compression");
            outDescriptor.textureSettings.quality = ParseJsonFloat(json, "textureSettings.quality");
            outDescriptor.textureSettings.generateMipmaps = ParseJsonBool(json, "textureSettings.generateMipmaps");
            outDescriptor.textureSettings.srgb = ParseJsonBool(json, "textureSettings.srgb");
            outDescriptor.textureSettings.inputFiles = ParseJsonArray(json, "textureSettings.inputFiles");
        }

        //Parse Audio Settings if present
        if (json.find("audioSettings") != std::string::npos) {
            outDescriptor.audioSettings.outputFormat = ParseJsonValue(json, "audioSettings.outputFormat");
            outDescriptor.audioSettings.compression = ParseJsonValue(json, "audioSettings.compression");
            outDescriptor.audioSettings.quality = ParseJsonFloat(json, "audioSettings.quality");
            outDescriptor.audioSettings.sampleRate = ParseJsonFloat(json, "audioSettings.sampleRate");
            outDescriptor.audioSettings.channelMode = ParseJsonValue(json, "audioSettings.channelMode");

        }

        //parse Mesh Settings if present
        if (json.find("meshSettings") != std::string::npos) {
            outDescriptor.meshSettings.outputFormat = ParseJsonValue(json, "meshSettings.outputFormat");
            outDescriptor.meshSettings.includePos = ParseJsonBool(json, "meshSettings.includePos");
            outDescriptor.meshSettings.includeNormals = ParseJsonBool(json, "meshSettings.includeNormals");
            outDescriptor.meshSettings.includeColors = ParseJsonBool(json, "meshSettings.includeColors");
            outDescriptor.meshSettings.includeTexCoords = ParseJsonBool(json, "meshSettings.includeTexCoords");
            outDescriptor.meshSettings.indexType = ParseJsonValue(json, "meshSettings.indexType");
            outDescriptor.meshSettings.scale = ParseJsonFloat(json, "meshSettings.scale");
            outDescriptor.meshSettings.optimizeVertices = ParseJsonBool(json, "meshSettings.optimizeVertices");
            outDescriptor.meshSettings.generateNormals = ParseJsonBool(json, "meshSettings.generateNormals");
        }

        //parse shader settings if present
        if (json.find("shaderSettings") != std::string::npos) {
            outDescriptor.shaderSettings.vertexShader = ParseJsonValue(json, "shaderSettings.vertexShader");
            outDescriptor.shaderSettings.fragmentShader = ParseJsonValue(json, "shaderSettings.fragmentShader");
            outDescriptor.shaderSettings.outputFormat = ParseJsonValue(json, "shaderSettings.outputFormat");
            outDescriptor.shaderSettings.targetAPI = ParseJsonValue(json, "shaderSettings.targetAPI");
            outDescriptor.shaderSettings.targetVersion = ParseJsonValue(json, "shaderSettings.targetVersion");
            outDescriptor.shaderSettings.optimizationLevel = ParseJsonValue(json, "shaderSettings.optimizationLevel");
            outDescriptor.shaderSettings.stripDebugInfo = ParseJsonBool(json, "shaderSettings.stripDebugInfo");
        }


        return true;
    }

    bool DescriptorEditor::WriteDescriptorFile(const std::string& filePath,
        const EditableDescriptor& descriptor) {
        // Get the asset record to include in JSON
        const AssetRecord* rec = m_db.Find(descriptor.assetId);
        if (!rec) return false;

        // Build DescriptorExtras from EditableDescriptor
        DescriptorExtras extras;
        extras.displayName = descriptor.displayName;
        extras.category = descriptor.category;
        extras.tags = descriptor.tags;
        extras.lastImported = std::time(nullptr);

        // Copy texture settings if applicable
        if (descriptor.assetType == AssetType::Texture) {
            extras.usageType = descriptor.textureSettings.usageType;
            extras.compression = descriptor.textureSettings.compression;
            extras.quality = descriptor.textureSettings.quality;
            extras.generateMipmaps = descriptor.textureSettings.generateMipmaps;
            extras.srgb = descriptor.textureSettings.srgb;
            extras.inputFiles = descriptor.textureSettings.inputFiles;
        }

        //copy audio settings 
        if (descriptor.assetType == AssetType::Audio) {
            extras.audioSettings = descriptor.audioSettings;
        }

        //copy mesh settings
        if (descriptor.assetType == AssetType::Mesh) {
            extras.meshSettings = descriptor.meshSettings;
        }

        // Copy shader settings if applicable
        if (descriptor.assetType == AssetType::Shader) {
            extras.shaderSettings = descriptor.shaderSettings;
        }


        // Copy user properties
        extras.user = descriptor.userProperties;

        // Use existing generator to write
        std::string outPath;
        return m_descGen.GenerateFor(*rec, &extras, &outPath);
    }

    std::string DescriptorEditor::ParseJsonValue(const std::string& json, const std::string& fieldPath) {
        // Simple JSON parser for "key": "value" patterns
        // Split path by dots
        std::vector<std::string> parts;
        std::stringstream ss(fieldPath);
        std::string part;
        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }

        // Find the final field in JSON
        std::string searchKey = "\"" + parts.back() + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";

        // Find the colon
        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) return "";

        // Find opening quote of value
        size_t valueStart = json.find('"', colonPos);
        if (valueStart == std::string::npos) return "";

        // Find closing quote
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) return "";

        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }

    bool DescriptorEditor::ParseJsonBool(const std::string& json, const std::string& fieldName) {
        std::string searchKey = "\"" + fieldName + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return false;

        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) return false;

        size_t truePos = json.find("true", colonPos);
        size_t falsePos = json.find("false", colonPos);

        if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos)) {
            return true;
        }
        return false;
    }

    float DescriptorEditor::ParseJsonFloat(const std::string& json, const std::string& fieldName) {
        std::string searchKey = "\"" + fieldName + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return 0.0f;

        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) return 0.0f;

        // Skip whitespace after colon
        size_t valueStart = colonPos + 1;
        while (valueStart < json.length() && std::isspace(json[valueStart])) {
            valueStart++;
        }

        // Extract number
        size_t valueEnd = valueStart;
        while (valueEnd < json.length() &&
            (std::isdigit(json[valueEnd]) || json[valueEnd] == '.' || json[valueEnd] == '-')) {
            valueEnd++;
        }

        std::string valueStr = json.substr(valueStart, valueEnd - valueStart);
        try {
            return std::stof(valueStr);
        }
        catch (...) {
            return 0.0f;
        }
    }

    std::vector<std::string> DescriptorEditor::ParseJsonArray(const std::string& json,
        const std::string& fieldName) {
        std::vector<std::string> result;

        std::string searchKey = "\"" + fieldName + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return result;

        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) return result;

        size_t arrayStart = json.find('[', colonPos);
        if (arrayStart == std::string::npos) return result;

        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayEnd == std::string::npos) return result;

        // Parse array contents
        std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        size_t start = 0;
        while (start < arrayContent.length()) {
            size_t quoteStart = arrayContent.find('"', start);
            if (quoteStart == std::string::npos) break;

            size_t quoteEnd = arrayContent.find('"', quoteStart + 1);
            if (quoteEnd == std::string::npos) break;

            std::string element = arrayContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            result.push_back(element);

            start = quoteEnd + 1;
        }

        return result;
    }

    void DescriptorEditor::InitializePropertyOptions() {
        // Texture usage types (updated to match actual descriptor.txt)
        m_propertyOptions["textureSettings.usageType"] = {
            "COLOR", "NORMAL", "METALLIC", "ROUGHNESS", "AO", "EMISSIVE", "UI", "GENERIC"
        };

        // Compression formats
        m_propertyOptions["textureSettings.compression"] = {
            "BC1", "BC3", "BC4", "BC5", "BC7", "None"
        };
    }

} // namespace gam300