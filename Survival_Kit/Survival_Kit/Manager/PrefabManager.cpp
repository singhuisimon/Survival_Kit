#include "PrefabManager.h"
#include "ECSManager.h"
#include "LogManager.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include "../Component/AudioComponent.h"

// Use same RapidJSON includes as your SerialisationManager
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace gam300 {

    int PrefabManager::startUp() {
        LM.writeLog("PrefabManager::startUp() - Starting Prefab Manager");

        if (!std::filesystem::exists(m_prefabDirectory)) {
            try {
                std::filesystem::create_directories(m_prefabDirectory);
            }
            catch (const std::exception& e) {
                LM.writeLog("PrefabManager::startUp() - Failed to create prefab directory: %s", e.what());
                return -1;
            }
        }

        int loadedCount = loadAllPrefabs();
        LM.writeLog("PrefabManager::startUp() - Loaded %d prefabs", loadedCount);
        return 0;
    }

    void PrefabManager::shutDown() {
        LM.writeLog("PrefabManager::shutDown() - Shutting down");
        m_prefabs.clear();
        Manager::shutDown();
    }

    bool PrefabManager::createPrefabFromEntity(EntityID entityId, const std::string& prefabName, bool saveToFile) {
        Entity* entity = EM.getEntity(entityId);
        if (!entity) {
            LM.writeLog("PrefabManager::createPrefabFromEntity() - Entity %d not found", entityId);
            return false;
        }

        auto prefabData = std::make_shared<PrefabData>(prefabName, m_prefabDirectory + prefabName + ".prefab");

        // Serialize Transform3D
        if (Transform3D* transform = EM.getComponent<Transform3D>(entityId)) {
            auto serializer = SEM.getComponentSerializer("Transform3D");
            if (serializer) {
                prefabData->componentData["Transform3D"] = serializer->serialize(transform);
                prefabData->defaultPosition = transform->getPosition();
                prefabData->defaultRotation = transform->getRotation();
                prefabData->defaultScale = transform->getScale();
            }
        }

        // Serialize RigidBody
        if (RigidBody* rigidBody = EM.getComponent<RigidBody>(entityId)) {
            auto serializer = SEM.getComponentSerializer("RigidBody");
            if (serializer) {
                prefabData->componentData["RigidBody"] = serializer->serialize(rigidBody);
            }
        }

        // Serialize AudioComponent
        if (AudioComponent* audio = EM.getComponent<AudioComponent>(entityId)) {
            auto serializer = SEM.getComponentSerializer("AudioComponent");
            if (serializer) {
                prefabData->componentData["Audio_Component"] = serializer->serialize(audio);
            }
        }

        m_prefabs[prefabName] = prefabData;

        if (saveToFile) {
            return savePrefab(prefabName);
        }
        return true;
    }

    EntityID PrefabManager::instantiatePrefab(const std::string& prefabName, const PrefabInstanceOptions& options) {
        auto it = m_prefabs.find(prefabName);
        if (it == m_prefabs.end()) {
            LM.writeLog("PrefabManager::instantiatePrefab() - Prefab '%s' not found", prefabName.c_str());
            return INVALID_ENTITY_ID;
        }

        const auto& prefabData = it->second;

        std::string entityName = options.entityName;
        if (entityName.empty() || options.generateUniqueName) {
            entityName = generateUniqueEntityName(entityName.empty() ? prefabName : entityName, prefabName);
        }

        Entity& entity = EM.createEntity(entityName);
        EntityID entityId = entity.get_id();

        if (!createComponentsFromPrefab(entityId, *prefabData, options)) {
            EM.destroyEntity(entityId);
            return INVALID_ENTITY_ID;
        }

        applyTransformOverride(entityId, *prefabData, options);

        LM.writeLog("PrefabManager::instantiatePrefab() - Created entity %d from prefab '%s'", entityId, prefabName.c_str());
        return entityId;
    }

    EntityID PrefabManager::instantiatePrefabAt(const std::string& prefabName, const Vector3D& position, const std::string& customName) {
        PrefabInstanceOptions options;
        options.position = position;
        options.useDefaultTransform = false;
        if (!customName.empty()) {
            options.entityName = customName;
            options.generateUniqueName = false;
        }
        return instantiatePrefab(prefabName, options);
    }

    bool PrefabManager::prefabExists(const std::string& prefabName) const {
        return m_prefabs.find(prefabName) != m_prefabs.end();
    }

    std::vector<std::string> PrefabManager::getAllPrefabNames() const {
        std::vector<std::string> names;
        for (const auto& pair : m_prefabs) {
            names.push_back(pair.first);
        }
        return names;
    }

    size_t PrefabManager::getPrefabCount() const {
        return m_prefabs.size();
    }

    // Helper methods implementation (simplified for space)
    std::string PrefabManager::generateUniqueEntityName(const std::string& baseName, const std::string& prefabName) {
        return baseName + "_" + std::to_string(++m_instanceCounter);
    }

    bool PrefabManager::createComponentsFromPrefab(EntityID entityId, const PrefabData& prefabData, const PrefabInstanceOptions& options) {
        for (const auto& componentPair : prefabData.componentData) {
            std::string formattedJson = "{\"" + componentPair.first + "\": " + componentPair.second + "}";
            if (!SEM.parseComponents(entityId, formattedJson)) {
                return false;
            }
        }
        return true;
    }

    void PrefabManager::applyTransformOverride(EntityID entityId, const PrefabData& prefabData, const PrefabInstanceOptions& options) {
        Transform3D* transform = EM.getComponent<Transform3D>(entityId);
        if (!transform) return;

        if (options.useDefaultTransform) {
            transform->setPosition(prefabData.defaultPosition);
            transform->setRotation(prefabData.defaultRotation);
            transform->setScale(prefabData.defaultScale);
        }
        else {
            transform->setPosition(options.position);
            transform->setRotation(options.rotation);
            transform->setScale(options.scale);
        }
    }

    // Add minimal implementations for other required methods
    bool PrefabManager::savePrefab(const std::string& prefabName, const std::string& filePath) {
        // Simplified - just log for now
        LM.writeLog("PrefabManager::savePrefab() - Saving %s", prefabName.c_str());
        return true;
    }

    bool PrefabManager::loadPrefab(const std::string& filePath) {
        // Simplified - just log for now
        LM.writeLog("PrefabManager::loadPrefab() - Loading %s", filePath.c_str());
        return true;
    }

    int PrefabManager::loadAllPrefabs() {
        return 0; // Simplified for now
    }

    bool PrefabManager::parsePrefabFile(const std::string& filePath, PrefabData& outPrefabData) {
        return true; // Simplified for now
    }

    std::string PrefabManager::getIndent(int level) const {
        return std::string(level * 2, ' ');
    }

} // namespace gam300