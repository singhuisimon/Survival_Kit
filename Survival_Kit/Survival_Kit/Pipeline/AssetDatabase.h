#pragma once
#ifndef __ASSET_DATABASE_H__
#define __ASSET_DATABASE_H__
#include <string>
#include <unordered_map>
#include <vector> // iteration helper
#include <ctime> // lastWriteTime

namespace gam300 {

	// A stable identifier for an asset. Generated internally when needed.
	using AssetId = unsigned long long;


	// Broad categories of assets the pipeline recognizes.
	// Expand as needed for your project.
	enum class AssetType { Unknown, Shader, Texture, Audio, Mesh, Material, Scene };

	/**
	* @brief Per-asset metadata tracked by the pipeline/editor.
	*/
	struct AssetRecord {
		AssetId id = 0; //!< Stable ID
		std::string sourcePath; //!< Canonical source path (forward slashes)
		std::string intermediatePath; //!< Where the importer writes the cooked/intermediate file
		std::string compiledPath; //!< (Future) where a fully compiled runtime blob may live
		AssetType type = AssetType::Unknown; //!< Asset classification
		std::string ext; //!< Extension from source (e.g. ".png")
		std::string contentHash; //!< Optional strong content hash (hex)
		std::time_t lastWriteTime = 0; //!< Last observed source mtime (seconds)
		bool valid = false; //!< Import success flag (true when last import succeeded)
	};

	/**
	* @brief Map of AssetId <-> AssetRecord with helpers for path lookups.
	*/

	struct AssetDatabase {

		/** Load database from a text file. Returns false on I/O failure. */
		bool Load(const std::string& file);

		/** Save database to a text file. Returns false on I/O failure. */
		bool Save(const std::string& file) const;

		/**
		* @brief Ensure there is an id for a given *source* path and return it.
		* @details If the path is new, a record is created with a fresh id.
		* Path is normalized to forward slashes for the key.
		*/
		AssetId EnsureIdForPath(const std::string& path);

		/** Find record by id (const). Returns nullptr if missing. */
		const AssetRecord* Find(AssetId id) const;

		/** Find record by id (mutable). Returns nullptr if missing. */
		AssetRecord* FindMutable(AssetId id);

		/** Find record by *source* path (const). Returns nullptr if missing. */
		const AssetRecord* FindBySource(const std::string& path) const;

		/** Find record by *source* path (mutable). Returns nullptr if missing. */
		AssetRecord* FindBySourceMutable(const std::string& path);


		/** Remove a record by id. Returns true if a record was erased. */
		bool Remove(AssetId id);

		/** Remove a record by source path. Returns true if a record was erased. */
		bool RemoveBySource(const std::string& path);

		/** @return A convenience vector of mutable pointers to all records. */
		std::vector<AssetRecord*> AllMutable();

		/** Clear the whole database (in-memory). */
		void Clear();

		/** @return Number of records currently stored. */
		size_t Count() const { return byId.size(); }

		/** Normalize a path to forward slashes and no trailing slash. */
		static std::string NormalizePath(const std::string& path);

		/** Extract the lowercase extension (including the dot) from a path. */
		static std::string ExtensionLower(const std::string& path);

		//storage
		std::unordered_map<AssetId, AssetRecord> byId; //!< id -> record
		std::unordered_map<std::string, AssetId> bySourcePath; //!< normalized source -> id

	};

} //end of namespace gam300

#endif // __ASSET_DATABASE_H__