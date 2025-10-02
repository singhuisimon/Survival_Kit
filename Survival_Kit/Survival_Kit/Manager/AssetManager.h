#pragma once
#ifndef __ASSET_MANAGER_H__
#define __ASSET_MANAGER_H__

#include <string>
#include <vector>
#include <ctime>
#include <memory>

// Manager base + logging
#include "Manager.h"
#include "LogManager.h"

//Pipeline headers (In Pipeline folder)
#include "../Pipeline/AssetDatabase.h"
#include "../Pipeline/AssetImporter.h"
#include "../Pipeline/AssetScanner.h"
#include "../Pipeline/AssetDescriptorGenerator.h" 

//asset path
#include "../Utility/AssetPath.h"


//define acronnym for easier access 
#define AM gam300::AssetManager::getInstance()

namespace gam300 {

	/**
	* @class AssetManager
	* @brief Central editor tool coordinating scanning, importing and DB.
	*/
	class AssetManager : public Manager {
	private:
		AssetManager(); // singleton
		AssetManager(const AssetManager&) = delete; // no copy
		void operator=(const AssetManager&) = delete; // no assign


	public:

		// Singleton accessor (same pattern as other managers)
		static AssetManager& getInstance();


		// ---------------- Configuration ----------------
		struct Config {
			std::vector<std::string> sourceRoots; //!< Folders to scan
			std::vector<std::string> scanExtensions; //!< Allowed extensions (no dots); empty = all
			std::vector<std::string> ignoreSubstrings; //!< Quick ignore substrings
			bool includeHidden = false; //!< Scan dot-files on POSIX
			bool followSymlinks = false; //!< Recurse through symlinks

			//temporary for now
			//std::string intermediateDirectory = "Cache/Intermediate"; //!< Where import output goes
			//std::string databaseFile = "Cache/assetdb.txt"; //!< Asset DB persistence
			//std::string snapshotFile = "Cache/scan.snapshot"; //!< Scanner warm start

			std::string intermediateDirectory = "Survival_Kit/Survival_Kit/Assets/Cache/Intermediate"; //!< Where import output goes
			std::string databaseFile = "Survival_Kit/Survival_Kit/Assets/Cache/assetdb.txt"; //!< Asset DB persistence
			std::string snapshotFile = "Survival_Kit/Survival_Kit/Assets/Cache/scan.snapshot"; //!< Scanner warm start


			bool writeDescriptors = true; //!< Emit .desc files
			bool descriptorSidecar = true; //!< `foo.png.desc` next to source
			std::string descriptorRoot; //!< Used when sidecar = false

			std::string repoRoot; //!< Base path to resolve relative asset paths
		};

		/**
		* @brief Create default configuration for the project
		* @return Configured AssetManager::Config with sensible defaults
		*/
		static Config createDefaultConfig() {
			Config cfg{};

			std::string assetsPath = getAssetsPath();

			// Source directories to scan
			cfg.sourceRoots = {
				assetsPath + "Audio",
				assetsPath + "Textures",
				assetsPath + "Scene",
				assetsPath + "Shaders",
				assetsPath + "Meshes"
			};

			// Descriptor configuration
			cfg.descriptorRoot = assetsPath + "Descriptors";
			cfg.descriptorSidecar = false;
			cfg.writeDescriptors = true;

			// Ignore patterns to prevent infinite loops
			cfg.ignoreSubstrings = {
				"/Descriptors/", "\\Descriptors\\",   // Don't scan descriptor output
				"/Cache/", "\\Cache\\",               // Don't scan cache folders
				".desc/", ".desc\\",                  // Don't scan .desc directories
				"Descriptor.txt"                      // Don't scan descriptor files
			};

			return cfg;
		}

		/** Apply configuration before startUp() */
		void setConfig(const Config& cfg);


		// --------------- Lifetime (Manager API) ---------------
		int startUp() override; //!< Configure pipeline and warm-load DB
		void shutDown() override; //!< Persist DB and snapshot


		// --------------- Main work ---------------
		/** Scan source roots, import changes, update DB, optionally emit .desc */
		void scanAndProcess();


		// --------------- Accessors ---------------
		AssetDatabase& db() { return m_db; }
		const AssetDatabase& db() const { return m_db; }
		AssetImporterRegistry& importers() { return m_importers; }
		const Config& config() const { return m_cfg; }
		AssetDescriptorGenerator& descriptorGenerator() { return m_descGen; }
		const AssetDescriptorGenerator& descriptorGenerator() const { return m_descGen; }


		//--------------Validating Descriptors ----------
		void validateExistingDescriptors();

		/**
		 * @brief Get the AssetId for a given source path.
		 * @param sourcePath Path to the asset file (e.g., "Assets/Textures/rock.png")
		 * @return AssetId (0 if not found)
		 */
		AssetId getAssetId(const std::string& sourcePath) const;

		/**
		 * @brief Get the AssetId for a given filename (searches all assets).
		 * @param filename Just the filename (e.g., "rock.png")
		 * @return AssetId (0 if not found, first match if multiple with same name)
		 */
		AssetId getAssetIdByFilename(const std::string& filename) const;

		/**
		 * @brief Get the AssetRecord for a given AssetId.
		 * @param id The asset ID
		 * @return Pointer to AssetRecord (nullptr if not found)
		 */
		const AssetRecord* getAssetRecord(AssetId id) const;

		/**
		 * @brief Check if an asset exists in the database.
		 * @param sourcePath Path to the asset file
		 * @return True if asset exists
		 */
		bool assetExists(const std::string& sourcePath) const;

	private:

		void handleAddedOrModified(const std::string& src);
		void handleRemoved(const std::string& src);
		static const char* typeName(AssetType t);


		// State
		Config m_cfg{};
		::gam300::AssetScanner m_scanner; //!< Scanner lives in namespace *game300*
		AssetImporterRegistry m_importers;
		AssetDatabase m_db;
		AssetDescriptorGenerator m_descGen;
	};

}	//end of namespace gam300

#endif // __ASSET_MANAGER_H__