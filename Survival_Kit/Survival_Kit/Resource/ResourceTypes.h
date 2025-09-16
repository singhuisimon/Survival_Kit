#pragma once
#ifndef _RESOURCE_TYPES_H
#define _RESOURCE_TYPES_H

//external libraries
#include "../include/xresource_guid-main/source/xresource_guid.h"


//c++ libraries
#include <string>
#include <vector>

namespace gam300 {

	/*
	* @brief enum for resource types
	*/
	enum class ResourceType {
		TEXTURE = 0, 
		MESH,
		MATERIAL,
		AUDIO,
		SHADER,
		UNKNOWN
	};

	/**
	 * @brief Convert ResourceType enum to string.
	 * @param type The resource type to convert.
	 * @return String representation of the resource type.
	 */
	std::string resourceTypeToString(ResourceType type) {
		switch (type) {
		case ResourceType::TEXTURE: 
			return "Texture";
		case ResourceType::MESH: 
			return "Mesh";
		case ResourceType::MATERIAL: 
			return "Material";
		case ResourceType::AUDIO: 
			return "Audio";
		case ResourceType::SHADER: 
			return "Shader";
		default: 
			return "Unknown";
		}
	}
	ResourceType stringToResourceType(const std::string& type_str)  {
		if (type_str == "Texture") return ResourceType::TEXTURE;
		if (type_str == "Mesh") return ResourceType::MESH;
		if (type_str == "Material") return ResourceType::MATERIAL;
		if (type_str == "Audio") return ResourceType::AUDIO;
		if (type_str == "Shader") return ResourceType::SHADER;
		return ResourceType::UNKNOWN;
	}
} // end of name space gam300

#endif // !_RESOURCE_TYPES_H
