#include "AssetManager.h"
#include <filesystem>
#include <iostream>
#include "../Utility/AssetPath.h"

namespace fs = std::filesystem;

namespace gam300 {

	// Singleton plumbing
	AssetManager::AssetManager() {
		setType("AssetManager");
	}

	AssetManager& AssetManager::getInstance() {
		static AssetManager s_mgr;
		return s_mgr;
	}

	// Configuration
	void AssetManager::setConfig(const Config& cfg) {
		m_cfg = cfg;
	}

	// Lifetime
	int AssetManager::startUp() {
		if (Manager::startUp())
			return -1;

		// Logging header
		LM.writeLog("AssetManager::startUp() - begin");

		// Find base root
		auto AutoDetectRepoRoot = []() -> fs::path {
			fs::path p = fs::current_path();
			while (!p.empty()) {
				if (fs::exists(p / ".git")) {
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
			m_cfg.intermediateDirectory = getIntermediatePath();

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

		// Normalize all paths
		for (auto& r : m_cfg.sourceRoots) r = Resolve(r);
		m_cfg.intermediateDirectory = Resolve(m_cfg.intermediateDirectory);
		m_cfg.databaseFile = Resolve(m_cfg.databaseFile);
		m_cfg.snapshotFile = Resolve(m_cfg.snapshotFile);
		if (!m_cfg.descriptorSidecar && !m_cfg.descriptorRoot.empty())
			m_cfg.descriptorRoot = Resolve(m_cfg.descriptorRoot);

		// Configure scanner
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

		// NEW: Initialize compiler system
		initializeCompilers();

		LM.writeLog("AssetManager::startUp() - ready");
		return 0;
	}

	void AssetManager::shutDown() {
		LM.writeLog("AssetManager::shutDown() - Starting shutdown");

		// NEW: Shut down compilers FIRST
		shutdownCompilers();

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

	// ==================== COMPILER SYSTEM ====================

	void AssetManager::initializeCompilers() {
		LM.writeLog("AssetManager::initializeCompilers() - Initializing compiler system");

		// Create compiler registry
		m_compilerRegistry = std::make_unique<CompilerRegistry>();

		// Register all compilers
		m_compilerRegistry->registerCompiler(std::make_unique<TextureCompiler>());
		m_compilerRegistry->registerCompiler(std::make_unique<MeshCompiler>());
		m_compilerRegistry->registerCompiler(std::make_unique<AudioCompiler>());
		m_compilerRegistry->registerCompiler(std::make_unique<ShaderCompiler>());

		// Create resource paths utility
		m_resourcePaths = std::make_unique<ResourcePaths>();
		m_resourcePaths->initializeDirectories();

		// Start compiler worker thread
		m_compilerThreadRunning = true;
		m_compilerThread = std::thread(&AssetManager::compilerWorkerThread, this);

		LM.writeLog("AssetManager::initializeCompilers() - Compiler system ready");
	}

	void AssetManager::queueCompilation(const AssetRecord* rec) {
		if (!rec || !rec->valid) {
			return;
		}

		// Create compilation job
		CompilationJob job;
		job.assetId = rec->id;
		job.intermediatePath = rec->intermediatePath;
		job.assetType = rec->type;

		// Generate GUID for this asset
		job.guid.m_Instance = xresource::instance_guid::GenerateGUIDCopy();
		job.guid.m_Type = xresource::type_guid::GenerateGUIDCopy(typeName(rec->type));

		// Create properties based on asset type
		switch (rec->type) {
		case AssetType::Texture:
			job.properties = std::make_unique<TextureProperties>();
			break;
		case AssetType::Mesh:
			job.properties = std::make_unique<MeshProperties>();
			break;
		case AssetType::Audio:
			job.properties = std::make_unique<AudioProperties>();
			break;
		case AssetType::Shader:
			job.properties = std::make_unique<ShaderProperties>();
			break;
		default:
			return; // Unknown type
		}

		// Queue the job
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_compilationQueue.push(std::move(job));
		}

		LM.writeLog("AssetManager - Queued compilation for: %s", rec->sourcePath.c_str());
	}

	void AssetManager::processCompilationQueue() {
		// Check for completed compilations
		std::vector<CompileResult> completed = getCompletedCompilations();

		for (const auto& result : completed) {
			if (result.success) {
				LM.writeLog("AssetManager - Compilation successful: %s",
					result.compiledPath.c_str());
			}
			else {
				LM.writeLog("AssetManager - Compilation FAILED: %s",
					result.error.c_str());
			}
		}
	}

	std::vector<CompileResult> AssetManager::getCompletedCompilations() {
		std::lock_guard<std::mutex> lock(m_resultsMutex);
		std::vector<CompileResult> results = std::move(m_completedCompilations);
		m_completedCompilations.clear();
		return results;
	}

	void AssetManager::shutdownCompilers() {
		LM.writeLog("AssetManager::shutdownCompilers() - Shutting down compiler thread");

		// Stop worker thread
		m_compilerThreadRunning = false;

		if (m_compilerThread.joinable()) {
			m_compilerThread.join();
		}

		// Clear any remaining jobs
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			while (!m_compilationQueue.empty()) {
				m_compilationQueue.pop();
			}
		}

		LM.writeLog("AssetManager::shutdownCompilers() - Compiler thread stopped");
	}

	void AssetManager::compilerWorkerThread() {
		LM.writeLog("AssetManager - Compiler worker thread started");

		while (m_compilerThreadRunning) {
			CompilationJob job;
			bool hasJob = false;

			// Try to get a job from the queue
			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				if (!m_compilationQueue.empty()) {
					job = std::move(m_compilationQueue.front());
					m_compilationQueue.pop();
					hasJob = true;
				}
			}

			if (hasJob) {
				// Compile the job (safely, in separate thread)
				compileJob(job);
			}
			else {
				// Sleep a bit if no jobs
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		LM.writeLog("AssetManager - Compiler worker thread exiting");
	}

	void AssetManager::compileJob(const CompilationJob& job) {
		LM.writeLog("AssetManager - Compiling asset ID: %llu", job.assetId);

		CompileResult result;

		try {
			// This is safe - runs in separate thread, won't crash ImGui
			result = m_compilerRegistry->compile(
				job.assetType,
				job.intermediatePath,
				job.properties.get(),
				*m_resourcePaths,
				job.guid
			);
		}
		catch (const std::exception& e) {
			result.success = false;
			result.error = std::string("Exception during compilation: ") + e.what();
			LM.writeLog("AssetManager - Compilation exception: %s", e.what());
		}
		catch (...) {
			result.success = false;
			result.error = "Unknown exception during compilation";
			LM.writeLog("AssetManager - Unknown compilation exception");
		}

		// Store result
		{
			std::lock_guard<std::mutex> lock(m_resultsMutex);
			m_completedCompilations.push_back(result);
		}
	}

	// ==================== CHANGE HANDLING ====================

	void AssetManager::handleAddedOrModified(const std::string& src) {
		// Ensure DB record
		auto id = m_db.EnsureIdForPath(src);
		auto* rec = m_db.FindMutable(id);
		if (!rec) return;

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
			DescriptorExtras x;
			x.displayName = fs::path(rec->sourcePath).filename().string();
			x.category = typeName(rec->type);
			x.lastImported = std::time(nullptr);

			// If this is a texture and the importer provided settings
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

		// NEW: Queue for compilation after successful import
		if (rec->valid) {
			queueCompilation(rec);
		}
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

			// Clean up empty parent folders
			fs::path currentFolder = fs::path(descriptorPath).parent_path();
			fs::path descriptorsRoot = fs::absolute(m_cfg.descriptorRoot);

			while (currentFolder.has_parent_path()) {
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
					break;
				}
			}
		}

		// Remove from database
		if (m_db.RemoveBySource(src)) {
			LM.writeLog("AssetManager - Removed from DB: %s", src.c_str());
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
				handleAddedOrModified(c.sourcePath);
				break;
			case ::gam300::ScanChange::Kind::Removed:
				handleRemoved(c.sourcePath);
				break;
			}
		}

		LM.writeLog("AssetManager::scanAndProcess() - Snapshot has %zu files after scan",
			m_scanner.GetSnapshotSize());

		// Persist after a pass
		if (!m_cfg.databaseFile.empty())
			m_db.Save(m_cfg.databaseFile);

		// NEW: Process any completed compilations
		processCompilationQueue();
	}

	// ==================== UTILITY ====================

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

			std::string expectedDescriptorPath = m_descGen.DefaultDescPathForRecord(*rec);

			if (!fs::exists(expectedDescriptorPath)) {
				LM.writeLog("AssetManager - Missing descriptor for %s, regenerating...",
					rec->sourcePath.c_str());

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
		auto allRecords = const_cast<AssetDatabase&>(m_db).AllMutable();

		for (const auto* rec : allRecords) {
			if (!rec) continue;

			fs::path p(rec->sourcePath);
			if (p.filename().string() == filename) {
				return rec->id;
			}
		}
		return 0;
	}

	const AssetRecord* AssetManager::getAssetRecord(AssetId id) const {
		return m_db.Find(id);
	}

	bool AssetManager::assetExists(const std::string& sourcePath) const {
		return m_db.FindBySource(sourcePath) != nullptr;
	}

} // end of namespace gam300