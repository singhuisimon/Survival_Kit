#pragma once
#ifndef __PREFAB_MANAGER_H__
#define __PREFAB_MANAGER_H__

#include "Manager.h"
#include "SerialisationManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../Utility/ECS_Variables.h"
#include "../Utility/Vector3D.h"

#define PM gam300::PrefabManager::getInstance()

namespace gam300 {
    class Entity;
    class Transform3D;

    struct PrefabData {
        std::string name;
        std::string filePath;
        std::unordered_map<std::string, std::string> componentData;
        Vector3D defaultPosition{ 0.0f, 0.0f, 0.0f };
        Vector3D defaultRotation{ 0.0f, 0.0f, 0.0f };
        Vector3D defaultScale{ 1.0f, 1.0f, 1.0f };

        PrefabData() = default;
        PrefabData(const std::string& prefabName, const std::string& path)
            : name(prefabName), filePath(path) {
        }
    };

    struct PrefabInstanceOptions {
        std::string entityName;
        Vector3D position{ 0.0f, 0.0f, 0.0f };
        Vector3D rotation{ 0.0f, 0.0f, 0.0f };
        Vector3D scale{ 1.0f, 1.0f, 1.0f };
        bool useDefaultTransform = true;
        bool generateUniqueName = true;

        PrefabInstanceOptions() = default;
        PrefabInstanceOptions(const std::string& name) : entityName(name) {}
    };

    class PrefabManager : public Manager {
    private:
        std::unordered_map<std::string, std::shared_ptr<PrefabData>> m_prefabs;
        std::string m_prefabDirectory = "Assets/Prefabs/";
        int m_instanceCounter = 0;
        PrefabManager() = default;

    public:
        PrefabManager(const PrefabManager&) = delete;
        PrefabManager& operator=(const PrefabManager&) = delete;

        static PrefabManager& getInstance() {
            static PrefabManager instance;
            return instance;
        }

        int startUp() override;
        void shutDown() override;

        // Core methods
        bool createPrefabFromEntity(EntityID entityId, const std::string& prefabName, bool saveToFile = true);
        EntityID instantiatePrefab(const std::string& prefabName, const PrefabInstanceOptions& options = {});
        EntityID instantiatePrefabAt(const std::string& prefabName, const Vector3D& position, const std::string& customName = "");

        // Management
        bool prefabExists(const std::string& prefabName) const;
        std::vector<std::string> getAllPrefabNames() const;
        size_t getPrefabCount() const;

    private:
        std::string generateUniqueEntityName(const std::string& baseName, const std::string& prefabName);
        bool parsePrefabFile(const std::string& filePath, PrefabData& outPrefabData);
        bool createComponentsFromPrefab(EntityID entityId, const PrefabData& prefabData, const PrefabInstanceOptions& options);
        void applyTransformOverride(EntityID entityId, const PrefabData& prefabData, const PrefabInstanceOptions& options);
        std::string getIndent(int level) const;
        bool savePrefab(const std::string& prefabName, const std::string& filePath = "");
        bool loadPrefab(const std::string& filePath);
        int loadAllPrefabs();
    };
}

#endif // __PREFAB_MANAGER_H__