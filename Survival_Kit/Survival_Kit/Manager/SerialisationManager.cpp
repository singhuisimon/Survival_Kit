/**
 * @file SerialisationManager.cpp
 * @brief Implementation of the Serialisation Manager for the game engine.
 * @details Handles loading and saving game objects to/from files in .scn format.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "SerialisationManager.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"   
#include "rapidjson/prettywriter.h"    
#include "rapidjson/writer.h" 

#include "LogManager.h"
#include "ECSManager.h"
#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include "../Component/AudioComponent.h"
#include "../Component/Collider.h"

#include "../Component/Script.h"
#include "../Component/MeshComponent.h"
#include "../Component/TextureComponent.h"
#include <fstream>
#include <sstream>
#include <functional>
#include <iomanip>

namespace gam300 {

    // ==================== Transform3D Serializer ====================
    // Transform3DSerializer implementation
    std::string Transform3DSerializer::serialize(Component* component) {
        Transform3D* transform = static_cast<Transform3D*>(component);
        if (!transform) {
            return "{}";
        }

        std::stringstream ss;
        ss << "{\n";

        // Serialize position
        const Vector3D& pos = transform->getPosition();
        ss << "          \"position\": [\n";
        ss << "            " << pos.x << ",\n";
        ss << "            " << pos.y << ",\n";
        ss << "            " << pos.z << "\n";
        ss << "          ],\n";

        // Serialize previous position
        const Vector3D& prevPos = transform->getPrevPosition();
        ss << "          \"prev_position\": [\n";
        ss << "            " << prevPos.x << ",\n";
        ss << "            " << prevPos.y << ",\n";
        ss << "            " << prevPos.z << "\n";
        ss << "          ],\n";

        // Serialize rotation
        const Vector3D& rotation = transform->getRotation();
        ss << "          \"rotation\": [\n";
        ss << "            " << rotation.x << ",\n";
        ss << "            " << rotation.y << ",\n";
        ss << "            " << rotation.z << "\n";
        ss << "          ],\n";

        // Serialize scale
        const Vector3D& scale = transform->getScale();
        ss << "          \"scale\": [\n";
        ss << "            " << scale.x << ",\n";
        ss << "            " << scale.y << ",\n";
        ss << "            " << scale.z << "\n";
        ss << "          ]\n";

        ss << "        }";

        return ss.str();
    }

    // Transform3DDeserializer implementation
    Component* Transform3DSerializer::deserialize(EntityID entityId, const std::string& jsonData) {
        // Parse position
        Vector3D position = Vector3D::ZERO;
        std::string positionData = SerialisationManager::extractObjectValue(jsonData, "position");
        if (!positionData.empty()) {
            std::vector<float> posArray = SerialisationManager::parseFloatArray(positionData);
            if (posArray.size() >= 3) {
                position = Vector3D(posArray[0], posArray[1], posArray[2]);
            }
        }

        // Parse rotation
        Vector3D rotation = Vector3D::ZERO;
        std::string rotationData = SerialisationManager::extractObjectValue(jsonData, "rotation");
        if (!rotationData.empty()) {
            std::vector<float> rotArray = SerialisationManager::parseFloatArray(rotationData);
            if (rotArray.size() >= 3) {
                rotation = Vector3D(rotArray[0], rotArray[1], rotArray[2]);
            }
        }

        // Parse scale
        Vector3D scale = Vector3D::ONE;
        std::string scaleData = SerialisationManager::extractObjectValue(jsonData, "scale");
        if (!scaleData.empty()) {
            std::vector<float> scaleArray = SerialisationManager::parseFloatArray(scaleData);
            if (scaleArray.size() >= 3) {
                scale = Vector3D(scaleArray[0], scaleArray[1], scaleArray[2]);
            }
        }

        // Create the Transform3D component
        Transform3D* transform = EM.addComponent<Transform3D>(entityId, position, rotation, scale);

        // Parse and set previous position if available
        std::string prevPosData = SerialisationManager::extractObjectValue(jsonData, "prev_position");
        if (!prevPosData.empty()) {
            std::vector<float> prevPosArray = SerialisationManager::parseFloatArray(prevPosData);
            if (prevPosArray.size() >= 3) {
                // Since there's no direct setter for prev_position, we'll just log it
                // In a real implementation, you might add a setPrevPosition method
                LM.writeLog("Transform3D::deserialize() - Previous position loaded but not set (no direct setter)");
            }
        }

        return transform;
    }

    // ==================== RigidBody Serializer ====================
    // RigidBodySerializer implementation
    std::string RigidBodySerializer::serialize(Component* component) {

        RigidBody* rigidBody = static_cast<RigidBody*>(component);

        if (!rigidBody) {
            return "{}";
        }
        std::stringstream ss;

        ss << "{\n";
        //ss << "          \"Mass\": \"" << rigidBody->getMass() << "\"\n";
        ss << "          \"Mass\": " << rigidBody->getMass() << ",\n";

        const Vector3D& vel = rigidBody->getVelocity();
        ss << "          \"velocity\": [\n";
        ss << "            " << vel.x << ",\n";
        ss << "            " << vel.y << ",\n";
        ss << "            " << vel.z << "\n";
        ss << "          ],\n";
        //ss << "        }";

        const Vector3D& accel = rigidBody->getAcceleration();
        ss << "          \"acceleration\": [\n";
        ss << "            " << accel.x << ",\n";
        ss << "            " << accel.y << ",\n";
        ss << "            " << accel.z << "\n";
        ss << "          ],\n";


        const Vector3D& angularVel = rigidBody->getAngularVelocity();
        ss << "          \"angularVelocity\": [\n";
        ss << "            " << angularVel.x << ",\n";
        ss << "            " << angularVel.y << ",\n";
        ss << "            " << angularVel.z << "\n";
        ss << "          ],\n";

        const Vector3D& inertiaDiag = rigidBody->getInertiaDiagonal();
        ss << "          \"inertiaDiag\": [\n";
        ss << "            " << inertiaDiag.x << ",\n";
        ss << "            " << inertiaDiag.y << ",\n";
        ss << "            " << inertiaDiag.z << "\n";
        ss << "          ],\n";


        ss << "          \"layer\": " << rigidBody->getLayer() << ",\n";

        ss << "          \"forceMask\": " << ((rigidBody->getForceMask() != 0u) ? "true" : "false") << ",\n";

        ss << "          \"torqueMask\": " << ((rigidBody->getTorqueMask() != 0u) ? "true" : "false") << "\n";


        ss << "        }";


        return ss.str();
    }

    // RigidBodySerializer implementation
    Component* RigidBodySerializer::deserialize(EntityID entityId, const std::string& jsonData) {

        // Parse the rigid body type 
        float mass = 1.0f;
        std::string massData = SerialisationManager::extractObjectValue(jsonData, "mass");
        if (!massData.empty()) mass = std::stof(massData);

        Vector3D velocity = Vector3D::ZERO;
        //std::string velString = "velocity";
        //addComponentVec3D()
        Vector3D velcom = SerialisationManager::addComponentVec3D(velocity, jsonData, "velocity");


        Vector3D acceleration = Vector3D::ZERO;
        Vector3D accelCom = SerialisationManager::addComponentVec3D(acceleration, jsonData, "acceleration");

        Vector3D angularVel = Vector3D::ZERO;
        Vector3D angularVelcom = SerialisationManager::addComponentVec3D(angularVel, jsonData, "angularVelocity");

        Vector3D inertiaDiag = Vector3D::ONE;
        Vector3D inertiaDiagCom = SerialisationManager::addComponentVec3D(inertiaDiag, jsonData, "inertiaDiag");

        int layer = 0;
        std::string layerData = SerialisationManager::extractNumberValue(jsonData, "layer");
        if (!layerData.empty()) {
            layer = std::stoi(layerData);
        }

        unsigned forceMask = 0xFFFFFFFFu;
        std::string applyForcesData = SerialisationManager::extractNumberValue(jsonData, "forceMask");
        if (!applyForcesData.empty()) {
            forceMask = (applyForcesData == "true") ? 0xFFFFFFFFu : 0u;
        }

        unsigned torqueMask = 0xFFFFFFFFu;
        std::string torqueMaskData = SerialisationManager::extractNumberValue(jsonData, "torqueMask");
        if (!torqueMaskData.empty()) {
            torqueMask = (torqueMaskData == "true") ? 0xFFFFFFFFu : 0u;
        }

        RigidBody* rigidBody = EM.addComponent<RigidBody>(entityId, mass, velcom, accelCom, angularVelcom, inertiaDiagCom, layer, forceMask, torqueMask);

        return rigidBody;

    }

    // ==================== Collider Serializer ====================
    std::string ColliderSerializer::serialize(Component* component)
    {
        Collider* collider = static_cast<Collider*>(component);
        if (!collider)
        {
            return "{}";
        }

        std::stringstream  ss;
        ss << "{\n";
        const Vector3D& aabbHalfExtents = collider->getAABBHalfExtents();
        ss << "          \"AABBHalfExtents\": [\n";
        ss << "            " << aabbHalfExtents.x << ",\n";
        ss << "            " << aabbHalfExtents.y << ",\n";
        ss << "            " << aabbHalfExtents.z << "\n";
        ss << "          ],\n";

        const Vector3D& aabbOffset = collider->getAABBOffset();
        ss << "          \"Offset\": [\n";
        ss << "            " << aabbOffset.x << ",\n";
        ss << "            " << aabbOffset.y << ",\n";
        ss << "            " << aabbOffset.z << "\n";
        ss << "          ]\n";

        ss << "        }";
        return ss.str();

    }

    Component* ColliderSerializer::deserialize(EntityID entityId, const std::string& jsonData)
    {
        Vector3D aabbHalfExtents = { 0.5f,0.5f,0.5f };
        Vector3D aabbHEcom = SerialisationManager::addComponentVec3D(aabbHalfExtents, jsonData, "aabbHalfExtents");

        Vector3D offset = { 0.0f,0.0f,0.0f };
        Vector3D offsetCom = SerialisationManager::addComponentVec3D(offset, jsonData, "aabbOffset");

        Collider::AABBData aabbData;
        aabbData.halfExtents = aabbHEcom;
        aabbData.offset = offsetCom;
        Collider* collider = EM.addComponent<Collider>(entityId, aabbData);

        return collider;
    }

    // ==================== Audio Serializer ====================
    //AudioComponentSerializer implementation
    std::string AudioComponentSerializer::serialize(Component* component) {
        AudioComponent* audio = static_cast<AudioComponent*>(component);
        if (!audio) {
            return "{}";
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "{\n";
        ss << "          \"guid\": \"" << audio->getGUID() << "\",\n";
        //ss << "          \"audioID\": " << audio->getAudioID() << ",\n"; // Placeholder for audioID
        ss << "          \"type\": \"" << (audio->getType() == AudioType::BGM ? "BGM" : "SFX") << "\",\n";
        ss << "          \"state\": \""
            << (audio->getPlayState() == PlayState::PLAY ? "PLAY" :
                audio->getPlayState() == PlayState::PAUSE ? "PAUSE" : "STOP")
            << "\",\n";
        ss << "          \"volume\": " << audio->getVolume() << ",\n";
        ss << "          \"pitch\": " << audio->getPitch() << ",\n";
        ss << "          \"loop\": \"" << (audio->isLooping() ? "true" : "false") << "\",\n";
        ss << "          \"mute\": \"" << (audio->isMute() ? "true" : "false") << "\",\n";
        ss << "          \"is3D\": \"" << (audio->is3D() ? "true" : "false") << "\",\n";
        ss << "          \"minDistance\": " << audio->getMinDistance() << ",\n";
        ss << "          \"maxDistance\": " << audio->getMaxDistance() << "\n";
        ss << "        }";
        return ss.str();
    }

    //AudioComponentDeserializer implementation
    Component* AudioComponentSerializer::deserialize(EntityID entityId, const std::string& jsonData) {
        //GUID
        std::string guid = SerialisationManager::extractQuotedValue(jsonData, "guid");

        /*int64_t audioID = -1;
        int64_t audioIDData = SerialisationManager::extractObjectValue(jsonData, "audioID");
        if (!audioIDData.empty()) {
            try {
                audioID = std::stoll(audioIDData);
            }
            catch (const std::exception&) {
                LM.writeLog("AudioComponentSerializer::deserialize() - Fail to parse audioID");
            }
        }*/

        //Type
        std::string typeData = SerialisationManager::extractQuotedValue(jsonData, "type");
        AudioType type = (typeData == "BGM") ? AudioType::BGM : AudioType::SFX;

        //State
        PlayState state = PlayState::STOP;
        std::string stateData = SerialisationManager::extractQuotedValue(jsonData, "state");
        if (!stateData.empty()) {
            if (stateData == "PLAY") {
                state = PlayState::PLAY;
            }
            else if (stateData == "PAUSE") {
                state = PlayState::PAUSE;
            }
        }

        //Volume
        float volume = 1.0f;
        std::string volumeData = SerialisationManager::extractNumberValue(jsonData, "volume");
        if (!volumeData.empty()) {
            try {
                volume = std::stof(volumeData);
            }
            catch (const std::exception&) {
                LM.writeLog("AudioComponentSerializer::deserialize() - Fail to parse volume");
            }
        }

        //Pitch
        float pitch = 1.0f;
        std::string pitchData = SerialisationManager::extractNumberValue(jsonData, "pitch");
        if (!pitchData.empty()) {
            try {
                pitch = std::stof(pitchData);
            }
            catch (const std::exception&) {
                LM.writeLog("AudioComponentSerializer::deserialize() - Fail to parse pitch");
            }
        }

        // Loop
        bool loop = false;
        std::string loopData = SerialisationManager::extractQuotedValue(jsonData, "loop");
        if (!loopData.empty()) {
            loop = (loopData == "true");
        }

        // Mute
        bool mute = false;
        std::string muteData = SerialisationManager::extractQuotedValue(jsonData, "mute");
        if (!muteData.empty()) {
            mute = (muteData == "true");
        }

        // is3D
        bool is3D = true; // default true
        std::string is3DData = SerialisationManager::extractQuotedValue(jsonData, "is3D");
        if (!is3DData.empty()) {
            is3D = (is3DData == "true");
            //LM.writeLog("AudioComponentSerializer::deserialize() - is3D is %s", is3DData.c_str());
        }

        //minDistance
        float minDistance = 1.0f;
        std::string minDistanceData = SerialisationManager::extractNumberValue(jsonData, "minDistance");
        if (!minDistanceData.empty()) {
            try {
                minDistance = std::stof(minDistanceData);
            }
            catch (const std::exception&) {
                LM.writeLog("AudioComponentSerializer::deserialize() - Fail to parse minDistance");
            }
        }

        //maxDistance
        float maxDistance = 100.0f;
        std::string maxDistanceData = SerialisationManager::extractNumberValue(jsonData, "maxDistance");
        if (!maxDistanceData.empty()) {
            try {
                maxDistance = std::stof(maxDistanceData);
            }
            catch (const std::exception&) {
                LM.writeLog("AudioComponentSerializer::deserialize() - Fail to parse maxDistance");
            }
        }
        LM.writeLog("AudioComponentSerializer::deserialize() - Parsed AudioComponent data: guid=%s, type=%s, state=%s, volume=%.2f, pitch=%.2f, loop=%s, mute=%s, is3D=%s, minDistance=%.2f, maxDistance=%.2f",
            guid.c_str(), (type == AudioType::BGM ? "BGM" : "SFX"),
            (state == PlayState::PLAY ? "PLAY" : state == PlayState::PAUSE ? "PAUSE" : "STOP"),
            volume, pitch, loop ? "true" : "false", mute ? "true" : "false", is3D ? "true" : "false", minDistance, maxDistance);

        // Create the Audio_Component
        AudioComponent* audio = EM.addComponent<AudioComponent>(entityId, guid, type, state, volume, pitch, loop, mute, is3D, minDistance, maxDistance);

        // Set the play state
        if (audio) {
            audio->setPlayState(state);
        }
        return audio;
    }

    // ==================== Script Serializer ====================

    std::string ScriptSerializer::serialize(Component* component) {
        Script* script = static_cast<Script*>(component);
        if (!script) {
            return "{}";
        }

        std::stringstream ss;
        ss << "{\n";
        ss << "          \"script_name\": \"" << script->getScriptName() << "\",\n";
        ss << "          \"is_active\": \"" << (script->isActive() ? "true" : "false") << "\"\n";
        ss << "        }";

        return ss.str();
    }

    Component* ScriptSerializer::deserialize(EntityID entityId, const std::string& jsonData) {
        // Extract script_name
        std::string script_name = SerialisationManager::extractQuotedValue(jsonData, "script_name");

        // Extract is_active
        bool is_active = true; // default true
        std::string is_active_data = SerialisationManager::extractQuotedValue(jsonData, "is_active");
        if (!is_active_data.empty()) {
            is_active = (is_active_data == "true");
        }

        LM.writeLog("ScriptSerializer::deserialize() - Parsed Script data: script_name=%s, is_active=%s",
            script_name.c_str(), is_active ? "true" : "false");

        // Create the Script component
        Script* script = EM.addComponent<Script>(entityId, script_name, is_active);

        return script;
    }

    // ==================== MeshComponent Serializer ====================
    std::string MeshComponentSerializer::serialize(Component* component) {

        // Prepare mesh component for serializing
        MeshComponent* mesh_comp = static_cast<MeshComponent*>(component);
        if (!mesh_comp) {
            return "{}";
        }

        // Serializing mesh component data
        std::stringstream ss;
        ss << "{\n";
        ss << "          \"guid\": \"" << mesh_comp->getGUID() << "\",\n";
        ss << "          \"mesh_handle\": " << mesh_comp->getMeshHandle() << ",\n";
        ss << "          \"material_handle\": " << mesh_comp->getMaterialHandle() << "\n";
        ss << "        }";
        return ss.str();
    }

    // MeshComponentSerializer implementation
    Component* MeshComponentSerializer::deserialize(EntityID entityId, const std::string& jsonData) {

        // Retrieve guid
        std::string guid = SerialisationManager::extractQuotedValue(jsonData, "guid");

        // Parse mesh handle
        uint16_t mesh_handle = 0;
        std::string mesh_handle_data = SerialisationManager::extractNumberValue(jsonData, "mesh_handle");
        if (!mesh_handle_data.empty()) {
            try {
                mesh_handle = static_cast<uint16_t>(std::stoi(mesh_handle_data));
            }
            catch (const std::exception&) {
                LM.writeLog("MeshComponentSerializer::deserialize() - Fail to parse mesh handle");
            }
        }

        // Parse material handle
        uint16_t material_handle = 0;
        std::string material_handle_data = SerialisationManager::extractNumberValue(jsonData, "material_handle");
        if (!material_handle_data.empty()) {
            try {
                material_handle = static_cast<uint16_t>(std::stoi(material_handle_data));
            }
            catch (const std::exception&) {
                LM.writeLog("MeshComponentSerializer::deserialize() - Fail to parse material handle");
            }
        }

        // Create and return mesh component
        MeshComponent* mesh_comp = EM.addComponent<MeshComponent>(entityId, guid, mesh_handle, material_handle);
        return mesh_comp;
    }

    // ==================== TextureComponent Serializer ====================
    std::string TextureComponentSerializer::serialize(Component* component ) {
        
        // Prepare texture component for serializing 
        TextureComponent* tex_comp = static_cast<TextureComponent*>(component);
        if (!tex_comp) {
            return "{}";
        }

        // Serializing texture component data
        std::stringstream ss;
        ss << "{\n";
        ss << "          \"guid\": \"" << tex_comp->getGUID() << "\",\n";
        ss << "          \"texture_handle\": " << tex_comp->getTextureHandle() << ",\n";
        ss << "        }";
        return ss.str();
    }

    // TextureComponentSerializer implementation
    Component* TextureComponentSerializer::deserialize(EntityID entityId, const std::string& jsonData) {

        std::string guid = SerialisationManager::extractQuotedValue(jsonData, "guid");

        uint16_t texture_handle = 0;
        std::string texture_handle_data = SerialisationManager::extractNumberValue(jsonData, "texture_handle");
        if (!texture_handle_data.empty()) {
            try {
                texture_handle = static_cast<uint16_t>(std::stoi(texture_handle_data));
            }
            catch (const std::exception&) {
                LM.writeLog("TextureComponentSerializer::deserialize() - Fail to parse texture handle");
            }
        }

        TextureComponent* tex_comp = EM.addComponent<TextureComponent>(entityId, guid, texture_handle);
        return tex_comp;
    }

    // Initialize singleton instance
    SerialisationManager::SerialisationManager() {
        setType("SerialisationManager");
    }

    // Get the singleton instance
    SerialisationManager& SerialisationManager::getInstance() {
        static SerialisationManager instance;
        return instance;
    }

    // Start up the SerialisationManager
    int SerialisationManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // ==============Register Transform3D component serializers ===========================
        registerComponentSerializer("Transform3D", std::make_shared<Transform3DSerializer>());
        // Register component creators
        registerComponentCreator("Transform3D", [this](EntityID entityId, const std::string& componentData) {
            // Use the serializer to create the component
            auto serializer = m_component_serializers["Transform3D"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("Transform3D created for entity %d", entityId);
            }
            });

        // ==============Register Audio component serializers ==========================
        registerComponentSerializer("AudioComponent", std::make_shared<AudioComponentSerializer>());
        registerComponentCreator("AudioComponent", [this](EntityID entityId, const std::string& componentData) {
            //Use the serializer to create the component
            auto serializer = m_component_serializers["AudioComponent"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("Audio_Component created for entity %d", entityId);
            }
            else {
                LM.writeLog("Audio_Component serializer not found for entity %d", entityId);
            }
            });

        // ==============Register Script component serializers ====================
        registerComponentSerializer("Script", std::make_shared<ScriptSerializer>());

        registerComponentCreator("Script", [this](EntityID entityId, const std::string& componentData) {
            auto serializer = m_component_serializers["Script"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("Script component created for entity %d", entityId);
            }
            else {
                LM.writeLog("Script serializer not found for entity %d", entityId);
            }
            });
        

        // ==============Register RigidBody component serializers==========================
        registerComponentSerializer("RigidBody", std::make_shared<RigidBodySerializer>());

        // Register component creators
        registerComponentCreator("RigidBody", [this](EntityID entityId, const std::string& componentData) {

            auto serializer = m_component_serializers["RigidBody"];
            if (serializer)
            {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("RigidBody created for entity %d", entityId);
            }
            });

        // ==============Register Collider component serializers==========================
        // Register component serializers for Collider
        registerComponentSerializer("Collider", std::make_shared<ColliderSerializer>());

        // Register component creators
        registerComponentCreator("Collider", [this](EntityID entityId, const std::string& componentData) {

            auto serializer = m_component_serializers["Collider"];
            if (serializer)
            {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("Collider created for entity %d", entityId);
            }
            });

        // Mesh Component
        registerComponentCreator("MeshComponent", [this](EntityID entityId, const std::string& componentData) {
            //Use the serializer to create the component
            auto serializer = m_component_serializers["MeshComponent"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("Mesh_Component created for entity %d", entityId);
            }
            else {
                LM.writeLog("Mesh_Component serializer not found for entity %d", entityId);
            }
            });

        // ==============Register TextureComponent component serializers==========================
        registerComponentSerializer("TextureComponent", std::make_shared<TextureComponentSerializer>());

        registerComponentCreator("TextureComponent", [this](EntityID entityId, const std::string& componentData) {
            auto serializer = m_component_serializers["TextureComponent"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("TextureComponent created for entity %d", entityId);
            }
            else {
                LM.writeLog("Texture Comonent serializer not found for entity &d", entityId);
            }
            });


        // Log startup
        LM.writeLog("SerialisationManager::startUp() - Serialisation Manager started successfully");

        return 0;
    }

    // Shut down the SerialisationManager
    void SerialisationManager::shutDown() {
        // Log shutdown
        LM.writeLog("SerialisationManager::shutDown() - Shutting down Serialisation Manager");

        // Clear component creators and serializers
        m_component_creators.clear();
        m_component_serializers.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Register a component creator function
    void SerialisationManager::registerComponentCreator(const std::string& componentName, ComponentCreatorFunc creatorFunc) {
        m_component_creators[componentName] = creatorFunc;
        LM.writeLog("SerialisationManager::registerComponentCreator() - Registered creator for '%s'", componentName.c_str());
    }

    // Register a component serializer
    void SerialisationManager::registerComponentSerializer(const std::string& componentName, std::shared_ptr<IComponentSerializer> serializer) {
        m_component_serializers[componentName] = serializer;
        LM.writeLog("SerialisationManager::registerComponentSerializer() - Registered serializer for '%s'", componentName.c_str());
    }

    // Load entities from a scene file
    bool SerialisationManager::loadScene(const std::string& filename) {
        LM.writeLog("SerialisationManager::loadScene() - Loading scene from '%s'", filename.c_str());

        // Read file content
        std::string fileContent;
        if (!parseJsonFile(filename, fileContent)) {
            LM.writeLog("SerialisationManager::loadScene() - Failed to read scene file");
            return false;
        }



        /////////////////////////////////////////////////Amanda Code Version/////////////////////////////////////////////////
        // Parse with RapidJSON instead of string::find
        rapidjson::Document doc;
        if (doc.Parse(fileContent.c_str()).HasParseError()) {
            LM.writeLog("SerialisationManager::loadScene() - JSON parse error: %s",
                rapidjson::GetParseError_En(doc.GetParseError()));
            return false;
        }

        if (!doc.HasMember("objects") || !doc["objects"].IsArray()) {
            LM.writeLog("SerialisationManager::loadScene() - No 'objects' array found in scene");
            return false;
        }

        const auto& objects = doc["objects"];
        int entityCount = 0;

        for (auto& obj : objects.GetArray()) {
            if (!obj.HasMember("name") || !obj["name"].IsString()) continue;

            std::string entityName = obj["name"].GetString();
            Entity& entity = EM.createEntity(entityName);
            LM.writeLog("SerialisationManager::loadScene() - Created entity '%s' (ID %d)",
                entityName.c_str(), entity.get_id());

            if (obj.HasMember("components") && obj["components"].IsObject()) {
                // Dump the "components" JSON back into a string
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                obj["components"].Accept(writer);

                // Now let parseComponents handle all registered types
                parseComponents(entity.get_id(), buffer.GetString());
            }

            entityCount++;
        }

        //// Read file content
        //std::string fileContent;
        //if (!parseJsonFile(filename, fileContent)) {
        //    LM.writeLog("SerialisationManager::loadScene() - Failed to read scene file");
        //    return false; // PROPER ERROR RETURN
        //}

        //////////////////////////////////////////////////////////////////////////////////////////////
        //LM.writeLog("SerialisationManager::loadScene() - File loaded, size: %zu characters", fileContent.length());

        //// Simple approach: Find each occurrence of a named entity
        //size_t searchPos = 0;
        //int entityCount = 0;
        //bool foundAnyEntities = false;

        //while (true) {
        //    // Find next "name" field
        //    size_t namePos = fileContent.find("\"name\"", searchPos);
        //    if (namePos == std::string::npos) {
        //        break; // No more entities
        //    }

        //    // Extract the entity name
        //    size_t colonPos = fileContent.find(':', namePos);
        //    size_t nameStartQuote = fileContent.find('"', colonPos);
        //    size_t nameEndQuote = fileContent.find('"', nameStartQuote + 1);

        //    if (colonPos == std::string::npos || nameStartQuote == std::string::npos || nameEndQuote == std::string::npos) {
        //        LM.writeLog("SerialisationManager::loadScene() - Malformed name field at position %zu", namePos);
        //        searchPos = namePos + 1;
        //        continue;
        //    }

        //    std::string entityName = fileContent.substr(nameStartQuote + 1, nameEndQuote - nameStartQuote - 1);
        //    LM.writeLog("SerialisationManager::loadScene() - Found entity: '%s'", entityName.c_str());

        //    // Create the entity
        //    Entity& entity = EM.createEntity(entityName);
        //    entityCount++;
        //    foundAnyEntities = true;
        //    LM.writeLog("SerialisationManager::loadScene() - Created entity '%s' with ID %d", entityName.c_str(), entity.get_id());

        //    // Look for Transform3D component
        //    size_t transform3DPos = fileContent.find("\"Transform3D\"", namePos);
        //    size_t nextNamePos = fileContent.find("\"name\"", namePos + 1);

        //    // Make sure this Transform3D belongs to this entity (not the next one)
        //    if (transform3DPos != std::string::npos &&
        //        (nextNamePos == std::string::npos || transform3DPos < nextNamePos)) {

        //        LM.writeLog("SerialisationManager::loadScene() - Found Transform3D for entity '%s'", entityName.c_str());

        //        // Extract Transform3D data using simple string search
        //        Vector3D position = extractVector3D(fileContent, transform3DPos, "position");
        //        Vector3D rotation = extractVector3D(fileContent, transform3DPos, "rotation");
        //        Vector3D scale = extractVector3D(fileContent, transform3DPos, "scale");

        //        LM.writeLog("SerialisationManager::loadScene() - Position: (%.1f, %.1f, %.1f)", position.x, position.y, position.z);
        //        LM.writeLog("SerialisationManager::loadScene() - Rotation: (%.1f, %.1f, %.1f)", rotation.x, rotation.y, rotation.z);
        //        LM.writeLog("SerialisationManager::loadScene() - Scale: (%.1f, %.1f, %.1f)", scale.x, scale.y, scale.z);

        //        // Create the Transform3D component
        //        Transform3D* transform = EM.addComponent<Transform3D>(entity.get_id(), position, rotation, scale);
        //        if (transform) {
        //            LM.writeLog("SerialisationManager::loadScene() - Transform3D component created successfully for entity '%s'", entityName.c_str());
        //        }
        //        else {
        //            LM.writeLog("SerialisationManager::loadScene() - Failed to create Transform3D component for entity '%s'", entityName.c_str());
        //        }
        //    }
        //    else {
        //        LM.writeLog("SerialisationManager::loadScene() - No Transform3D component found for entity '%s'", entityName.c_str());
        //    }

        //    // Move search position past this entity
        //    searchPos = namePos + 1;
        //}

        ////  PROPER SUCCESS/FAILURE LOGIC
        //if (!foundAnyEntities) {
        //    LM.writeLog("SerialisationManager::loadScene() - ERROR: No entities found in scene file");
        //    return false; // RETURN FALSE IF NO ENTITIES LOADED
        //}

        //while (true) {
        //    // Find next "name" field
        //    size_t namePos = fileContent.find("\"name\"", searchPos);
        //    if (namePos == std::string::npos) {
        //        break; // No more entities
        //    }

        //    // Extract the entity name
        //    size_t colonPos = fileContent.find(':', namePos);
        //    size_t nameStartQuote = fileContent.find('"', colonPos);
        //    size_t nameEndQuote = fileContent.find('"', nameStartQuote + 1);

        //    if (colonPos == std::string::npos || nameStartQuote == std::string::npos || nameEndQuote == std::string::npos) {
        //        LM.writeLog("SerialisationManager::loadScene() - Malformed name field at position %zu", namePos);
        //        searchPos = namePos + 1;
        //        continue;
        //    }

        //    std::string entityName = fileContent.substr(nameStartQuote + 1, nameEndQuote - nameStartQuote - 1);
        //    LM.writeLog("SerialisationManager::loadScene() - Found entity: '%s'", entityName.c_str());

        //    // Create the entity
        //    Entity& entity = EM.createEntity(entityName);
        //    entityCount++;
        //    foundAnyEntities = true;
        //    LM.writeLog("SerialisationManager::loadScene() - Created entity '%s' with ID %d", entityName.c_str(), entity.get_id());

        //    // Look for Transform3D component
        //    size_t transform3DPos = fileContent.find("\"Transform3D\"", namePos);
        //    size_t nextNamePos = fileContent.find("\"name\"", namePos + 1);

        //    // Make sure this Transform3D belongs to this entity (not the next one)
        //    if (transform3DPos != std::string::npos &&
        //        (nextNamePos == std::string::npos || transform3DPos < nextNamePos)) {

        //        LM.writeLog("SerialisationManager::loadScene() - Found Transform3D for entity '%s'", entityName.c_str());

        //        // Extract Transform3D data using simple string search
        //        Vector3D position = extractVector3D(fileContent, transform3DPos, "position");
        //        Vector3D rotation = extractVector3D(fileContent, transform3DPos, "rotation");
        //        Vector3D scale = extractVector3D(fileContent, transform3DPos, "scale");

        //        LM.writeLog("SerialisationManager::loadScene() - Position: (%.1f, %.1f, %.1f)", position.x, position.y, position.z);
        //        LM.writeLog("SerialisationManager::loadScene() - Rotation: (%.1f, %.1f, %.1f)", rotation.x, rotation.y, rotation.z);
        //        LM.writeLog("SerialisationManager::loadScene() - Scale: (%.1f, %.1f, %.1f)", scale.x, scale.y, scale.z);

        //        // Create the Transform3D component
        //        Transform3D* transform = EM.addComponent<Transform3D>(entity.get_id(), position, rotation, scale);
        //        if (transform) {
        //            LM.writeLog("SerialisationManager::loadScene() - Transform3D component created successfully for entity '%s'", entityName.c_str());
        //        }
        //        else {
        //            LM.writeLog("SerialisationManager::loadScene() - Failed to create Transform3D component for entity '%s'", entityName.c_str());
        //        }

        //        
        //        
        //    }
        //    else {
        //        LM.writeLog("SerialisationManager::loadScene() - No Transform3D component found for entity '%s'", entityName.c_str());
        //    }


        //    // Look for RigidBody compoenents
        //    size_t rigidBodyPos = fileContent.find("\"RigidBody\"", namePos);

        //    if (rigidBodyPos != std::string::npos &&
        //        (nextNamePos == std::string::npos || rigidBodyPos < nextNamePos))
        //    {
        //        LM.writeLog("SerialisationManager::loadScene() - Found RigidBody for entity '%s'", entityName.c_str());
        //        std::string typeStr = SerialisationManager::extractQuotedValue(fileContent, "rigidBodyType");
        //        BodyType rigidBodyType = RigidBody::stringToBodyType(typeStr); // convert string back to enum BodyType
        //        //LM.writeLog("SerialisationManager::loadScene() - rigidBodyType: (%d)", rigidBodyType);
        //        RigidBody* rigidBody = EM.addComponent<RigidBody>(entity.get_id(), rigidBodyType);
        //        if (rigidBody) {
        //            LM.writeLog("SerialisationManager::loadScene() - RigidBody component created successfully for entity '%s'", entityName.c_str());
        //        }
        //        else {
        //            LM.writeLog("SerialisationManager::loadScene() - Failed to create RigidBody component for entity '%s'", entityName.c_str());
        //        }
        //    }
        //    else {
        //        LM.writeLog("SerialisationManager::loadScene() - No RigidBody component found for entity '%s'", entityName.c_str());
        //    }
        //    

        //    // Move search position past this entity
        //    searchPos = namePos + 1;
        //}

        ////  PROPER SUCCESS/FAILURE LOGIC
        //if (!foundAnyEntities) {
        //    LM.writeLog("SerialisationManager::loadScene() - ERROR: No entities found in scene file");
        //    return false; // RETURN FALSE IF NO ENTITIES LOADED
        //}

        //LM.writeLog("SerialisationManager::loadScene() - Scene loaded successfully, processed %d entities", entityCount);
        return true; // ONLY RETURN TRUE IF ENTITIES WERE ACTUALLY LOADED
    }

    // Save current entities to a scene file
    bool SerialisationManager::saveScene(const std::string& filename) {
        LM.writeLog("SerialisationManager::saveScene() - Saving scene to '%s'", filename.c_str());

        // Create the scene file
        std::ofstream file(filename);
        if (!file.is_open()) {
            LM.writeLog("SerialisationManager::saveScene() - Failed to open file for writing");
            return false;
        }

        // Get all entities
        const auto& entities = EM.getAllEntities();

        // Start the JSON structure
        file << "{\n";
        file << getIndent(1) << "\"objects\": [\n";

        // Save each entity
        for (size_t i = 0; i < entities.size(); ++i) {
            const Entity& entity = entities[i];
            bool hasComponents = false;

            // Vector to store all the components 
            std::vector<std::string> componentStrings;

            file << getIndent(2) << "{\n";
            file << getIndent(3) << "\"name\": \"" << entity.get_name() << "\",\n";
            file << getIndent(3) << "\"components\": {\n";

            // =================== Check for Transform3D component ========================
            if (auto serializer = m_component_serializers.find("Transform3D");
                serializer != m_component_serializers.end()) {
                if (Transform3D* transform = EM.getComponent<Transform3D>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"Transform3D\": " +
                        serializer->second->serialize(transform));
                    hasComponents = true;
                }
            }

            // ============================= Check for Audio_Component ==============================
            if (auto serializer = m_component_serializers.find("AudioComponent");
                serializer != m_component_serializers.end()) {
                if (AudioComponent* audio = EM.getComponent<AudioComponent>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"AudioComponent\": " +
                        serializer->second->serialize(audio));
                    hasComponents = true;
                }
            }

            // ======================== Check for Script component ============================
            if (auto serializer = m_component_serializers.find("Script");
                serializer != m_component_serializers.end()) {
                if (Script* script = EM.getComponent<Script>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"Script\": " +
                        serializer->second->serialize(script));
                    hasComponents = true;
                }
            }

            // ==================== Check for RigidBody component ===========================
            if (auto serializer = m_component_serializers.find("RigidBody");
                serializer != m_component_serializers.end()) {
                if (RigidBody* rigidBody = EM.getComponent<RigidBody>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"RigidBody\": " +
                        serializer->second->serialize(rigidBody));
                    hasComponents = true;
                }
            }

            // ===================== Check for Collider component =====================
            if (auto serializer = m_component_serializers.find("Collider");
                serializer != m_component_serializers.end()) {
                if (Collider* collider = EM.getComponent<Collider>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"Collider\": " +
                        serializer->second->serialize(collider));
                    hasComponents = true;
                }
            }

            // Check for MeshComponent 
            if (auto serializer = m_component_serializers.find("MeshComponent");
                serializer != m_component_serializers.end()) {
                if (MeshComponent* mesh = EM.getComponent<MeshComponent>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"MeshComponent\": " +
                        serializer->second->serialize(mesh));
                    hasComponents = true;
                }
            }

            // ===================== Check for Texture component =====================
            if (auto serializer = m_component_serializers.find("TextureComponent");
                serializer != m_component_serializers.end()) {
                if (TextureComponent* tex = EM.getComponent<TextureComponent>(entity.get_id())) {
                    componentStrings.push_back(getIndent(4) + "\"TextureComponent\": " + serializer->second->serialize(tex));
                    hasComponents = true;
                }


            }


            // TODO: Add more component types here as needed

            // Write all components with proper comma separation
            for (size_t j = 0; j < componentStrings.size(); ++j) {
                file << componentStrings[j];
                if (j < componentStrings.size() - 1) {
                    file << ",";
                }
                file << "\n";
            }

            // Close the components object
            file << getIndent(3) << "}\n";

            // Close the entity object
            file << getIndent(2) << "}";

            // Add comma if not the last entity (FIXED: Only one comma logic now)
            if (i < entities.size() - 1) {
                file << ",";
            }
            file << "\n";
        }

        // Close the JSON structure
        file << getIndent(1) << "]\n";
        file << "}\n";

        file.close();

        LM.writeLog("SerialisationManager::saveScene() - Scene saved successfully");
        return true;
    }

    // Get a registered component serializer
    std::shared_ptr<IComponentSerializer> SerialisationManager::getComponentSerializer(const std::string& componentName) {
        auto it = m_component_serializers.find(componentName);
        if (it != m_component_serializers.end()) {
            return it->second;
        }
        return nullptr;
    }

    // ================================= Helper Methods =================================

    // Helper method to parse a JSON file
    bool SerialisationManager::parseJsonFile(const std::string& filename, std::string& jsonContent) {
        // Open the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            LM.writeLog("SerialisationManager::parseJsonFile() - Failed to open file '%s'", filename.c_str());
            return false;
        }

        // Read the entire file into a string
        std::stringstream buffer;
        buffer << file.rdbuf();
        jsonContent = buffer.str();

        // Close the file
        file.close();

        return true;
    }

    // Helper method to parse components and create them for an entity
    bool SerialisationManager::parseComponents(EntityID entityId, const std::string& componentData) {
        // For each registered component type, check if it exists in the data
        for (const auto& pair : m_component_creators) {
            const std::string& componentName = pair.first;
            const ComponentCreatorFunc& creatorFunc = pair.second;

            // Check if this component type exists in the data
            size_t componentPos = componentData.find("\"" + componentName + "\"");
            if (componentPos != std::string::npos) {
                // Extract the component data
                size_t componentBraceStart = componentData.find('{', componentPos);
                if (componentBraceStart != std::string::npos) {
                    // Find the closing brace, accounting for nested braces
                    int braceLevel = 1;
                    size_t componentBraceEnd = componentBraceStart + 1;

                    while (braceLevel > 0 && componentBraceEnd < componentData.length()) {
                        if (componentData[componentBraceEnd] == '{') {
                            braceLevel++;
                        }
                        else if (componentData[componentBraceEnd] == '}') {
                            braceLevel--;
                        }
                        componentBraceEnd++;
                    }

                    if (braceLevel == 0) {
                        componentBraceEnd--; // Move back to the closing brace
                        std::string componentContent = componentData.substr(componentBraceStart, componentBraceEnd - componentBraceStart + 1);

                        // Call the creator function with the component data
                        creatorFunc(entityId, componentContent);
                    }
                }
            }
        }

        return true;
    }


    // Helper function to extract a section from JSON
    std::string SerialisationManager::extractSection(const std::string& json, const std::string& sectionName) {
        size_t pos = json.find(sectionName);
        if (pos == std::string::npos) {
            return ""; // Section not found
        }

        // Find the beginning of the array
        size_t arrayStart = json.find('[', pos);
        if (arrayStart == std::string::npos) {
            return ""; // Array not found
        }

        // Find the end of the array, accounting for nested arrays
        int bracketLevel = 1;
        size_t arrayEnd = arrayStart + 1;

        while (bracketLevel > 0 && arrayEnd < json.length()) {
            if (json[arrayEnd] == '[') {
                bracketLevel++;
            }
            else if (json[arrayEnd] == ']') {
                bracketLevel--;
            }
            arrayEnd++;
        }

        if (bracketLevel != 0) {
            return ""; // Unbalanced brackets
        }

        return json.substr(arrayStart, arrayEnd - arrayStart);
    }

    // Helper function to extract quoted string
    std::string SerialisationManager::extractQuotedValue(const std::string& json, const std::string& fieldName) {
        size_t pos = json.find("\"" + fieldName + "\"");
        if (pos == std::string::npos) {
            return ""; // Field not found
        }

        // Find the colon after the field name
        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) {
            return ""; // Invalid JSON format
        }

        // Find the beginning of the string value
        size_t valueStart = json.find('"', colonPos);
        if (valueStart == std::string::npos) {
            return ""; // Invalid JSON format
        }

        // Find the end of the string value
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) {
            return ""; // Invalid JSON format
        }

        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }

    // Helper function to extract object/array value
    std::string SerialisationManager::extractObjectValue(const std::string& json, const std::string& fieldName) {
        size_t pos = json.find("\"" + fieldName + "\"");
        if (pos == std::string::npos) {
            return ""; // Field not found
        }

        // Find the colon after the field name
        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) {
            return ""; // Invalid JSON format
        }

        // Find the beginning of the array
        size_t arrayStart = json.find('[', colonPos);
        if (arrayStart == std::string::npos) {
            return ""; // Array not found
        }

        // Find the end of the array, accounting for nested arrays
        int bracketLevel = 1;
        size_t arrayEnd = arrayStart + 1;

        while (bracketLevel > 0 && arrayEnd < json.length()) {
            if (json[arrayEnd] == '[') {
                bracketLevel++;
            }
            else if (json[arrayEnd] == ']') {
                bracketLevel--;
            }
            arrayEnd++;
        }

        if (bracketLevel != 0) {
            return ""; // Unbalanced brackets
        }

        return json.substr(arrayStart, arrayEnd - arrayStart);
    }

    // Helper function to parse float array
    std::vector<float> SerialisationManager::parseFloatArray(const std::string& arrayJson) {
        std::vector<float> result;

        // Remove brackets and spaces
        std::string content = arrayJson;
        if (content.front() == '[') content = content.substr(1);
        if (content.back() == ']') content.pop_back();

        // Split by comma and parse floats
        std::stringstream ss(content);
        std::string token;

        while (std::getline(ss, token, ',')) {
            // Remove leading/trailing whitespace
            size_t start = token.find_first_not_of(" \t\n\r");
            size_t end = token.find_last_not_of(" \t\n\r");

            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
                try {
                    result.push_back(std::stof(token));
                }
                catch (const std::exception&) {
                    // Skip invalid numbers
                }
            }
        }

        return result;
    }

    // Split a JSON array into individual objects
    std::vector<std::string> SerialisationManager::splitJsonArray(const std::string& jsonArray) {
        std::vector<std::string> result;

        // Find each object in the array
        size_t pos = 0;
        while (pos < jsonArray.length()) {
            // Find the start of an object
            size_t objectStart = jsonArray.find('{', pos);
            if (objectStart == std::string::npos) {
                break; // No more objects
            }

            // Find the end of the object, accounting for nested objects
            int braceLevel = 1;
            size_t objectEnd = objectStart + 1;

            while (braceLevel > 0 && objectEnd < jsonArray.length()) {
                if (jsonArray[objectEnd] == '{') {
                    braceLevel++;
                }
                else if (jsonArray[objectEnd] == '}') {
                    braceLevel--;
                }
                objectEnd++;
            }

            if (braceLevel != 0) {
                break; // Unbalanced braces
            }

            // Extract the object and add it to the result
            std::string object = jsonArray.substr(objectStart, objectEnd - objectStart);
            result.push_back(object);

            // Move to the next position
            pos = objectEnd;
        }

        return result;
    }

    // Helper for JSON indentation
    std::string SerialisationManager::getIndent(int level) const {
        std::string indent;
        for (int i = 0; i < level; ++i) {
            indent += "  "; // Two spaces per level
        }
        return indent;
    }

    Vector3D SerialisationManager::extractVector3D(const std::string& json, size_t startPos, const std::string& fieldName) {
        // Find the field
        size_t fieldPos = json.find("\"" + fieldName + "\"", startPos);
        if (fieldPos == std::string::npos) {
            LM.writeLog("SerialisationManager::extractVector3D() - Field '%s' not found", fieldName.c_str());
            return Vector3D::ZERO;
        }

        // Find the opening bracket
        size_t bracketStart = json.find('[', fieldPos);
        if (bracketStart == std::string::npos) {
            return Vector3D::ZERO;
        }

        // Find the closing bracket
        size_t bracketEnd = json.find(']', bracketStart);
        if (bracketEnd == std::string::npos) {
            return Vector3D::ZERO;
        }

        // Extract array content
        std::string arrayContent = json.substr(bracketStart + 1, bracketEnd - bracketStart - 1);

        // Parse the three float values
        std::vector<float> values;
        std::stringstream ss(arrayContent);
        std::string token;

        while (std::getline(ss, token, ',')) {
            // Remove whitespace
            size_t start = token.find_first_not_of(" \t\n\r");
            size_t end = token.find_last_not_of(" \t\n\r");

            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
                try {
                    values.push_back(std::stof(token));
                }
                catch (const std::exception&) {
                    LM.writeLog("SerialisationManager::extractVector3D() - Failed to parse float: '%s'", token.c_str());
                    values.push_back(0.0f);
                }
            }
        }

        // Ensure we have at least 3 values
        while (values.size() < 3) {
            values.push_back(0.0f);
        }

        return Vector3D(values[0], values[1], values[2]);
    }

    std::string SerialisationManager::extractNumberValue(const std::string& json, const std::string& fieldName) {
        size_t pos = json.find("\"" + fieldName + "\"");
        if (pos == std::string::npos) {
            return "";
        }

        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) {
            return "";
        }

        // Move past colon
        size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
        if (valueStart == std::string::npos) {
            return "";
        }

        size_t valueEnd = json.find_first_of(",}\n\r", valueStart);
        if (valueEnd == std::string::npos) valueEnd = json.length();

        return json.substr(valueStart, valueEnd - valueStart);
    }
    Vector3D SerialisationManager::addComponentVec3D(Vector3D val, const std::string& jsonData, const std::string valName)
    {
        std::string valData = SerialisationManager::extractObjectValue(jsonData, valName);
        if (!valData.empty())
        {
            std::vector<float> valArray = SerialisationManager::parseFloatArray(valData);
            if (valArray.size() >= 3)
            {
                val = Vector3D(valArray[0], valArray[1], valArray[2]);
            }

        }

        return val;

    }

} // end of namespace gam300