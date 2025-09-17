# ECS Entity Management & Lookup Guide

## Table of Contents

1. [Creating Entities](#creating-entities)
2. [Managing Components](#managing-components)
3. [Entity Lookup Methods](#entity-lookup-methods)
4. [Creating New Components](#creating-new-components)
5. [Best Practices](#best-practices)
6. [Common Use Cases](#common-use-cases)

---

## 1. Creating Entities

### Basic Entity Creation

// Create an unnamed entity
Entity& player = EM.createEntity();

// Create a named entity
Entity& enemy = EM.createEntity("Enemy_Goblin");
Entity& weapon = EM.createEntity("Sword_of_Power");

---

## 2. Managing Components

### Adding Components (Not using serialisation)

// Add Transform3D component with default values
Transform3D* transform = EM.addComponent<Transform3D>(entity_id);

// Add Transform3D with specific values
Vector3D position(10.0f, 0.0f, 5.0f);
Vector3D rotation(0.0f, 45.0f, 0.0f);
Vector3D scale(2.0f, 2.0f, 2.0f);
Transform3D* transform = EM.addComponent<Transform3D>(entity_id, position, rotation, scale);

// Add multiple components to one entity
EntityID player_id = EM.createEntity("Player").get_id();
EM.addComponent<Transform3D>(player_id);
EM.addComponent<Health>(player_id, 100); // Health component with 100 HP
EM.addComponent<Renderer>(player_id, "player_model.obj");

### Removing Components

// Remove a specific component
EM.removeComponent<Transform3D>(entity_id);

// Remove multiple components
EM.removeComponent<Health>(entity_id);
EM.removeComponent<Renderer>(entity_id);

### Checking for Components

// Check single component
if (EM.hasComponent<Transform3D>(entity_id)) {
    // Entity has Transform3D
}

// Check multiple components
if (EM.hasAllComponents<Transform3D, Health, Renderer>(entity_id)) {
    // Entity has all three components
}

### Getting Components

// Get a component (returns nullptr if not found)
Transform3D* transform = EM.getComponent<Transform3D>(entity_id);
if (transform) {
    Vector3D pos = transform->getPosition();
    // Use the component
}

---

## 3. Creating New Components

Adding a new component to the ECS involves 4 main steps: creating the component class, implementing serialization, registering with managers, and testing.

### Step 1: Create the Component Class

Create a new header file (e.g., `Health.h`) in the `Component` folder:
Create the implementation file (`Health.cpp`):

### Step 2: Create the Serializer

### Create the serializer class (add to `SerialisationManager.h`):

class HealthSerializer : public IComponentSerializer {
public:
    std::string serialize(Component* component) override;
    Component* deserialize(EntityID entityId, const std::string& jsonData) override;
};

### Implement the serializer (add to `SerialisationManager.cpp`):

// HealthSerializer implementation
std::string HealthSerializer::serialize(Component* component) {
    Health* health = static_cast<Health*>(component);
    if (!health) {
        return "{}";
    }

    std::stringstream ss;
    ss << "{\n";
    ss << "          \"max_health\": " << health->getMaxHealth() << ",\n";
    ss << "          \"current_health\": " << health->getCurrentHealth() << ",\n";
    ss << "          \"is_invulnerable\": " << (health->isInvulnerable() ? "true" : "false") << "\n";
    ss << "        }";

    return ss.str();
}

Component* HealthSerializer::deserialize(EntityID entityId, const std::string& jsonData) {
    // Parse max_health
    float max_health = 100.0f;
    std::string maxHealthStr = SerialisationManager::extractQuotedValue(jsonData, "max_health");
    if (!maxHealthStr.empty()) {
        try {
            max_health = std::stof(maxHealthStr);
        } catch (const std::exception&) {
            LM.writeLog("HealthSerializer::deserialize() - Failed to parse max_health");
        }
    }

    // Parse current_health
    float current_health = max_health;
    std::string currentHealthStr = SerialisationManager::extractQuotedValue(jsonData, "current_health");
    if (!currentHealthStr.empty()) {
        try {
            current_health = std::stof(currentHealthStr);
        } catch (const std::exception&) {
            LM.writeLog("HealthSerializer::deserialize() - Failed to parse current_health");
        }
    }

    // Parse invulnerability
    bool is_invulnerable = false;
    std::string invulnStr = SerialisationManager::extractQuotedValue(jsonData, "is_invulnerable");
    if (invulnStr == "true") {
        is_invulnerable = true;
    }

    // Create the Health component
    Health* health = EM.addComponent<Health>(entityId, max_health, is_invulnerable);
    
    // Set current health if different from max
    if (current_health != max_health && health) {
        health->heal(current_health - health->getCurrentHealth());
    }

    return health;
}

### Step 3: Register with Managers

### Update the component registration in `GameManager::startUp()` or `SerialisationManager::startUp()`:

int SerialisationManager::startUp() {
    // ... existing code ...

    // Register component serializers
    registerComponentSerializer("Transform3D", std::make_shared<Transform3DSerializer>());
    registerComponentSerializer("Health", std::make_shared<HealthSerializer>());  // Add this line

    // Register component creators
    registerComponentCreator("Transform3D", [this](EntityID entityId, const std::string& componentData) {
        // ... existing Transform3D creator ...
    });

    // Add Health component creator
    registerComponentCreator("Health", [this](EntityID entityId, const std::string& componentData) {
        auto serializer = m_component_serializers["Health"];
        if (serializer) {
            serializer->deserialize(entityId, componentData);
            LM.writeLog("Health created for entity %d", entityId);
        }
    });

    // ... rest of startup code ...
    return 0;
}

### Also register the component type with ComponentManager in `GameManager::startUp()`:

int GameManager::startUp() {
    // ... existing code ...

    // Register components with the ComponentManager
    CM.register_component<Transform3D>();
    CM.register_component<Health>();  // Add this line

    // ... rest of startup code ...
}

### Step 4: Update Serialization Save Logic (For debugging)

### Update the `saveScene` method in `SerialisationManager.cpp` to handle the new component:

bool SerialisationManager::saveScene(const std::string& filename) {
    // ... existing code ...

    for (size_t i = 0; i < entities.size(); ++i) {
        const Entity& entity = entities[i];

        file << getIndent(2) << "{\n";
        file << getIndent(3) << "\"name\": \"" << entity.get_name() << "\",\n";
        file << getIndent(3) << "\"components\": {\n";

        bool hasComponents = false;
        std::vector<std::string> componentStrings;

        // Check for Transform3D component
        if (auto serializer = m_component_serializers.find("Transform3D");
            serializer != m_component_serializers.end()) {
            if (Transform3D* transform = EM.getComponent<Transform3D>(entity.get_id())) {
                componentStrings.push_back(getIndent(4) + "\"Transform3D\": " + 
                                         serializer->second->serialize(transform));
                hasComponents = true;
            }
        }

        // Check for Health component (ADD THIS BLOCK)
        if (auto serializer = m_component_serializers.find("Health");
            serializer != m_component_serializers.end()) {
            if (Health* health = EM.getComponent<Health>(entity.get_id())) {
                componentStrings.push_back(getIndent(4) + "\"Health\": " + 
                                         serializer->second->serialize(health));
                hasComponents = true;
            }
        }

        // Write all components with proper comma separation
        for (size_t j = 0; j < componentStrings.size(); ++j) {
            file << componentStrings[j];
            if (j < componentStrings.size() - 1) {
                file << ",";
            }
            file << "\n";
        }

        // ... rest of save logic ...
    }
}

### Step 5: Include Headers

Don't forget to add the necessary includes:

In `SerialisationManager.h`:

#include "../Component/Health.h"  // Add this

In `GameManager.cpp`:

#include "../Component/Health.h"  // Add this

### Component Creation Checklist

- [ ] Create component header file (`Component/YourComponent.h`)
- [ ] Create component implementation file (`Component/YourComponent.cpp`)
- [ ] Inherit from `Component` base class
- [ ] Implement `init()` and `update()` methods
- [ ] Create serializer class in `SerialisationManager.h`
- [ ] Implement `serialize()` and `deserialize()` methods
- [ ] Register component type with `ComponentManager`
- [ ] Register serializer with `SerialisationManager`
- [ ] Add component creator callback
- [ ] Update `saveScene()` method to handle new component
- [ ] Add necessary `#include` statements
- [ ] Test component creation, serialization, and loading

---

### 4. Best Practices

### Component Management

// ✅ Check before using components
Transform3D* transform = EM.getComponent<Transform3D>(entity_id);
if (transform) {
    transform->setPosition(new_position);
}

// ✅ Use hasComponent for conditional logic
if (EM.hasComponent<Health>(entity_id)) {
    // Only damage entities that can take damage
    Health* health = EM.getComponent<Health>(entity_id);
    health->takeDamage(10);
}

### Bulk Operations

// ✅ Process multiple entities efficiently
std::vector<EntityID> enemies = EM.getEntitiesWithComponent<Enemy>();
for (EntityID enemy_id : enemies) {
    Transform3D* transform = EM.getComponent<Transform3D>(enemy_id);
    Enemy* enemy = EM.getComponent<Enemy>(enemy_id);
    if (transform && enemy) {
        // Update enemy AI, movement, etc.
    }
}

### Error Handling

// ✅ Always check for valid entities and components
Entity* entity = EM.getEntityByName("Player");
if (!entity) {
    LM.writeLog("Error: Player entity not found!");
    return;
}

Transform3D* transform = EM.getComponent<Transform3D>(entity->get_id());
if (!transform) {
    LM.writeLog("Error: Player has no Transform3D component!");
    return;
}

### Quick Reference

### Entity Operations

- `EM.createEntity(name)` - Create new entit
- `EM.destroyEntity(id)` - Remove entity
- `EM.getEntity(id)` - Get entity by ID
- `EM.getEntityByName(name)` - Get entity by name

### Component Operations

- `EM.addComponent<T>(id, args...)` - Add component
- `EM.removeComponent<T>(id)` - Remove component
- `EM.getComponent<T>(id)` - Get component
- `EM.hasComponent<T>(id)` - Check single component
- `EM.hasAllComponents<T1, T2...>(id)` - Check multiple components

### Search Operations

- `EM.getEntitiesWithComponent<T>()` - Find by single component
- `EM.getEntitiesWithComponents<T1, T2...>()` - Find by multiple components
- `EM.getFirstEntityWithComponent<T>()` - Find first with component
- `EM.countEntitiesWithComponent<T>()` - Count entities with component

---
