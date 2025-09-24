/**
 * @file AudioSystem.h
 * @brief RAII Wrappers for GPU resources (OpenGL based).
 * @details Contains the prototype of RAII wrappers for GPU resources like vertex array objects and buffer objects.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __AUDIO_SYSTEM_H__
#define __AUDIO_SYSTEM_H__

#include "../System/System.h"
#include "../Component/AudioComponent.h"
#include "../Component/Transform3D.h"

#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <fmod_errors.h>

#include <unordered_map>
#include <string>

namespace gam300 {

	class AudioSystem : public ComponentSystem<AudioComponent, Transform3D> {
	public:
		AudioSystem();
		~AudioSystem();

		bool init(SystemManager& system_manager) override;
		void update(float dt) override;
		void shutdown() override;

		void process_entity(EntityID entity_id) override;

		//Core helpers
		void playSound(EntityID id, AudioComponent* audio);
		void stopSound(EntityID id);
		void pauseSound(EntityID id, bool pause);

		bool playEvent(EntityID id, const std::string& eventpath);
		bool stopEvent(EntityID id, bool immediate = true);
		bool pauseEvent(EntityID id, bool pause);

		//Temporary loading functions
		bool loadBankTemp(const std::string& path);
		bool loadSoundTemp(const std::string& path, bool loop = false);

		void unloadBank(const std::string& path);
		void unloadSound(const std::string& path);

		void setListenerAttributes(const Vector3D& position, const Vector3D& forward, const Vector3D& up, const Vector3D& velocity);

		void setMasterVolume(float volume);
		void setBusVolume(const std::string& buspath, float volume);
		void setSFXGroupVolume(float volume);

		void getMasterVolume(float& volume) const;
		void getBusVolume(const std::string& buspath, float& volume) const;
		void getSFXGroupVolume(float& volume) const;

		//bool isChannelVirtual(EntityID id); //not sure if need
		//bool isEventValid(const std::string& eventpath); //not sure if need

	private:
		void cleanupInactiveChannels();
		void cleanupInactiveEvents();

		void updateVolumes();

		void update3DAttributes(EntityID id, AudioComponent* audio, Transform3D* transform);

		//void errorCheck(FMOD_RESULT result);

		FMOD::System* m_coresystem = nullptr;
		FMOD::Studio::System* m_studiosystem = nullptr;
		FMOD::Studio::Bank* m_masterbank = nullptr;

		//A map from GUID -> FMOD::Sound*
		std::unordered_map<std::string, FMOD::Sound*> m_loadedsounds;
		std::unordered_map<EntityID, FMOD::Channel*> m_activechannels;

		std::unordered_map<std::string, FMOD::Studio::Bank*> m_loadedbanks;
		std::unordered_map<EntityID, FMOD::Studio::EventInstance*> m_activeevents;
		std::unordered_map<std::string, FMOD::Studio::EventDescription*> m_eventdes;
		std::unordered_map<std::string, FMOD::Studio::Bus*> m_buses;

		FMOD::ChannelGroup* m_sfxgroup = nullptr;

		float m_mastervolume = 1.0f;
		float m_bgmvolume = 1.0f;
		float m_sfxgroupvolume = 1.0f;

		
	};

}

#endif // __AUDIO_SYSTEM_H__