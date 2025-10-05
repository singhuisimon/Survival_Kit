#pragma once

/**
 * @file AssetDescriptorGenerator.h
 * @brief Declares the asset descriptor generator class. 
 * @author Wai Lwin Thit, Rio Shannon Yvon Leonardo
 * @date 15/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#ifndef __ASSET_DESCGENERATOR_H__
#define __ASSET_DESCGENERATOR_H__
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include "AssetDatabase.h"
#include "../Utility/AssetPath.h"

//external library for GUID
#include "../include/xresource_guid-main/source/xresource_guid.h"


namespace gam300 {

	// Forward declaration to avoid coupling. Defined in AssetDatabase.h
	struct AssetRecord;
	//for basic audio compilation settings 
	struct AudioSettings {

		//output format 
		std::string outputFormat = "OGG"; //the target format could be OGG, WAV
		std::string compression = "VORBIS"; //VORBIS, PCM

		//quality
		float quality = 0.7f;	//compression quality 0.0-1.0
		int sampleRate = 44100; //output sample rate 
		//channels
		std::string channelMode = "STEREO"; //MONO or STEREO
	};

	//for basic mesh compilation settings
	struct MeshSettings {

		//output format
		std::string outputFormat = "CUSTOM"; //custom binary format for the engine

		//vertex Data 
		bool includePos = true; //need position
		bool includeNormals = true; //for lighting
		bool includeColors = false; //vertex colors
		bool includeTexCoords = true; //for texturing

		//index format
		std::string indexType = "UINT32"; //UINT16 or UINT32

		//transform 
		float scale = 1.0f; //uniform scale

		//optimizations 
		bool optimizeVertices = true; //remove duplicates and optimize cache
		bool generateNormals = false; //generate if missing
	};

	//basic shader compilation settings
	struct ShaderSettings {
		//input files
		std::string vertexShader = ""; //Path to .vert source
		std::string fragmentShader = ""; //Path to .frag source

		//output format 
		std::string outputFormat = "GLSL"; //SPIRV or GLSL

		//target platform is OPENGL
		std::string targetAPI = "OPENGL";
		std::string targetVersion = "460"; //what is thisisssssss

		//optimization 
		std::string optimizationLevel = "PERFORMANCE"; //none, size, performance
		bool stripDebugInfo = true; //remove debug symbols 
	};

	/**
	* @brief Optional extra metadata to embed in the descriptor.
	* @details These fields are generic and do not depend on Resource*.
	*/
	struct DescriptorExtras
	{
		std::string displayName; 
		std::string category; 
		std::vector<std::string> tags; 
		std::time_t lastImported = 0; 
		std::unordered_map<std::string, std::string> user; 
	
		//added texture settings -- needed for compiler
		std::string usageType;
		std::string compression;
		float quality = 1.0f;
		bool generateMipmaps = false;
		bool srgb = false;
		std::vector<std::string> inputFiles;

		//new audio settings (for compile settings)
		AudioSettings audioSettings;
		//new mesh settings (for compile settings)
		MeshSettings meshSettings;
		//new shader settings (for compiler)
		ShaderSettings shaderSettings;
	};

	/**
	* @class AssetDescriptorGenerator
	* @brief Emits .desc files with metadata for editor/importer use.
	*/
	class AssetDescriptorGenerator
	{
	public:
		/**
		* @brief Control where descriptors are written.
		* @param sidecar If true, write next to the source file (foo.png.desc).
		* If false, write to m_outputRoot/filename.ext.desc.
		*/
		void SetSidecar(bool sidecar);


		/** @brief Set centralized output root (used when sidecar=false). */
		void SetOutputRoot(const std::string& root);


		/** @brief Pretty-print JSON with indentation (default: true). */
		void SetPretty(bool pretty);


		/**
		* @brief Generate a descriptor for a known AssetRecord.
		* @param rec Asset record (id/paths/type/etc.).
		* @param extras Optional extra fields to embed.
		* @param outPath Optional output: the path of the file written.
		* @return True on success (I/O succeeded).
		*/
		bool GenerateFor(const AssetRecord& rec, const DescriptorExtras* extras = nullptr,
			std::string* outPath = nullptr) const;


		/**
		* @brief Generate a descriptor for a raw source path (no AssetRecord).
		* @param sourcePath Path to the original asset file on disk.
		* @param extras Optional extra fields to embed.
		* @param outPath Optional output path string.
		*/
		bool GenerateForPath(AssetDatabase& db, const std::string& sourcePath,
			const DescriptorExtras* extras = nullptr, std::string* outPath = nullptr) const;


		/**
		* @brief Compute the default descriptor path for a source file.
		*/
		std::string DefaultDescPathForRecord(const AssetRecord& rec) const;

	private:
		// ---- Helpers (implementation only) ----
		std::string BuildJson(const AssetRecord* recOpt,
			const DescriptorExtras* extras) const;
		static std::string EscapeJson(const std::string& s);
		static bool EnsureParentDir(const std::string& path);
		static bool WriteText(const std::string& path, const std::string& text);


		bool m_sidecar = true; 
		std::string m_outputRoot; //Used when m_sidecar == false
		bool m_pretty = true; 

	};


} //end of namespace gam300

#endif // __ASSET_DESCGENERATOR_H__