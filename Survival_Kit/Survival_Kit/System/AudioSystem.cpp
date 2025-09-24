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

namespace gam300 {

	AudioSystem::AudioSystem() : ComponentSystem<AudioComponent, Transform3D>("AudioSystem") {
		set_priority(150); //set priority above graphics but above others
	}

	AudioSystem::~AudioSystem() {
		shutdown();
	}

	bool AudioSystem::init(SystemManager& /*system_manager*/) {

		//Create FMOD systems
		if (FMOD::Studio::System::create(&m_studiosystem) != FMOD_OK) {
			//LM write log
			return false;
		}

		if (m_studiosystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
			return false;
		}

		if (m_studiosystem->getCoreSystem(&m_coresystem) != FMOD_OK) {
			return false;
		}

		if(m_coresystem->createChannelGroup("SFXGroup", &m_sfxgroup) != FMOD_OK) {
			return false;
		}

		LM.writeLog("AudioSystem::init() - Audio System Initialized");
		return true;
	}

	void AudioSystem::update(float dt) {
		(void)dt;

		LM.writeLog("AudioSystem::update() - Updating Audio System");

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
			Entity* retrievecube = EM.getEntityByName("Cube");
			if (retrievecube) {
				if (retrievecube->has_component(get_component_type_id<AudioComponent>())) {
					AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
					if (audio) {
						audio->setPlayState(PlayState::PLAY);
					}
				}
			}
		}

		//Iterate through all entities with AudioComponent
		auto entities = EM.getEntitiesWithComponent<AudioComponent>();
		for (EntityID id : entities) {
			
			process_entity(id);
			
			//AudioComponent* audio = EM.getComponent<AudioComponent>(id);
			//if (!audio) {
			//	continue;
			//}

			//switch (audio->getPlayState()) {
			//	case PlayState::PLAY:
			//		playSound(id, audio);
			//		break;
			//	case PlayState::PAUSE:
			//		pauseSound(id, true);
			//		break;
			//	case PlayState::STOP:
			//		stopSound(id);
			//		break;
			//}

			//auto channel_it = m_activechannels.find(id);
			//if(channel_it != m_activechannels.end() && channel_it->second) {
			//	// Update volume and pitch in case they changed
			//	channel_it->second->setVolume(audio->getVolume());
			//	channel_it->second->setPitch(audio->getPitch());
			//}

		}

		cleanupInactiveChannels();
		cleanupInactiveEvents();
		updateVolumes();

		if (m_studiosystem) {
			m_studiosystem->update();
		}

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

		for (auto& pair : m_loadedbanks) {
			if (pair.second) {
				pair.second->unload();
			}
		}

		m_loadedbanks.clear();
		m_eventdes.clear();
		m_activeevents.clear();
		m_buses.clear();

		if (m_studiosystem) {
			m_studiosystem->unloadAll();
			m_studiosystem->release();
			m_studiosystem = nullptr;
		}

		m_coresystem = nullptr;
		m_sfxgroup = nullptr;

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

		//check if it has been played b4 / recorded in the active channels map
		auto channel_it = m_activechannels.find(id);
		if(channel_it != m_activechannels.end() && channel_it->second) {
			bool is_playing = false;
			channel_it->second->isPlaying(&is_playing);
			if (is_playing) {
				// Already playing, do not restart
				LM.writeLog("AudioSystem::playSound() - Sound %s on entity %u is already playing", audio->getGUID().c_str(), id);
				return;
			} else {
				// Channel is not playing, remove it from active channels
				m_activechannels.erase(channel_it);
			}
		}

		FMOD::Channel* channel = nullptr;
		if(m_coresystem->playSound(sound, nullptr, true, &channel) == FMOD_OK) {
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

	bool AudioSystem::playEvent(EntityID id, const std::string& eventpath) {

		auto it = m_eventdes.find(eventpath);
		if (it == m_eventdes.end()) {
			FMOD::Studio::EventDescription* eventdesc = nullptr;
			if (m_studiosystem->getEvent(eventpath.c_str(), &eventdesc) != FMOD_OK || !eventdesc) {
				LM.writeLog("AudioSystem::playEvent() - Failed to get event description for %s", eventpath.c_str());
				return false;
			}
			m_eventdes[eventpath] = eventdesc;
			it = m_eventdes.find(eventpath);
		}

		FMOD::Studio::EventDescription* eventdesc = it->second;
		if (!eventdesc) {
			return false;
		}

		FMOD::Studio::EventInstance* eventinstance = nullptr;
		if (eventdesc->createInstance(&eventinstance) != FMOD_OK || !eventinstance) {
			LM.writeLog("AudioSystem::playEvent() - Failed to create event instance for %s", eventpath.c_str());
			return false;
		}
		eventinstance->start();
		m_activeevents[id] = eventinstance;

		return true;
	}

	bool AudioSystem::stopEvent(EntityID id, bool immediate) {
		auto it = m_activeevents.find(id);
		if(it != m_activeevents.end() && it->second) {
			it->second->stop(immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT);
			it->second->release();
			m_activeevents.erase(it);
			LM.writeLog("AudioSystem::stopEvent() - Stopped event on entity %u", id);
			return true;
		}
		return false;
	}

	bool AudioSystem::pauseEvent(EntityID id, bool pause) {
		auto it = m_activeevents.find(id);
		if(it != m_activeevents.end() && it->second) {
			it->second->setPaused(pause);
			LM.writeLog("AudioSystem::pauseEvent() - %s event on entity %u", pause ? "Paused" : "Resumed", id);
			return true;
		}
		return false;
	}

	bool AudioSystem::loadBankTemp(const std::string& path) {
		if (!m_studiosystem) {
			return false;
		}
		if (m_loadedbanks.find(path) != m_loadedbanks.end()) {
			return true; // already loaded
		}

		FMOD::Studio::Bank* bank = nullptr;
		if (m_studiosystem->loadBankFile(path.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank) != FMOD_OK) {
			LM.writeLog("AudioSystem::loadBankTemp() - Failed to load bank %s", path.c_str());
			return false;
		}
		m_loadedbanks[path] = bank;
		LM.writeLog("AudioSystem::loadBankTemp() - Loaded bank %s", path.c_str());
		return true;
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
		if (m_coresystem->createSound(path.c_str(), mode, nullptr, &sound) != FMOD_OK) {
			LM.writeLog("AudioSystem::loadSoundTemp() - Failed to load %s", path.c_str());
			return false;
		}

		m_loadedsounds[path] = sound;
		LM.writeLog("AudioSystem::loadSoundTemp() - Loaded %s", path.c_str());
		return true;
	}

	void AudioSystem::unloadBank(const std::string& path) {
		auto it = m_loadedbanks.find(path);
		if (it != m_loadedbanks.end() && it->second) {
			it->second->unload();
			m_loadedbanks.erase(it);
			LM.writeLog("AudioSystem::unloadBank() - Unloaded bank %s", path.c_str());
		}
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

	void AudioSystem::setBusVolume(const std::string& buspath, float volume) {
		if (!m_studiosystem) {
			return;
		}

		auto it = m_buses.find(buspath);
		FMOD::Studio::Bus* bus = nullptr;
		if (it == m_buses.end()) {
			if (m_studiosystem->getBus(buspath.c_str(), &bus) != FMOD_OK) {
				LM.writeLog("AudioSystem::setBusVolume() - Failed to get bus %s", buspath.c_str());
				return;
			}
			m_buses[buspath] = bus;
		} else {
			bus = it->second;
		}
		if (bus) {
			bus->setVolume(volume);
			LM.writeLog("AudioSystem::setBusVolume() - Set volume of bus %s to %f", buspath.c_str(), volume);
		}
	}

	void AudioSystem::getMasterVolume(float& volume) const {
		volume = m_mastervolume;
	}

	void AudioSystem::getSFXGroupVolume(float& volume) const {
		volume = m_sfxgroupvolume;
	}

	void AudioSystem::getBusVolume(const std::string& buspath, float& volume) const {
		if (!m_studiosystem) {
			volume = 0.0f;
			return;
		}
		auto it = m_buses.find(buspath);
		if(it != m_buses.end() && it->second) {
			it->second->getVolume(&volume);
		} else {
			volume = 0.0f;
		}
	}

	void AudioSystem::cleanupInactiveChannels() {

		std::vector<EntityID> to_remove;

		for (const auto& pair : m_activechannels) {
			bool is_playing = false;
			if (pair.second && pair.second->isPlaying(&is_playing) == FMOD_OK) {
				if (!is_playing) {
					to_remove.push_back(pair.first);
				}
			}
		}

		for (EntityID id : to_remove) {
			m_activechannels.erase(id);
			LM.writeLog("AudioSystem::cleanupInactiveChannels() - Removed inactive channel for entity %u", id);
		}
	}

	void AudioSystem::cleanupInactiveEvents() {
		std::vector<EntityID> to_remove;
		for (const auto& pair : m_activeevents) {
			FMOD_STUDIO_PLAYBACK_STATE state;
			if (pair.second && pair.second->getPlaybackState(&state) == FMOD_OK) {
				if (state == FMOD_STUDIO_PLAYBACK_STOPPED) {
					pair.second->release();
					to_remove.push_back(pair.first);
				}
			}
		}
		for (EntityID id : to_remove) {
			m_activeevents.erase(id);
			LM.writeLog("AudioSystem::cleanupInactiveEvents() - Removed inactive event for entity %u", id);
		}
	}

	void AudioSystem::updateVolumes() {
		if (m_sfxgroup) {
			float final_volume = m_mastervolume * m_sfxgroupvolume;
			m_sfxgroup->setVolume(final_volume);
		}

		for (auto& pair : m_buses) {
			if (pair.second) {
				pair.second->setVolume(m_mastervolume);
			}
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

}