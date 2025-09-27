#include "ShaderImporter.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 
{

    bool ShaderImporter::CanImport(const std::string& ext) const {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return (lower == ".hlsl" || lower == ".glsl" || lower == ".vert" || lower == ".frag");
    }

    ImportResult ShaderImporter::Import(const std::string& srcPath,
        const std::string& intermediateDir) {
        ImportResult r;
        try {
            fs::path src(srcPath);
            fs::path out = fs::path(intermediateDir) / src.filename();

            fs::create_directories(out.parent_path());
            fs::copy_file(src, out, fs::copy_options::overwrite_existing);

            r.ok = true;
            r.intermediatePath = out.string();
            r.type = AssetType::Shader;
        }
        catch (const std::exception& e) {
            r.ok = false;
            r.error = e.what();
        }
        return r;
    }

} // end of namespace gam300