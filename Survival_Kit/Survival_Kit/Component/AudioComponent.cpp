/**
 * @file AudioComponent.cpp
 * @brief Implementation of the AudioComponent for the Entity Component System.
 * @details Contains implementations for all member functions declared in Audio_Component.h.
 * @author Amanda Leow Boon Suan (100%)
 * @date   21/9/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Component/AudioComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {

	// Constructor
	AudioComponent::AudioComponent(const std::string& guid,
		//int64_t audioID = -1,
		AudioType type, float volume, float pitch, bool loop, PlayState playstate, bool is3D, Vector3D position)
		: m_guid(guid), 
		//m_audioID(audioID),
		m_type(type), m_volume(volume), m_pitch(pitch), m_loop(loop), m_playState(PlayState::STOP), m_is3D(is3D), m_position(position) {
	}

	// Initialize the component
	void AudioComponent::init(EntityID entity_id) {
		m_owner_id = entity_id;
		LM.writeLog("AudioComponent::init() - AudioComponent initialized for entity %d", entity_id);
	}
	// Update the component
	void AudioComponent::update(float dt) {
		// Audio_Component doesn't need to do much in update - it's primarily a data container
		// Audio systems will read and modify this data
		// Mark parameter as unused to avoid compiler warning
		(void)dt;
	}
}