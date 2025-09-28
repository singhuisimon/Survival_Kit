#include "AssetDatabase.h"
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <algorithm>

//external library for GUID
#include "../include/xresource_guid-main/source/xresource_guid.h"

namespace gam300 {

	//generate Asset IDs from the GUID library 
	static AssetId GenId()
	{
		const xresource::instance_guid inst = xresource::instance_guid::GenerateGUIDCopy();

		return static_cast<AssetId>(inst.m_Value);
	}

	std::string AssetDatabase::NormalizePath(const std::string& path)
	{
		// Convert to a canonical, forward-slash style so map keys are stable
		namespace fs = std::filesystem;
		fs::path p(path);
		p = p.lexically_normal();
		return p.generic_string();
	}

	std::string AssetDatabase::ExtensionLower(const std::string& path)
	{
		namespace fs = std::filesystem;
		std::string ext = fs::path(path).extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
			});
		return ext;
	}

	bool AssetDatabase::Load(const std::string& file)
	{
		// Reset in-memory state first to avoid mixing sessions
		byId.clear();
		bySourcePath.clear();


		std::ifstream in(file);
		if (!in) return false; // caller may log the filename if needed


		// Format per line:
		// id|source|intermediate|compiled|type|hash|lastWrite|valid\n
		std::string line;
		while (std::getline(in, line))
		{
			if (line.empty()) continue;
			std::istringstream ss(line);


			AssetRecord rec;
			int typeInt = 0;
			int validInt = 0;


			// Read id until pipe
			ss >> rec.id; ss.ignore(1); // skip '|'
			// Read delimited strings
			std::getline(ss, rec.sourcePath, '|');
			std::getline(ss, rec.intermediatePath, '|');
			std::getline(ss, rec.compiledPath, '|');
			// Read type, skip '|'
			ss >> typeInt; ss.ignore(1);
			// Read hash (may be empty), then lastWrite, valid
			std::getline(ss, rec.contentHash, '|');
			ss >> rec.lastWriteTime; ss.ignore(1);
			ss >> validInt;


			rec.type = static_cast<AssetType>(typeInt);
			rec.sourcePath = NormalizePath(rec.sourcePath);
			rec.ext = ExtensionLower(rec.sourcePath);
			rec.valid = (validInt != 0);


			// Insert into maps
			bySourcePath[rec.sourcePath] = rec.id;
			byId[rec.id] = std::move(rec);
		}
		return true;
	}

	bool AssetDatabase::Save(const std::string& file) const
	{
		std::ofstream out(file);
		if (!out) return false;


		// Write each record in one line with pipe separators
		for (const auto& [id, rec] : byId)
		{
			out << rec.id << "|" << rec.sourcePath << "|" << rec.intermediatePath
				<< "|" << rec.compiledPath << "|" << static_cast<int>(rec.type) << "|"
				<< rec.contentHash << "|" << rec.lastWriteTime << "|" << (rec.valid ? 1 : 0)
				<< "\n";
		}
		return true;
	}

	AssetId AssetDatabase::EnsureIdForPath(const std::string& path)
	{
		const std::string key = NormalizePath(path);


		if (auto it = bySourcePath.find(key); it != bySourcePath.end())
			return it->second;


		// Create new record with generated id
		AssetId id = GenId();
		AssetRecord rec;
		rec.id = id;
		rec.sourcePath = key;
		rec.ext = ExtensionLower(key);


		byId[id] = rec;
		bySourcePath[key] = id;
		return id;
	}

	const AssetRecord* AssetDatabase::Find(AssetId id) const
	{
		auto it = byId.find(id);
		return (it == byId.end()) ? nullptr : &it->second;
	}

	AssetRecord* AssetDatabase::FindMutable(AssetId id)
	{
		auto it = byId.find(id);
		return (it == byId.end()) ? nullptr : &it->second;
	}

	const AssetRecord* AssetDatabase::FindBySource(const std::string& path) const
	{
		const std::string key = NormalizePath(path);
		auto it = bySourcePath.find(key);
		if (it == bySourcePath.end()) return nullptr;
		return Find(it->second);
	}

	AssetRecord* AssetDatabase::FindBySourceMutable(const std::string& path)
	{
		const std::string key = NormalizePath(path);
		auto it = bySourcePath.find(key);
		if (it == bySourcePath.end()) return nullptr;
		return FindMutable(it->second);
	}

	bool AssetDatabase::Remove(AssetId id)
	{
		auto it = byId.find(id);
		if (it == byId.end()) return false;
		bySourcePath.erase(it->second.sourcePath);
		byId.erase(it);
		return true;
	}

	bool AssetDatabase::RemoveBySource(const std::string& path)
	{
		const std::string key = NormalizePath(path);
		auto it = bySourcePath.find(key);
		if (it == bySourcePath.end()) return false;
		return Remove(it->second);
	}

	std::vector<AssetRecord*> AssetDatabase::AllMutable()
	{
		std::vector<AssetRecord*> v;
		v.reserve(byId.size());
		for (auto& [id, rec] : byId)
			v.push_back(&rec);
		return v;
	}

	void AssetDatabase::Clear()
	{
		byId.clear();
		bySourcePath.clear();
	}

} //end of namespace gam300