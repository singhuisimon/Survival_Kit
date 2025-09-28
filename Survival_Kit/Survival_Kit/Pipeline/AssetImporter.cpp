#include "AssetImporter.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>

//all the other seperated importer for different asset types
#include "Importers/AudioImporter.h"
#include "Importers/MeshImporter.h"
#include "Importers/SceneImporter.h"
#include "Importers/ShaderImporter.h"
#include "Importers/TextureImporter.h"


namespace fs = std::filesystem;

namespace gam300 {

	namespace {

		// Normalize an extension to lowercase without a leading dot.
		std::string NormalizeExt(std::string ext) {
			if (!ext.empty() && ext[0] == '.') ext.erase(ext.begin());
			std::transform(ext.begin(), ext.end(), ext.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return ext;
		}

		// -------------- OPTIONAL --------------
		// Compute a 64-bit FNV-1a hash of a file, returned as 16-hex chars.
		std::string HashFileFNV1a64(const std::string& path) {
			static constexpr unsigned long long FNV_OFFSET = 1469598103934665603ull;
			static constexpr unsigned long long FNV_PRIME = 1099511628211ull;
			unsigned long long h = FNV_OFFSET;


			std::ifstream f(path, std::ios::binary);
			if (!f) return {};
			char buf[64 * 1024];
			while (f) {
				f.read(buf, sizeof(buf));
				std::streamsize n = f.gcount();
				for (std::streamsize i = 0; i < n; ++i) {
					h ^= static_cast<unsigned char>(buf[i]);
					h *= FNV_PRIME;
				}
			}
			// to hex
			char out[17]{};
			static const char* HEX = "0123456789abcdef";
			for (int i = 15; i >= 0; --i) { out[i] = HEX[h & 0xF]; h >>= 4; }
			return std::string(out, 16);
		}

		// Build an intermediate file path in the given directory with a stable naming scheme.
		std::string BuildIntermediatePath(const std::string& srcPath, const std::string& intermediateDir) {
			fs::path p(srcPath);
			const std::string name = p.filename().string();
			// Keep original filename and append .intermediate to make it obvious.
			const std::string outName = name + ".intermediate";
			return (fs::path(intermediateDir) / outName).string();
		}

		// Copy file with directory creation; returns true on success.
		bool CopyTo(const std::string& src, const std::string& dst) {
			std::error_code ec;
			fs::create_directories(fs::path(dst).parent_path(), ec);
			ec.clear();
			fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
			return !ec;
		}

		// Classify a file extension to AssetType.
		AssetType GuessTypeFromExt(const std::string& extNoDotLower) {
			const std::string e = extNoDotLower;
			if (e == "png" || e == "jpg" || e == "jpeg" || e == "bmp" || e == "tga" || e == "dds")
				return AssetType::Texture;
			if (e == "wav" || e == "ogg" || e == "mp3" || e == "flac")
				return AssetType::Audio;
			if (e == "obj" || e == "fbx" || e == "gltf" || e == "glb")
				return AssetType::Mesh;
			if (e == "mat")
				return AssetType::Material;
			if (e == "vert" || e == "frag" || e == "glsl" || e == "hlsl" || e == "shader")
				return AssetType::Shader;
			if (e == "scene" || e == "scn" || e == "json")
				return AssetType::Scene;
			return AssetType::Unknown;
		}

		// Minimal copy importers
		class CopyImporter : public IAssetImporter {
		public:
			explicit CopyImporter(std::vector<std::string> exts) {
				for (auto& e : exts) m_exts.push_back(NormalizeExt(e));
			}
			bool CanImport(const std::string& ext) const override {
				const std::string e = NormalizeExt(ext);
				return std::find(m_exts.begin(), m_exts.end(), e) != m_exts.end();
			}
			ImportResult Import(const std::string& srcPath, const std::string& intermediateDir) override {
				ImportResult r;
				const std::string ext = NormalizeExt(fs::path(srcPath).extension().string());
				r.type = GuessTypeFromExt(ext);
				r.intermediatePath = BuildIntermediatePath(srcPath, intermediateDir);
				if (!CopyTo(srcPath, r.intermediatePath)) {
					r.ok = false; r.error = "Copy failed"; return r;
				}
				r.contentHash = HashFileFNV1a64(srcPath);
				r.ok = true;
				return r;
			}
		private:
			std::vector<std::string> m_exts;
		};


		// A generic fallback: accepts any extension and just copies.
		class FallbackImporter : public IAssetImporter {
		public:
			bool CanImport(const std::string&) const override { return true; }
			ImportResult Import(const std::string& srcPath, const std::string& intermediateDir) override {
				ImportResult r;
				const std::string ext = NormalizeExt(fs::path(srcPath).extension().string());
				r.type = GuessTypeFromExt(ext);
				r.intermediatePath = BuildIntermediatePath(srcPath, intermediateDir);
				if (!CopyTo(srcPath, r.intermediatePath)) {
					r.ok = false; r.error = "Copy failed"; return r;
				}
				r.contentHash = HashFileFNV1a64(srcPath);
				r.ok = true;
				return r;
			}
		};

	}  //end of anonymous namespace

	//registry
	void AssetImporterRegistry::Register(std::unique_ptr<IAssetImporter> imp) {
		m_importers.push_back(std::move(imp));
	}


	ImportResult AssetImporterRegistry::Import(const std::string& srcPath,
		const std::string& intermediateDir)
	{
		const std::string ext = fs::path(srcPath).extension().string();
		for (auto& imp : m_importers) {
			if (imp->CanImport(ext))
				return imp->Import(srcPath, intermediateDir);
		}
		// If nothing registered claims it, run the permissive fallback.
		FallbackImporter fb;
		return fb.Import(srcPath, intermediateDir);
	}

	// ---------------- Defaults ----------------
	void RegisterDefaultImporters(AssetImporterRegistry& reg)
	{
		reg.Register(std::make_unique<TextureImporter>());
		reg.Register(std::make_unique<MeshImporter>());
		reg.Register(std::make_unique<AudioImporter>());
		reg.Register(std::make_unique<ShaderImporter>());
		reg.Register(std::make_unique<SceneImporter>());

		//// Texture-like formats
		//reg.Register(std::make_unique<CopyImporter>(std::vector<std::string>{
			//".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds"
		//}));
		//// Audio-like formats
		//reg.Register(std::make_unique<CopyImporter>(std::vector<std::string>{
			//".wav", ".ogg", ".mp3", ".flac"
		//}));
		//// Mesh-like formats
		//reg.Register(std::make_unique<CopyImporter>(std::vector<std::string>{
			//".obj", ".fbx", ".gltf", ".glb"
		//}));
		//// Materials / Shaders / Scenes
		//reg.Register(std::make_unique<CopyImporter>(std::vector<std::string>{
			//".mat", ".vert", ".frag", ".glsl", ".hlsl", ".shader", ".scene", ".scn", ".json"
		//}));
	}

}	//end of namespace gam300