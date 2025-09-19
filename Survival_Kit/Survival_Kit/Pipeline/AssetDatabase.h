#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <ctime>

using AssetId = unsigned long long;

enum class AssetType { Unknown, Shader ,Texture2D, Audio, Mesh, Material, Scene };

struct AssetRecord {

	AssetId id = 0;
	std::string sourcePath;
	std::string intermediatePath;
	std::string compiledPath;		// for future use
	AssetType type = AssetType::Unknown;
	std::string ext;		// ".png", ".wav", etc.
	std::string contentHash;		// file-content hash
	std::time_t lastWriteTime = 0;
	bool	valid = false;

};


struct AssetDatabase {
	std::unordered_map<AssetId, AssetRecord> byId;
	std::unordered_map<std::string, AssetId> bySourcePath;		// path -> id

	bool Load(const std::string& file);
	bool Save(const std::string& file) const;

	AssetId EnsureIdForPath(const std::string& path);
	const AssetRecord* Find(AssetId id);
	AssetRecord* FindMutable(AssetId id);
	std::vector<AssetRecord*> AllMutable();
}; 
