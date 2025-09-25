#include "TextureImporter.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300
{
	bool TextureImporter::CanImport(const std::string& ext) const {
		std::string lower = ext;
		std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

		return (lower == ".png" || lower == ".jpg" || lower == ".jpeg" || lower == ".tga");
	}

    ImportResult TextureImporter::Import(const std::string& srcPath,
        const std::string& intermediateDir) {
        ImportResult r;
        try {
            fs::path src(srcPath);
            fs::path out = fs::path(intermediateDir) / src.filename();

            fs::create_directories(out.parent_path());
            fs::copy_file(src, out, fs::copy_options::overwrite_existing);

            r.ok = true;
            r.intermediatePath = out.string();
            r.type = AssetType::Texture2D;
        }
        catch (const std::exception& e) {
            r.ok = false;
            r.error = e.what();
        }
        return r;
    }

} //end of namepsace gam300