#pragma once
#ifndef __ASSET_IMPORTER_H__
#define __ASSET_IMPORTER_H__


#include <memory>
#include <string>
#include <vector>
#include "AssetDatabase.h" // AssetType, AssetRecord (namespace gam300)

namespace gam300 {

	/**
	* @brief Result of a single import operation.
	*/
	struct ImportResult {
		bool ok = false; //!< True if importer succeeded
		std::string intermediatePath; //!< Full path to the generated file
		AssetType type = AssetType::Unknown; //!< Classified asset type
		std::string contentHash; //!< Optional content hash (hex)
		std::string error; //!< Short error message when ok==false
	};


	/**
	* @brief Interface implemented by concrete importers (PNG/WAV/etc.).
	* @note Importers should be deterministic and side-effect free beyond
	* writing their output file(s) to the requested directory.
	*/
	class IAssetImporter {
	public:
		virtual ~IAssetImporter() = default;
		/**
		* @brief Return true if this importer supports files with the given extension.
		* @param ext File extension (case-insensitive, may include leading dot).
		*/
		virtual bool CanImport(const std::string& ext) const = 0;
		/**
		* @brief Convert a source path to an intermediate file placed in \p intermediateDir.
		* @return ImportResult describing success/failure and paths.
		*/
		virtual ImportResult Import(const std::string& srcPath,
			const std::string& intermediateDir) = 0;
	};


	/**
	* @brief Registry that holds multiple importers and dispatches by extension.
	*/
	class AssetImporterRegistry {
	public:
		void Register(std::unique_ptr<IAssetImporter> imp);
		ImportResult Import(const std::string& srcPath, const std::string& intermediateDir);
	private:
		std::vector<std::unique_ptr<IAssetImporter>> m_importers;
	};

	/**
	* @brief Convenience helper to add a basic set of importers.
	* @details These default importers currently \"copy-as-intermediate\" while
	* tagging the AssetType appropriately (PNG/JPG > Texture2D,
	* WAV/OGG > Audio, OBJ/FBX/GLTF > Mesh, etc.). They also compute a
	* fast 64-bit FNV-1a content hash of the source file.
	*/
	void RegisterDefaultImporters(AssetImporterRegistry& reg);


}	//end of namespace gam300

#endif // __ASSET_IMPORTER_H__