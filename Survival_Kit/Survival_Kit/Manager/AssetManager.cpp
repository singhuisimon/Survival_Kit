#include "AssetManager.h"
#include <filesystem>


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


		// Ensure some sensible defaults
		if (m_cfg.sourceRoots.empty())
			m_cfg.sourceRoots = { "Assets" };


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
		if (!m_cfg.databaseFile.empty())
			m_db.Save(m_cfg.databaseFile);


		// Save snapshot to speed up next run
		if (!m_cfg.snapshotFile.empty())
			m_scanner.SaveSnapshot(m_cfg.snapshotFile);


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
			m_descGen.GenerateFor(*rec, &x);
		}


		LM.writeLog("AssetManager - Imported: %s -> %s (%s)",
			src.c_str(), rec->intermediatePath.c_str(), typeName(rec->type));
	}

	void AssetManager::handleRemoved(const std::string& src) {
		if (m_db.RemoveBySource(src)) {
			LM.writeLog("AssetManager - Removed from DB: %s", src.c_str());
		}
	}

	void AssetManager::scanAndProcess() {
		// Iterate changes from the scanner and act on them
		for (const auto& c : m_scanner.Scan()) {
			switch (c.kind) {
			case ::game300::ScanChange::Kind::Added:
			case ::game300::ScanChange::Kind::Modified:
				handleAddedOrModified(c.sourcePath); break;
			case ::game300::ScanChange::Kind::Removed:
				handleRemoved(c.sourcePath); break;
			}
		}


		// Persist after a pass (cheap for small DBs; adjust cadence if needed)
		if (!m_cfg.databaseFile.empty())
			m_db.Save(m_cfg.databaseFile);
	}

	const char* AssetManager::typeName(AssetType t) {
		switch (t) {
		case AssetType::Shader: return "Shader";
		case AssetType::Texture2D: return "Texture2D";
		case AssetType::Audio: return "Audio";
		case AssetType::Mesh: return "Mesh";
		case AssetType::Material: return "Material";
		case AssetType::Scene: return "Scene";
		default: return "Unknown";
		}
	}

} //end of namespace gam300