#include "AssetManager.h"
#include <filesystem>

#include <iostream>

#include "../Utility/AssetPath.h" //for path management

namespace fs = std::filesystem;

namespace gam300 {

	//singleton plumbing
	AssetManager::AssetManager() { setType("AssetManager"); }

	AssetManager& AssetManager::getInstance() {
		static AssetManager s_mgr; return s_mgr;
	}

	//configuration
	void AssetManager::setConfig(const Config& cfg) {
		m_cfg = cfg;
	}

	//lifetime
	int AssetManager::startUp() {
		if (Manager::startUp())
			return -1;


		// Logging header
		LM.writeLog("AssetManager::startUp() - begin");
		
		//find base root
		auto AutoDetectRepoRoot = []() -> fs::path {
			fs::path p = fs::current_path();
			while (!p.empty()) {
				if (fs::exists(p / ".git") 
					//||
					//fs::exists(p / "Survival_Kit")
					) {
					return p;
				}
				p = p.parent_path();
			}
			return fs::current_path(); // fallback
			};

		fs::path base = m_cfg.repoRoot.empty() ? AutoDetectRepoRoot() : fs::path(m_cfg.repoRoot);


		// Prefer project-aware defaults when fields are empty
		if (m_cfg.sourceRoots.empty())
			m_cfg.sourceRoots = { "Assets" };

		if (m_cfg.intermediateDirectory.empty())
			m_cfg.intermediateDirectory = getIntermediatePath();      // Cache/Intermediate

		if (m_cfg.databaseFile.empty())
			m_cfg.databaseFile = (fs::path(getLocalCachePath()) / "assetdb.txt").string();

		if (!m_cfg.descriptorSidecar && m_cfg.descriptorRoot.empty())
			m_cfg.descriptorRoot = (fs::path(getAssetsPath()) / "Descriptors").string();


		auto Resolve = [&](const std::string& in) -> std::string {
			if (in.empty()) return in;
			fs::path p(in);
			return p.is_absolute() ? p.string() : (base / p).string();
			};


		// Ensure some sensible defaults
		if (m_cfg.sourceRoots.empty())
			m_cfg.sourceRoots = { "Assets" };

		//normnalize all paths
		for (auto& r : m_cfg.sourceRoots) r = Resolve(r);
		m_cfg.intermediateDirectory = Resolve(m_cfg.intermediateDirectory);
		m_cfg.databaseFile = Resolve(m_cfg.databaseFile);
		m_cfg.snapshotFile = Resolve(m_cfg.snapshotFile);
		if (!m_cfg.descriptorSidecar && !m_cfg.descriptorRoot.empty())
			m_cfg.descriptorRoot = Resolve(m_cfg.descriptorRoot);


		// Configure scanner (note: scanner is in namespace game300 and uses lowerCamel APIs)
		m_scanner.setRoots(m_cfg.sourceRoots);
		m_scanner.setExtensions(m_cfg.scanExtensions);
		m_scanner.setIgnoreSubstrings(m_cfg.ignoreSubstrings);
		m_scanner.setIncludeHidden(m_cfg.includeHidden);
		m_scanner.setFollowSymlinks(m_cfg.followSymlinks);


		// Directories and persistence
		fs::create_directories(m_cfg.intermediateDirectory);


		// Load previous DB if available
		if (!m_cfg.databaseFile.empty()) {
			if (m_db.Load(m_cfg.databaseFile))
				LM.writeLog("AssetManager - DB loaded: %s", m_cfg.databaseFile.c_str());
		}


		// Load scanner snapshot for faster first diff
		if (!m_cfg.snapshotFile.empty()) {
			m_scanner.LoadSnapshot(m_cfg.snapshotFile);
		}


		// Register built-in importers
		RegisterDefaultImporters(m_importers);


		// Descriptor writer setup
		m_descGen.SetSidecar(m_cfg.descriptorSidecar);
		if (!m_cfg.descriptorSidecar && !m_cfg.descriptorRoot.empty())
			m_descGen.SetOutputRoot(m_cfg.descriptorRoot);


		LM.writeLog("AssetManager::startUp() - ready");
		return 0;
	}

	void AssetManager::shutDown() {
		// Save DB
		if (!m_cfg.databaseFile.empty()) {
			m_db.Save(m_cfg.databaseFile);
			LM.writeLog("AssetManager::shutDown() - Saved database with %zu assets", m_db.Count());
		}

		// Save snapshot to speed up next run
		if (!m_cfg.snapshotFile.empty()) {
			size_t snapCount = m_scanner.GetSnapshotSize();
			bool success = m_scanner.SaveSnapshot(m_cfg.snapshotFile);
			LM.writeLog("AssetManager::shutDown() - Saved snapshot: %zu files, success=%d, path=%s",
				snapCount, success, m_cfg.snapshotFile.c_str());
		}



		LM.writeLog("AssetManager::shutDown() - complete");
		Manager::shutDown();
	}

	//change handling
	void AssetManager::handleAddedOrModified(const std::string& src) {
		// Ensure DB record
		auto id = m_db.EnsureIdForPath(src);
		auto* rec = m_db.FindMutable(id);
		if (!rec) return; // Should not happen, but be defensive


		// Import the source to intermediate directory
		ImportResult r = m_importers.Import(src, m_cfg.intermediateDirectory);
		if (!r.ok) {
			rec->valid = false;
			LM.writeLog("AssetManager - Import FAILED: %s (%s)", src.c_str(), r.error.c_str());
			return;
		}


		// Update record
		rec->intermediatePath = r.intermediatePath;
		rec->type = r.type;
		rec->contentHash = r.contentHash;
		rec->ext = AssetDatabase::ExtensionLower(rec->sourcePath);
		rec->valid = true;


		// Optional: emit .desc for editor tools
		if (m_cfg.writeDescriptors) {
			DescriptorExtras x; // resource-agnostic payload
			x.displayName = fs::path(rec->sourcePath).filename().string();
			x.category = typeName(rec->type);
			x.lastImported = std::time(nullptr);

			//ADDED 
			//If this is a texture and the importer provided settings,
			if (rec->type == AssetType::Texture && r.textureSettings.has_value()) {
				const auto& ts = *r.textureSettings;
				x.usageType = ts.usageType;
				x.compression = ts.compression;
				x.quality = ts.quality;
				x.generateMipmaps = ts.generateMipmaps;
				x.srgb = ts.srgb;
				x.inputFiles = ts.inputFiles;
			}


			m_descGen.GenerateFor(*rec, &x);
		}



		LM.writeLog("AssetManager - Imported: %s -> %s (%s)",
			src.c_str(), rec->intermediatePath.c_str(), typeName(rec->type));
	}

	void AssetManager::handleRemoved(const std::string& src) {
		// Find the record before removing it
		const AssetRecord* rec = m_db.FindBySource(src);

		if (rec && m_cfg.writeDescriptors) {
			// Get the descriptor path
			std::string descriptorPath = m_descGen.DefaultDescPathForRecord(*rec);

			// Delete the descriptor file
			if (fs::exists(descriptorPath)) {
				fs::remove(descriptorPath);
				LM.writeLog("AssetManager - Deleted descriptor file: %s", descriptorPath.c_str());
			}

			//// Delete the GUID.desc folder if it's now empty
			//fs::path descriptorFolder = fs::path(descriptorPath).parent_path();
			//if (fs::exists(descriptorFolder) && fs::is_empty(descriptorFolder)) {
			//	fs::remove(descriptorFolder);
			//	LM.writeLog("AssetManager - Deleted empty descriptor folder: %s",
			//		descriptorFolder.string().c_str());
			//}

			// Clean up empty parent folders (GUID.desc folder, then subdirs)
			fs::path currentFolder = fs::path(descriptorPath).parent_path();

			// Walk up the directory tree, removing empty folders
			// Stop at the Descriptors root or when we hit a non-empty folder
			fs::path descriptorsRoot = fs::absolute(m_cfg.descriptorRoot);

			while (currentFolder.has_parent_path()) {

				// Stop if we've reached the descriptors root
				if (fs::equivalent(currentFolder, descriptorsRoot)) {
					break;
				}

				if (fs::exists(currentFolder) && fs::is_empty(currentFolder)) {
					fs::remove(currentFolder);
					LM.writeLog("AssetManager - Deleted empty folder: %s",
						currentFolder.string().c_str());
					currentFolder = currentFolder.parent_path();
				}
				else {
					// Folder not empty or doesn't exist, stop climbing
					break;
				}
			}
		}

		// Remove from database

		if (m_db.RemoveBySource(src)) {
			LM.writeLog("AssetManager - Removed from DB: %s", src.c_str());
			// FIX: Save immediately
			// Persist immediately
			m_db.Save(m_cfg.databaseFile);
			m_scanner.SaveSnapshot(m_cfg.snapshotFile);
		}
	}

	void AssetManager::scanAndProcess() {

		LM.writeLog("AssetManager::scanAndProcess() - Snapshot has %zu files before scan",
			m_scanner.GetSnapshotSize());
		// Iterate changes from the scanner and act on them
		for (const auto& c : m_scanner.Scan()) {
			switch (c.kind) {
			case ::gam300::ScanChange::Kind::Added:
			case ::gam300::ScanChange::Kind::Modified:
				handleAddedOrModified(c.sourcePath); break;
			case ::gam300::ScanChange::Kind::Removed:
				handleRemoved(c.sourcePath); break;
			}
		}
		LM.writeLog("AssetManager::scanAndProcess() - Snapshot has %zu files after scan",
			m_scanner.GetSnapshotSize());

		////NEW to check for missing descriptors of unchanged files 
		//if (m_cfg.writeDescriptors) {
		//	validateExistingDescriptors();
		//}

		// Persist after a pass (cheap for small DBs; adjust cadence if needed)
		if (!m_cfg.databaseFile.empty())
			m_db.Save(m_cfg.databaseFile);
	}


	const char* AssetManager::typeName(AssetType t) {
		switch (t) {
		case AssetType::Shader: return "Shader";
		case AssetType::Texture: return "Texture";
		case AssetType::Audio: return "Audio";
		case AssetType::Mesh: return "Mesh";
		case AssetType::Material: return "Material";
		case AssetType::Scene: return "Scene";
		default: return "Unknown";
		}
	}

	void AssetManager::validateExistingDescriptors() {
		auto records = m_db.AllMutable();
		for (auto* rec : records) {
			if (!rec || !rec->valid) continue;

			// Get the expected path WITHOUT generating the file
			std::string expectedDescriptorPath = m_descGen.DefaultDescPathForRecord(*rec);

			// Check if the descriptor file actually exists
			if (!fs::exists(expectedDescriptorPath)) {
				LM.writeLog("AssetManager - Missing descriptor for %s, regenerating...",
					rec->sourcePath.c_str());

				// Only NOW generate the missing descriptor
				DescriptorExtras extras;
				extras.displayName = fs::path(rec->sourcePath).filename().string();
				extras.category = typeName(rec->type);
				extras.lastImported = std::time(nullptr);

				m_descGen.GenerateFor(*rec, &extras);
			}
		}
	}

	AssetId AssetManager::getAssetId(const std::string& sourcePath) const {
		const AssetRecord* rec = m_db.FindBySource(sourcePath);
		return rec ? rec->id : 0;
	}

	AssetId AssetManager::getAssetIdByFilename(const std::string& filename) const {
		// Search through all assets for matching filename
		auto allRecords = const_cast<AssetDatabase&>(m_db).AllMutable();

		for (const auto* rec : allRecords) {
			if (!rec) continue;

			// Extract filename from sourcePath
			fs::path p(rec->sourcePath);
			if (p.filename().string() == filename) {
				return rec->id;
			}
		}
		return 0; // Not found
	}

	const AssetRecord* AssetManager::getAssetRecord(AssetId id) const {
		return m_db.Find(id);
	}

	bool AssetManager:: assetExists(const std::string& sourcePath) const {
		return m_db.FindBySource(sourcePath) != nullptr;
	}


} //end of namespace gam300