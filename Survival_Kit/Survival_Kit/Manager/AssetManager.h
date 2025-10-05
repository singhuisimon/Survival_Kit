#pragma once

/**
 * @file AssetManager.h
 * @brief Declares the AssetManager for coordinating asset pipeline tasks.
 * @author Wai Lwin Thit, Rio Shannon Yvon Leonardo
 * @date 18/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#ifndef __ASSET_MANAGER_H__
#define __ASSET_MANAGER_H__

#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

// Manager base + logging
#include "Manager.h"
#include "LogManager.h"

// Pipeline headers (In Pipeline folder)
#include "../Pipeline/AssetDatabase.h"
#include "../Pipeline/AssetImporter.h"
#include "../Pipeline/AssetScanner.h"
#include "../Pipeline/AssetDescriptorGenerator.h" 

// Compiler files (In Compiler folder)
#include "../Compiler/ResourceCompiler.h"
#include "../Compiler/TextureCompiler.h"
#include "../Compiler/MeshCompiler.h"
#include "../Compiler/AudioCompiler.h"
#include "../Compiler/ShaderCompiler.h"

//asset path
#include "../Utility/AssetPath.h"


//define acronym for easier access 
#define AM gam300::AssetManager::getInstance()

namespace gam300 {

	/**
	* @brief Job for the compilation thread pool
	*/
	struct CompilationJob {
		AssetId assetId;
		std::string intermediatePath;
		AssetType assetType;
		std::unique_ptr<ResourceProperties> properties;
		xresource::full_guid guid;
	};

	/**
	* @class AssetManager
	* @brief Central editor tool coordinating scanning, importing and DB.
	*/
	class AssetManager : public Manager {
	private:
		AssetManager(); // singleton
		AssetManager(const AssetManager&) = delete; // no copy
		void operator=(const AssetManager&) = delete; // no assign

		// NEW: Compiler support
		std::unique_ptr<CompilerRegistry> m_compilerRegistry;
		std::unique_ptr<ResourcePaths> m_resourcePaths;

		// Thread-safe compilation queue
		std::queue<CompilationJob> m_compilationQueue;
		std::vector<CompileResult> m_completedCompilations;
		std::mutex m_queueMutex;
		std::mutex m_resultsMutex;
		std::atomic<bool> m_compilerThreadRunning{ false };
		std::thread m_compilerThread;

	public:
		// Singleton accessor (same pattern as other managers)
		static AssetManager& getInstance();


		// ---------------- Configuration ----------------
		struct Config {
			std::vector<std::string> sourceRoots; // Folders to scan
			std::vector<std::string> scanExtensions; // Allowed extensions (no dots); empty = all
			std::vector<std::string> ignoreSubstrings; // Quick ignore substrings
			bool includeHidden = false; // Scan dot-files on POSIX
			bool followSymlinks = false; // Recurse through symlinks

			//temporary for now
			std::string intermediateDirectory = "Survival_Kit/Survival_Kit/Assets/Cache/Intermediate"; //!< Where import output goes
			std::string databaseFile = "Survival_Kit/Survival_Kit/Assets/Cache/assetdb.txt"; //!< Asset DB persistence
			std::string snapshotFile = "Survival_Kit/Survival_Kit/Assets/Cache/scan.snapshot"; //!< Scanner warm start

			bool writeDescriptors = true; // Emit .desc files
			bool descriptorSidecar = true; // `foo.png.desc` next to source
			std::string descriptorRoot; // Used when sidecar = false

			std::string repoRoot; // Base path to resolve relative asset paths
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
				assetsPath + "Shaders"
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


		/**
		 * @brief Initialize the compiler system
		 * @details Call this in startUp() after scanner is initialized
		 */
		void initializeCompilers();

		/**
		 * @brief Queue an asset for compilation
		 * @param rec Asset record to compile
		 */
		void queueCompilation(const AssetRecord* rec);

		/**
		 * @brief Process compilation queue (call in scanAndProcess)
		 */
		void processCompilationQueue();

		/**
		 * @brief Get completed compilation results
		 */
		std::vector<CompileResult> getCompletedCompilations();

		/**
		 * @brief Shut down compiler thread safely
		 */
		void shutdownCompilers();

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

		/**
		 * @brief Compiler worker thread function
		 */
		void compilerWorkerThread();

		/**
		 * @brief Compile a single job (safe, won't crash)
		 */
		void compileJob(const CompilationJob& job);
	};

}	//end of namespace gam300

#endif // __ASSET_MANAGER_H__