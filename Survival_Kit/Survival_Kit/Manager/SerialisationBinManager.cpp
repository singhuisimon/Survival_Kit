#include "../Component/Transform3D.h"      // ensure Transform3D is COMPLETE before templates
#include "ECSManager.h"
#include "LogManager.h"
#include "SerialisationBinManager.h"       // your header with Scene*Bin + SerializerBin

namespace gam300
{

    bool SerializerBin::saveScene(const std::string &filename)
    {
        LM.writeLog("BIN saveScene: '%s'", filename.c_str());

        SceneBin scene{};
        const auto &entities = EM.getAllEntities();
        scene.objects.reserve(entities.size());

        for (const Entity &e : entities)
        {
            SceneObjectBin sob{};
            sob.name = e.get_name();

            if (auto *t = EM.getComponent<Transform3D>(e.get_id()))
            {
                Transform3DBin tb{};

                // copy values (no refs, avoids “references must be initialized”)
                const auto p = t->getPosition();
                const auto pp = t->getPrevPosition();
                const auto r = t->getRotation();
                const auto s = t->getScale();

                tb.position = { p.x,  p.y,  p.z };
                tb.prev_position = { pp.x, pp.y, pp.z }; // parity only
                tb.rotation = { r.x,  r.y,  r.z };
                tb.scale = { s.x,  s.y,  s.z };

                sob.transform3d = tb;
            }

            scene.objects.emplace_back(std::move(sob));
        }

        std::ofstream os(filename, std::ios::binary);
        if (!os.is_open())
        {
            LM.writeLog("BIN saveScene: cannot open '%s'", filename.c_str());
            return false;
        }

        SceneFileHeader hdr{}; // magic=SCNB, version=1.0
        SerializerBin::write_any(os, hdr);
        SerializerBin::write_any(os, scene);

        if (!os)
        {
            LM.writeLog("BIN saveScene: stream error");
            return false;
        }

        LM.writeLog("BIN saveScene: ok (objects=%zu)", scene.objects.size());
        return true;
    }

    bool SerializerBin::loadScene(const std::string &filename)
    {
        LM.writeLog("BIN loadScene: '%s'", filename.c_str());

        std::ifstream is(filename, std::ios::binary);
        if (!is.is_open())
        {
            LM.writeLog("BIN loadScene: cannot open '%s'", filename.c_str());
            return false;
        }

        // header
        SceneFileHeader hdr{};
        SerializerBin::read_any(is, hdr);
        if (!is) { LM.writeLog("BIN loadScene: header read error"); return false; }
        if (hdr.magic != 0x424E4353) { LM.writeLog("BIN loadScene: bad magic"); return false; }
        if (hdr.major != 1) { LM.writeLog("BIN loadScene: unsupported version %u.%u", hdr.major, hdr.minor); return false; }

        // scene
        SceneBin scene{};
        SerializerBin::read_any(is, scene);
        if (!is) { LM.writeLog("BIN loadScene: scene read error"); return false; }

        int created = 0;
        for (const auto &sob : scene.objects)
        {
            Entity &ent = EM.createEntity(sob.name);
            ++created;

            if (sob.transform3d)
            {
                const Transform3DBin &tb = *sob.transform3d;

                Vector3D pos(tb.position.x, tb.position.y, tb.position.z);
                Vector3D rot(tb.rotation.x, tb.rotation.y, tb.rotation.z);
                Vector3D scl(tb.scale.x, tb.scale.y, tb.scale.z);

                Transform3D *comp = EM.addComponent<Transform3D>(ent.get_id(), pos, rot, scl);
                if (!comp)
                {
                    LM.writeLog("BIN loadScene: failed to add Transform3D to entity %d", ent.get_id());
                }
                // prev_position is persisted but not applied (no setter)
            }

            LM.writeLog("BIN loadScene: created entity '%s' (ID %d)",
                ent.get_name().c_str(), ent.get_id());
        }

        if (created == 0)
        {
            LM.writeLog("BIN loadScene: no entities created");
            return false;
        }

        LM.writeLog("BIN loadScene: ok (objects=%d)", created);
        return true;
    }

} // namespace gam300
