#include "AssetDescriptorGenerator.h"
#include "AssetDatabase.h" // AssetRecord definition

#include <filesystem>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace gam300 {

	// API
	void AssetDescriptorGenerator::SetSidecar(bool sidecar) { m_sidecar = sidecar; }
	void AssetDescriptorGenerator::SetOutputRoot(const std::string& root) { m_outputRoot = root; }
	void AssetDescriptorGenerator::SetPretty(bool pretty) { m_pretty = pretty; }


	bool AssetDescriptorGenerator::GenerateFor(const AssetRecord& rec,
		const DescriptorExtras* extras, std::string* outPath) const
	{
	    //compute default descriptor file path from record
        const std::string path = DefaultDescPathForRecord(rec);
        if (!EnsureParentDir(path)) return false;

        //build JSON contents
        const std::string json = BuildJson(&rec, extras);

        //write file
        if (!WriteText(path, json)) return false;

        if (outPath) *outPath = path;
        return true;
	}



	bool AssetDescriptorGenerator::GenerateForPath(AssetDatabase& db, const std::string& sourcePath,
		const DescriptorExtras* extras, std::string* outPath) const
	{
        //ensure the database has a stable GUID for this source path
        AssetId id = db.EnsureIdForPath(sourcePath);
        AssetRecord* rec = db.FindMutable(id);
        if (!rec) return false;

        //compute descriptor file path
        const std::string path = DefaultDescPathForRecord(*rec);
        if (!EnsureParentDir(path)) return false;

        //build json for this record
        const std::string json = BuildJson(rec, extras);

        //write to file
        if (!WriteText(path, json)) return false;

        if (outPath) *outPath = path;
        return true;
	}


    std::string AssetDescriptorGenerator::DefaultDescPathForRecord(const AssetRecord& rec) const
	{
        //convert the GUID into a 16-character uppercase hex string
        std::ostringstream ss;
        ss << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << rec.id;
        const std::string guid = ss.str();

        //use first 2 + next 2 characters as subfolders (the two directories)
        const std::string dir1 = guid.substr(0, 2);
        const std::string dir2 = guid.substr(2, 2);

        //map asset type to folder
        std::string typeFolder;

        switch (rec.type) {
            case AssetType::Texture: typeFolder = "Texture"; break;
            case AssetType::Mesh:      typeFolder = "Mesh";    break;
            case AssetType::Material:  typeFolder = "Material"; break;
            case AssetType::Shader:    typeFolder = "Shader";  break;
            case AssetType::Audio:     typeFolder = "Audio";   break;
            case AssetType::Scene:     typeFolder = "Scene";   break;
            default:                   typeFolder = "Unknown"; break;
        }

        //final path
        fs::path dir = "AssetDescriptors";
        dir /= typeFolder;
        dir /= dir1;
        dir /= dir2;
        dir /= guid + ".desc";

        fs::create_directories(dir);
        return (dir / "Descriptor.txt").string();
	}

	
	// JSON Builder
    /**
     * @brief Convert (optional) AssetRecord + Extras to a JSON string.
    * @notes We intentionally avoid any external JSON dependency for simplicity.
    */
    std::string AssetDescriptorGenerator::BuildJson(const AssetRecord* recOpt,
        const DescriptorExtras* extras) const
    {
        std::ostringstream o;
        auto nl = [&](int n) { if (m_pretty) for (int i = 0; i < n; ++i) o << '\n'; };
        auto ind = [&](int n) { if (m_pretty) for (int i = 0; i < n; ++i) o << "  "; };

        o << "{"; nl(1);

        // ---- asset (pipeline record) ----
        ind(1); o << "\"asset\": ";
        if (recOpt) {
            o << "{"; nl(1);

            // GUID in hex (readable)
            std::ostringstream ss;
            ss << std::uppercase << std::hex << std::setw(16) << std::setfill('0') << recOpt->id;

            ind(2); o << "\"guid\": \"" << ss.str() << "\","; nl(1);
            ind(2); o << "\"id\": " << recOpt->id << ","; nl(1);
            ind(2); o << "\"sourcePath\": \"" << EscapeJson(recOpt->sourcePath) << "\","; nl(1);
            ind(2); o << "\"intermediatePath\": \"" << EscapeJson(recOpt->intermediatePath) << "\","; nl(1);
            ind(2); o << "\"compiledPath\": \"" << EscapeJson(recOpt->compiledPath) << "\","; nl(1);
            ind(2); o << "\"type\": " << static_cast<int>(recOpt->type) << ","; nl(1);
            ind(2); o << "\"ext\": \"" << EscapeJson(recOpt->ext) << "\","; nl(1);
            ind(2); o << "\"contentHash\": \"" << EscapeJson(recOpt->contentHash) << "\","; nl(1);
            ind(2); o << "\"lastWriteTime\": " << static_cast<unsigned long long>(recOpt->lastWriteTime) << ","; nl(1);
            ind(2); o << "\"valid\": " << (recOpt->valid ? "true" : "false"); nl(1);

            ind(1); o << "},"; nl(1);
        }
        else {
            o << "null,"; nl(1);
        }

        // ---- extras (generic, resource-agnostic) ----
        ind(1); o << "\"extras\": ";
        if (extras) {
            o << "{"; nl(1);
            ind(2); o << "\"displayName\": \"" << EscapeJson(extras->displayName) << "\","; nl(1);
            ind(2); o << "\"category\": \"" << EscapeJson(extras->category) << "\","; nl(1);
            ind(2); o << "\"tags\": [";
            for (size_t i = 0; i < extras->tags.size(); ++i) {
                if (i) o << ",";
                o << "\"" << EscapeJson(extras->tags[i]) << "\"";
            }
            o << "],"; nl(1);
            ind(2); o << "\"lastImported\": " << static_cast<unsigned long long>(extras->lastImported) << ","; nl(1);
            ind(2); o << "\"user\": {";
            size_t c = 0;
            for (const auto& kv : extras->user) {
                if (c++) o << ",";
                nl(0);
                ind(3); o << "\"" << EscapeJson(kv.first) << "\": \"" << EscapeJson(kv.second) << "\"";
            }
            nl(0); o << "}"; nl(1);
            ind(1); o << "}"; nl(1);
        }
        else {
            o << "null"; nl(1);
        }

        o << "}"; nl(1);
        return o.str();
    }


    std::string AssetDescriptorGenerator::EscapeJson(const std::string& s)
    {
        std::string out; out.reserve(s.size() + 8);
        for (char c : s) {
            switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
            }
        }
        return out;
    }

    bool AssetDescriptorGenerator::EnsureParentDir(const std::string& path)
    {
        try {
            fs::path p(path);
            fs::path dir = p.parent_path();
            if (dir.empty()) return true;
            std::error_code ec;
            fs::create_directories(dir, ec);
            return !ec;
        }
        catch (...) 
        { 
            return false; 
        }
    }

    bool AssetDescriptorGenerator::WriteText(const std::string& path, const std::string& text)
    {
        try {
            std::ofstream ofs(path, std::ios::out | std::ios::trunc);
            if (!ofs.is_open()) return false;
            ofs << text;
            return true;
        }
        catch (...) {
            return false;
        }
    }


}  //end of namespace gam300