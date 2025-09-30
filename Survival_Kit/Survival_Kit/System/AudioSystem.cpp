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

#include "../System/AudioSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ECSManager.h"
#include "../Manager/LogManager.h"
#include "../Utility/AssetPath.h"
namespace gam300 {

	AudioSystem::AudioSystem() : ComponentSystem<AudioComponent, Transform3D>("AudioSystem") {
		set_priority(150); //set priority above graphics but above others
	}

	AudioSystem::~AudioSystem() {
		shutdown();
	}

	bool AudioSystem::init(SystemManager& /*system_manager*/) {

		if(FMOD::System_Create(&m_coresystem) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - FMOD::System_Create failed");
			return false;
		}

		if(m_coresystem->init(512, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - m_coresystem->init failed");
			return false;
		}

		if(m_coresystem->getMasterChannelGroup(&m_mastergroup) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - m_coresystem->getMasterChannelGroup failed");
			return false;
		}

		if (m_coresystem->createChannelGroup("SFXGroup", &m_sfxgroup) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - m_coresystem->createChannelGroup SFXGroup failed");
			return false;
		}

		if (m_coresystem->createChannelGroup("BGMGroup", &m_bgmgroup) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - m_coresystem->createChannelGroup BGMGroup failed");
			return false;
		}

		LM.writeLog("AudioSystem::init() - Audio System Initialized");
		return true;
	}

	void AudioSystem::update(float dt) {
		(void)dt;

		//LM.writeLog("AudioSystem::update() - Updating Audio System");

		//if (IM.isKeyJustReleased(GLFW_KEY_P)) {
		//	LM.writeLog("AudioSystem::update() - Play sound on Cube release");
		//	Entity* retrievecube = EM.getEntityByName("Cube");
		//	if (retrievecube) {
		//		if(retrievecube->has_component(get_component_type_id<AudioComponent>())) {
		//			AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
		//			if (audio) {
		//				audio->setPlayState(PlayState::PLAY);
		//			}
		//		}
		//	}
		//}

		//if (IM.isKeyJustPressed(GLFW_KEY_P)) {
		//	LM.writeLog("AudioSystem::update() - Play sound on Cube just press");
		//	Entity* retrievecube = EM.getEntityByName("Cube");
		//	if (retrievecube) {
		//		if (retrievecube->has_component(get_component_type_id<AudioComponent>())) {
		//			AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
		//			if (audio) {
		//				audio->setPlayState(PlayState::PLAY);
		//			}
		//		}
		//	}
		//}

		if (IM.isKeyPressed(GLFW_KEY_P)) {
			LM.writeLog("AudioSystem::update() - Play sound on Cube pressed");
			/*Entity* retrievecube = EM.getEntityByName("Cube");
			if (retrievecube) {
				if (retrievecube->has_component(get_component_type_id<AudioComponent>())) {
					AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
					if (audio) {
						audio->setPlayState(PlayState::PLAY);
					}
				}
			}*/

			playeditor("\\Audio\\laserSmall_001.ogg");
		}

		//Iterate through all entities with AudioComponent
		auto entities = EM.getEntitiesWithComponent<AudioComponent>();
		for (EntityID id : entities) {
			
			process_entity(id);

		}

		cleanupInactiveChannels();
		updateVolumes();


		if (m_coresystem) {
			m_coresystem->update();
		}
	}

	void AudioSystem::shutdown() {

		// Stop and release all active channels
		for(auto& pair : m_activechannels) {
			if (pair.second) {
				pair.second->stop();
			}
		}

		m_activechannels.clear();

		//Release all loaded sounds
		for (auto& pair : m_loadedsounds) {
			if (pair.second) {
				pair.second->release();
			}
		}

		m_loadedsounds.clear();

		for (auto& pair : m_editorchannel) {
			if(pair.second) {
				pair.second->stop();
			}
		}

		m_previousguids.clear();

		if (m_sfxgroup) {
			m_sfxgroup->release();
			m_sfxgroup = nullptr;
		}

		if (m_bgmgroup) {
			m_bgmgroup->release();
			m_bgmgroup = nullptr;
		}

		if(m_mastergroup) {
			//m_mastergroup->release();
			m_mastergroup = nullptr;
		}

		if (m_coresystem) {
			m_coresystem->release();
			m_coresystem = nullptr;
		}

		LM.writeLog("AudioSystem::shutdown() - Audio System shut down");
	}

	void AudioSystem::process_entity(EntityID entity_id) {
		AudioComponent* audio = EM.getComponent<AudioComponent>(entity_id);
		Transform3D* transform = EM.getComponent<Transform3D>(entity_id);

		if (!audio) {
			return;
		}

		switch (audio->getPlayState()) {
			case PlayState::PLAY:
				playSound(entity_id, audio);
				break;
			case PlayState::PAUSE:
				pauseSound(entity_id, true);
				break;
			case PlayState::STOP:
				stopSound(entity_id);
				break;
		}

		if(audio->is3D() && transform) {
			update3DAttributes(entity_id, audio, transform);
		}
	}

	//Core helpers
	void AudioSystem::playSound(EntityID id, AudioComponent* audio) {
		if (!audio || audio->getGUID().empty()) {
			return;
		}

		auto it = m_loadedsounds.find(audio->getGUID());

		//If sound not loaded, try to load it
		if (it == m_loadedsounds.end()) {
			if (!loadSoundTemp(audio->getGUID(), audio->isLooping())) {
				return;
			}
			it = m_loadedsounds.find(audio->getGUID());
		}

		LM.writeLog("AudioSystem::playSound() - sound is loaded");

		FMOD::Sound* sound = it->second;
		if (!sound) {
			return;
		}

		auto channel_prev = m_previousguids.find(id);
		if (channel_prev != m_previousguids.end() && channel_prev->second != audio->getGUID()) {
			// New sound or different sound, stop previous if any
			stopSound(id);
			m_previousguids[id] = audio->getGUID();
		}

		m_previousguids[id] = audio->getGUID();

		//check if it has been played b4 / recorded in the active channels map
		auto channel_it = m_activechannels.find(id);
		if(channel_it != m_activechannels.end() && channel_it->second) {
			bool is_playing = false;
			channel_it->second->isPlaying(&is_playing);
			bool is_paused = false;
			channel_it->second->getPaused(&is_paused);
			if (is_playing && !is_paused) {
				// Already playing, do not restart
				LM.writeLog("AudioSystem::playSound() - Sound %s on entity %u is already playing", audio->getGUID().c_str(), id);
				return;
			} else {
				// Channel is not playing, remove it from active channels
				m_activechannels.erase(channel_it);
			}
		}

		FMOD::Channel* channel = nullptr;
		FMOD::ChannelGroup* group = nullptr;

		if(audio->getType() == AudioType::SFX) {
			group = m_sfxgroup;
		} else if (audio->getType() == AudioType::BGM) {
			// Handle BGM group if needed
			group = m_bgmgroup;
			//playEvent(id, audio->getGUID());
			return;
		}

		if(m_coresystem->playSound(sound, group, false, &channel) == FMOD_OK) {
			if (channel) {
				channel->setVolume(audio->getVolume());
				channel->setPitch(audio->getPitch());
				m_activechannels[id] = channel;
				LM.writeLog("AudioSystem::playSound() - Playing sound %s on entity %u", audio->getGUID().c_str(), id);
			}
		}

	}

	void AudioSystem::stopSound(EntityID id) {
		auto it = m_activechannels.find(id);
		if(it != m_activechannels.end() && it->second) {
			it->second->stop();
			m_activechannels.erase(it);
			LM.writeLog("AudioSystem::stopSound() - Stopped sound on entity %u", id);
		}
	}

	void AudioSystem::pauseSound(EntityID id, bool pause) {
		auto it = m_activechannels.find(id);
		if(it != m_activechannels.end() && it->second) {
			it->second->setPaused(pause);
			LM.writeLog("AudioSystem::pauseSound() - %s sound on entity %u", pause ? "Paused" : "Resumed", id);
		}
	}

	bool AudioSystem::loadSoundTemp(const std::string& path, bool loop) {
		if (!m_coresystem) {
			return false;
		}

		if (m_loadedsounds.find(path) != m_loadedsounds.end()) {
			return true; // already loaded
		}

		FMOD_MODE mode = FMOD_DEFAULT;
		if (loop) {
			mode |= FMOD_LOOP_NORMAL;
		}

		FMOD::Sound* sound = nullptr;
		if (m_coresystem->createSound(getAssetFilePath(path).c_str(), mode, nullptr, &sound) != FMOD_OK) {
			LM.writeLog("AudioSystem::loadSoundTemp() - Failed to load %s", path.c_str());
			return false;
		}

		m_loadedsounds[path] = sound;
		LM.writeLog("AudioSystem::loadSoundTemp() - Loaded %s", path.c_str());
		return true;
	}

	void AudioSystem::unloadSound(const std::string& path) {
		auto it = m_loadedsounds.find(path);
		if (it != m_loadedsounds.end() && it->second) {
			it->second->release();
			m_loadedsounds.erase(it);
			LM.writeLog("AudioSystem::unloadSound() - Unloaded sound %s", path.c_str());
		}
	}

	void AudioSystem::setListenerAttributes(const Vector3D& position, const Vector3D& forward, const Vector3D& up, const Vector3D& velocity) {
		if (!m_coresystem) {
			return;
		}

		FMOD_VECTOR fmod_position = { position.x, position.y, position.z };
		FMOD_VECTOR fmod_forward = { forward.x, forward.y, forward.z };
		FMOD_VECTOR fmod_up = { up.x, up.y, up.z };
		FMOD_VECTOR fmod_velocity = { velocity.x, velocity.y, velocity.z };

		m_coresystem->set3DListenerAttributes(0, &fmod_position, &fmod_velocity, &fmod_forward, &fmod_up);
	}

	void AudioSystem::setMasterVolume(float volume) {
		m_mastervolume = volume;
		updateVolumes();
	}

	void AudioSystem::setSFXGroupVolume(float volume) {
		m_sfxgroupvolume = volume;
		updateVolumes();
	}

	void AudioSystem::getMasterVolume(float& volume) const {
		volume = m_mastervolume;
	}

	void AudioSystem::getSFXGroupVolume(float& volume) const {
		volume = m_sfxgroupvolume;
	}

	void AudioSystem::cleanupInactiveChannels() {

		std::vector<EntityID> to_remove;

		for (const auto& pair : m_activechannels) {
			bool is_playing = false;
			bool is_paused = false;

			pair.second->getPaused(&is_paused);
			pair.second->isPlaying(&is_playing);

			if(!is_playing && !is_paused) {
				to_remove.push_back(pair.first);
			}

			AudioComponent* audio = EM.getComponent<AudioComponent>(pair.first);
			if (audio && audio->getPlayState() != PlayState::STOP) {
				audio->setPlayState(PlayState::STOP);
			}
		}

		std::vector<std::string> editorguids_to_remove;
		for (const auto& pair: m_editorchannel) {
			bool is_playing = false;
			bool is_paused = false;

			pair.second->getPaused(&is_paused);
			pair.second->isPlaying(&is_playing);

			if (!is_playing && !is_paused) {
				editorguids_to_remove.push_back(pair.first);
			}
		}


		for (EntityID id : to_remove) {
			m_activechannels.erase(id);
			LM.writeLog("AudioSystem::cleanupInactiveChannels() - Removed inactive channel for entity %u", id);
		}
	}

	void AudioSystem::updateVolumes() {
		if (m_sfxgroup) {
			float final_volume = m_mastervolume * m_sfxgroupvolume;
			m_sfxgroup->setVolume(final_volume);
		}
	}

	void AudioSystem::update3DAttributes(EntityID id, AudioComponent* audio, Transform3D* transform) {
		if (!m_coresystem || !audio || !transform) {
			return;
		}
		auto it = m_activechannels.find(id);
		if (it != m_activechannels.end() && it->second) {
			FMOD_VECTOR fmod_position = { transform->getPosition().x, transform->getPosition().y, transform->getPosition().z };
			FMOD_VECTOR fmod_velocity = { 0.0f, 0.0f, 0.0f }; // Velocity can be calculated if needed
			it->second->set3DAttributes(&fmod_position, &fmod_velocity);
		}
	}

	void AudioSystem::playeditor(const std::string& path) {
		if (path.empty()) {
			return;
		}

		auto it = m_loadedsounds.find(path);

		//If sound not loaded, try to load it
		if (it == m_loadedsounds.end()) {
			if (!loadSoundTemp(path, false)) {
				return;
			}
			it = m_loadedsounds.find(path);
		}

		LM.writeLog("AudioSystem::playSound() - sound is loaded");

		FMOD::Sound* sound = it->second;
		if (!sound) {
			return;
		}

		//check if it has been played b4 / recorded in the active channels map
		auto channel_it = m_editorchannel.find(path);
		if (channel_it != m_editorchannel.end() && channel_it->second) {
			bool is_playing = false;
			channel_it->second->isPlaying(&is_playing);
			bool is_paused = false;
			channel_it->second->getPaused(&is_paused);
			if (is_playing && !is_paused) {
				// Already playing, do not restart
				LM.writeLog("AudioSystem::playSound() - Sound %s is already playing", path);
				return;
			}
			else {
				// Channel is not playing, remove it from active channels
				m_editorchannel.erase(channel_it);
			}
		}

		FMOD::Channel* channel = nullptr;
		FMOD::ChannelGroup* group = m_sfxgroup;

		if (m_coresystem->playSound(sound, group, false, &channel) == FMOD_OK) {
			if (channel) {
				channel->setVolume(1.0);
				channel->setPitch(1.0);
				m_editorchannel[path] = channel;
				LM.writeLog("AudioSystem::playSound() - Playing sound %s on enditor", path);
			}
		}
	}
	void AudioSystem::stopeditor(const std::string& path) {
		auto it = m_editorchannel.find(path);
		if (it != m_editorchannel.end() && it->second) {
			it->second->stop();
			m_editorchannel.erase(it);
			LM.writeLog("AudioSystem::stopSound() - Stopped sound %s on editor", path);
		}
	}

}