#include "AssetManager.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
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

	// Helper function convert AssetType to ResourceType
	ResourceType assetTypeToResourceType(AssetType assetType) {
		switch (assetType) {
		case AssetType::Texture: return ResourceType::TEXTURE;
		case AssetType::Mesh: return ResourceType::MESH;
		case AssetType::Audio: return ResourceType::AUDIO;
		case AssetType::Shader: return ResourceType::SHADER;
		case AssetType::Material: return ResourceType::MATERIAL;
		default: return ResourceType::UNKNOWN;
		}
	}

	// NEW: Helper function to read properties from descriptor file
	std::unique_ptr<ResourceProperties> readPropertiesFromDescriptor(
		const std::string& descriptorPath,
		AssetType assetType)
	{
		if (!fs::exists(descriptorPath)) {
			LM.writeLog("AssetManager - Descriptor file not found: %s", descriptorPath.c_str());
			return nullptr;
		}

		std::ifstream file(descriptorPath);
		if (!file.is_open()) {
			LM.writeLog("AssetManager - Cannot open descriptor: %s", descriptorPath.c_str());
			return nullptr;
		}

		// Read entire file
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string json = buffer.str();
		file.close();

		// Helper function to parse JSON values
		auto parseJsonValue = [&json](const std::string& fieldPath) -> std::string {
			std::string searchKey = "\"" + fieldPath + "\":";
			size_t pos = json.find(searchKey);
			if (pos == std::string::npos) return "";

			pos += searchKey.length();
			while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

			if (pos >= json.length()) return "";

			if (json[pos] == '"') {
				pos++;
				size_t end = json.find('"', pos);
				if (end != std::string::npos) {
					return json.substr(pos, end - pos);
				}
			}
			return "";
			};

		auto parseJsonBool = [&json](const std::string& fieldPath) -> bool {
			std::string searchKey = "\"" + fieldPath + "\":";
			size_t pos = json.find(searchKey);
			if (pos == std::string::npos) return false;

			pos += searchKey.length();
			while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

			return (json.substr(pos, 4) == "true");
			};

		auto parseJsonFloat = [&json](const std::string& fieldPath) -> float {
			std::string searchKey = "\"" + fieldPath + "\":";
			size_t pos = json.find(searchKey);
			if (pos == std::string::npos) return 0.0f;

			pos += searchKey.length();
			while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

			size_t end = pos;
			while (end < json.length() && (json[end] == '.' || json[end] == '-' ||
				(json[end] >= '0' && json[end] <= '9'))) {
				end++;
			}

			if (end > pos) {
				try {
					return std::stof(json.substr(pos, end - pos));
				}
				catch (...) {
					return 0.0f;
				}
			}
			return 0.0f;
			};

		auto parseJsonInt = [&json](const std::string& fieldPath) -> int {
			std::string searchKey = "\"" + fieldPath + "\":";
			size_t pos = json.find(searchKey);
			if (pos == std::string::npos) return 0;

			pos += searchKey.length();
			while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

			size_t end = pos;
			while (end < json.length() && (json[end] == '-' || (json[end] >= '0' && json[end] <= '9'))) {
				end++;
			}

			if (end > pos) {
				try {
					return std::stoi(json.substr(pos, end - pos));
				}
				catch (...) {
					return 0;
				}
			}
			return 0;
			};

		// Create properties based on asset type and populate from descriptor
		switch (assetType) {
		case AssetType::Texture: {
			auto props = std::make_unique<TextureProperties>();

			// Read texture settings from descriptor
			std::string compression = parseJsonValue("compression");
			if (!compression.empty()) {
				props->compressionFormat = compression;
			}

			// Parse quality (stored as float 0-1 in descriptor, but compressionQuality is int 0-100)
			float qualityFloat = parseJsonFloat("quality");
			if (qualityFloat > 0.0f) {
				props->compressionQuality = static_cast<int>(qualityFloat * 100.0f);
			}

			props->generateMipmaps = parseJsonBool("generateMipmaps");
			props->srgb = parseJsonBool("srgb");

			// These might not be in all descriptors, keep defaults if not found
			int width = parseJsonInt("maxWidth");
			if (width > 0) {
				props->maxWidth = width;
			}

			int height = parseJsonInt("maxHeight");
			if (height > 0) {
				props->maxHeight = height;
			}

			LM.writeLog("AssetManager - Loaded texture properties: compression=%s, quality=%d, mipmaps=%d, srgb=%d",
				props->compressionFormat.c_str(),
				props->compressionQuality,
				props->generateMipmaps,
				props->srgb);

			return props;
		}

		case AssetType::Mesh: {
			auto props = std::make_unique<MeshProperties>();

			// Read mesh-specific properties from descriptor if available
			float scale = parseJsonFloat("scaleFactor");
			if (scale > 0.0f) {
				props->scaleFactor = scale;
			}

			// These might be stored in descriptor
			props->optimizeVertices = parseJsonBool("optimizeVertices");
			props->generateNormals = parseJsonBool("generateNormals");
			props->generateTangents = parseJsonBool("generateTangents");

			LM.writeLog("AssetManager - Loaded mesh properties: scale=%.2f, optimize=%d, normals=%d, tangents=%d",
				props->scaleFactor,
				props->optimizeVertices,
				props->generateNormals,
				props->generateTangents);

			return props;
		}

		case AssetType::Audio: {
			auto props = std::make_unique<AudioProperties>();

			// Read audio-specific properties from descriptor if available
			int sampleRate = parseJsonInt("sampleRate");
			if (sampleRate > 0) {
				props->sampleRate = sampleRate;
			}

			int channels = parseJsonInt("channels");
			if (channels > 0) {
				props->channels = channels;
			}

			std::string compressionFormat = parseJsonValue("compressionFormat");
			if (!compressionFormat.empty()) {
				props->compressionFormat = compressionFormat;
			}

			LM.writeLog("AssetManager - Loaded audio properties: sampleRate=%d, channels=%d, format=%s",
				props->sampleRate,
				props->channels,
				props->compressionFormat.c_str());

			return props;
		}

		case AssetType::Shader: {
			auto props = std::make_unique<ShaderProperties>();

			// Read shader-specific properties from descriptor if available
			std::string vertPath = parseJsonValue("vertexShaderPath");
			if (!vertPath.empty()) {
				props->vertexShaderPath = vertPath;
			}

			std::string fragPath = parseJsonValue("fragmentShaderPath");
			if (!fragPath.empty()) {
				props->fragmentShaderPath = fragPath;
			}

			props->enableDebugInfo = parseJsonBool("enableDebugInfo");

			LM.writeLog("AssetManager - Loaded shader properties: vert=%s, frag=%s, debug=%d",
				props->vertexShaderPath.c_str(),
				props->fragmentShaderPath.c_str(),
				props->enableDebugInfo);

			return props;
		}

		default:
			LM.writeLog("AssetManager - Unknown asset type, cannot create properties");
			return nullptr;
		}
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
			return p.is_absolute() ? p.string() : (base / p).lexically_normal().string();
			};

		// Resolve paths
		for (auto& r : m_cfg.sourceRoots)
			r = Resolve(r);

		m_cfg.intermediateDirectory = Resolve(m_cfg.intermediateDirectory);
		m_cfg.databaseFile = Resolve(m_cfg.databaseFile);
		m_cfg.snapshotFile = Resolve(m_cfg.snapshotFile);

		if (!m_cfg.descriptorSidecar && !m_cfg.descriptorRoot.empty())
			m_cfg.descriptorRoot = Resolve(m_cfg.descriptorRoot);

		// Ensure intermediate directory exists
		fs::create_directories(m_cfg.intermediateDirectory);

		// Configure scanner (FIXED: Use lowercase method names)
		m_scanner.setRoots(m_cfg.sourceRoots);
		m_scanner.setExtensions(m_cfg.scanExtensions);
		m_scanner.setIgnoreSubstrings(m_cfg.ignoreSubstrings);
		m_scanner.setIncludeHidden(m_cfg.includeHidden);
		m_scanner.setFollowSymlinks(m_cfg.followSymlinks);

		if (!m_cfg.snapshotFile.empty())
			m_scanner.LoadSnapshot(m_cfg.snapshotFile);

		RegisterDefaultImporters(m_importers);

		if (!m_cfg.databaseFile.empty())
			m_db.Load(m_cfg.databaseFile);

		m_descGen.SetSidecar(m_cfg.descriptorSidecar);
		m_descGen.SetOutputRoot(m_cfg.descriptorRoot);
		m_descGen.SetPretty(true);

		// NEW: Initialize compiler system
		initializeCompilers();

		LM.writeLog("AssetManager::startUp() - complete");
		return 0;
	}

	void AssetManager::shutDown() {
		// NEW: Shutdown compilers before other systems
		shutdownCompilers();

		if (!m_cfg.databaseFile.empty())
			m_db.Save(m_cfg.databaseFile);

		if (!m_cfg.snapshotFile.empty()) {
			const size_t snapCount = m_scanner.GetSnapshotSize();
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

		// FIXED: Read properties from descriptor file instead of creating empty ones
		std::string descriptorPath = m_descGen.DefaultDescPathForRecord(*rec);
		job.properties = readPropertiesFromDescriptor(descriptorPath, rec->type);

		// If we couldn't read properties from descriptor, create defaults as fallback
		if (!job.properties) {
			LM.writeLog("AssetManager - Failed to read properties from descriptor, using defaults for: %s",
				rec->sourcePath.c_str());

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
				LM.writeLog("AssetManager - Unknown asset type, cannot queue compilation");
				return; // Unknown type
			}
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
				assetTypeToResourceType(job.assetType),
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