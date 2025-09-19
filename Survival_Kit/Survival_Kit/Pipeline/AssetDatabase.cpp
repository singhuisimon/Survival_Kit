#include "AssetDatabase.h"
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

static AssetId GenId()
{
	static std::mt19937_64 rng{ 0x57AF1A1DULL};
	static std::uniform_int_distribution<unsigned long long> d;
	return d(rng);
}

bool AssetDatabase::Load(const std::string& file)
{
	byId.clear();  bySourcePath.clear();
	std::ifstream in(file);
	if (!in) return false;
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		std::istringstream ss(line);
		AssetRecord rec; int typeInt, validInt;
		ss >> rec.id; ss.ignore(1);
		std::getline(ss, rec.sourcePath, '|');
		std::getline(ss, rec.intermediatePath, '|');
		std::getline(ss, rec.compiledPath, '|');
		ss >> typeInt; ss.ignore(1);
		std::getline(ss, rec.contentHash, '|');
		ss >> rec.lastWriteTime; ss.ignore(1);
		ss >> validInt;
		rec.type = static_cast<AssetType>(typeInt);
		rec.ext  = std::filesystem::path(rec.sourcePath).extension().string();
		rec.valid = validInt != 0;
		byId[rec.id] = rec;
		bySourcePath[rec.sourcePath] = rec.id;
	}
	return true;
}

bool AssetDatabase::Save(const std::string& file)  const {
	std::ofstream out(file);
	if (!out) return false;
	for (auto& [id, rec] : byId) {
		out << rec.id << "|" << rec.sourcePath << "|" << rec.intermediatePath
			<< "|" << rec.compiledPath << "|" << (int)rec.type << "|"
			<< rec.contentHash << "|" << rec.lastWriteTime << "|" << (rec.valid ? 1 : 0)
			<< "\n";
	}
	return true;
}