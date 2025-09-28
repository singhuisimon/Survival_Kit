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
		LM.writeLog("AudioSystem::init() - Compiled against FMOD version: %08x", FMOD_VERSION);

		// Create Core system first
		FMOD_RESULT result = FMOD::System_Create(&m_coresystem);
		if (result != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - Failed to create Core system: %s", FMOD_ErrorString(result));
			return false;
		}

		// Initialize Core system
		result = m_coresystem->init(512, FMOD_INIT_NORMAL, 0);
		if (result != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - Failed to initialize Core system: %s", FMOD_ErrorString(result));
			m_coresystem->release();
			m_coresystem = nullptr;
			return false;
		}

		// Verify Core system is working
		unsigned int core_version = 0;
		result = m_coresystem->getVersion(&core_version);
		if (result != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - Core system version check failed: %s", FMOD_ErrorString(result));
			m_coresystem->release();
			m_coresystem = nullptr;
			return false;
		}
		LM.writeLog("AudioSystem::init() - Core system version: %08x", core_version);

		// Create SFX channel group
		result = m_coresystem->createChannelGroup("SFXGroup", &m_sfxgroup);
		if (result != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - Failed to create SFX channel group: %s", FMOD_ErrorString(result));
			m_coresystem->release();
			m_coresystem = nullptr;
			return false;
		}

		// Create Studio system
		result = FMOD::Studio::System::create(&m_studiosystem);
		if (result != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - Failed to create Studio system: %s", FMOD_ErrorString(result));
			// Cleanup Core system
			if (m_sfxgroup) {
				m_sfxgroup = nullptr; // Channel groups are released automatically with Core system
			}
			m_coresystem->release();
			m_coresystem = nullptr;
			return false;
		}

		// Initialize Studio system
		result = m_studiosystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
		if (result != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - Failed to initialize Studio system: %s", FMOD_ErrorString(result));
			// Cleanup both systems
			m_studiosystem->release();
			m_studiosystem = nullptr;
			if (m_sfxgroup) {
				m_sfxgroup = nullptr;
			}
			m_coresystem->release();
			m_coresystem = nullptr;
			return false;
		}

		LM.writeLog("AudioSystem::init() - Audio System Initialized successfully");
		LM.writeLog("AudioSystem::init() - Core system: %p, Studio system: %p", (void*)m_coresystem, (void*)m_studiosystem);
		return true;
	}

	void AudioSystem::update(float dt) {
		(void)dt;

		//LM.writeLog("AudioSystem::update() - Updating Audio System");

		if (IM.isKeyJustReleased(GLFW_KEY_P)) {
			LM.writeLog("AudioSystem::update() - Play sound on Cube release");
			/*Entity* retrievecube = EM.getEntityByName("Cube");
			if (retrievecube) {
				if(retrievecube->has_component(get_component_type_id<AudioComponent>())) {
					AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
					if (audio) {
						audio->setPlayState(PlayState::PLAY);
					}
				}
			}*/
		}

		if (IM.isKeyJustPressed(GLFW_KEY_P)) {
			LM.writeLog("AudioSystem::update() - Play sound on Cube just press");
			/*Entity* retrievecube = EM.getEntityByName("Cube");
			if (retrievecube) {
				if (retrievecube->has_component(get_component_type_id<AudioComponent>())) {
					AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
					if (audio) {
						audio->setPlayState(PlayState::PLAY);
					}
				}
			}*/
		}

		//for testing rn
		if (IM.isKeyPressed(GLFW_KEY_P)){//} && !m_p_pressed) {
			LM.writeLog("AudioSystem::update() - Play sound when p pressed");
			/*Entity* sound = EM.getEntityByName("sound");
			if (sound) {
				if (sound->has_component(get_component_type_id<AudioComponent>())) {
					AudioComponent* audio = EM.getComponent<AudioComponent>(sound->get_id());
					if (audio) {
						audio->setPlayState(PlayState::PLAY);
					}
				}
			}
			m_p_pressed = true;*/
		}

		//Iterate through all entities with AudioComponent
		auto entities = EM.getEntitiesWithComponent<AudioComponent>();
		for (EntityID id : entities) {
			
			process_entity(id);

		}

		cleanupInactiveChannels();
		cleanupInactiveEvents();
		updateVolumes();

		if (m_coresystem) {
			m_coresystem->update();
		}

		if (m_studiosystem) {
			m_studiosystem->update();
		}
	}

	void AudioSystem::shutdown() {

		// Stop and release all active events
		for(auto& pair : m_activeevents) {
			if (pair.second) {
				pair.second->stop(FMOD_STUDIO_STOP_IMMEDIATE);
				pair.second->release();
			}
		}

		m_activeevents.clear();

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

		//Unload all loaded banks
		for (auto& pair : m_loadedbanks) {
			if (pair.second) {
				pair.second->unload();
			}
		}

		m_loadedbanks.clear();

		m_eventdes.clear();
		m_buses.clear();

		if (m_studiosystem) {
			m_studiosystem->unloadAll();
			m_studiosystem->release();
			m_studiosystem = nullptr;
			LM.writeLog("AudioSystem::shutdown() - Studio system released");
		}

		m_sfxgroup = nullptr;

		if(m_coresystem){
			m_coresystem->close();
			m_coresystem->release();
			m_coresystem = nullptr;
			LM.writeLog("AudioSystem::shutdown() - Core system released");
		}

		LM.writeLog("AudioSystem::shutdown() - Audio System shut down");
	}

	void AudioSystem::process_entity(EntityID entity_id) {
		AudioComponent* audio = EM.getComponent<AudioComponent>(entity_id);
		Transform3D* transform = EM.getComponent<Transform3D>(entity_id);

		if (!audio) {
			return;
		}

		/*auto channel_it = m_activechannels.find(entity_id);
		FMOD::Channel* channel = (channel_it != m_activechannels.end()) ? channel_it->second : nullptr;

		if (channel) {
			bool is_playing = false;
			channel->isPlaying(&is_playing);

			bool is_paused = false;
			channel->getPaused(&is_paused);

			if (!is_playing && !is_paused) {
				audio->setPlayState(PlayState::STOP);
				m_activechannels.erase(channel_it);
				LM.writeLog("AudioSystem::process_entity() - Sound on entity %u finished playing", entity_id);
			}
			else {
				if (is_paused && audio->getPlayState() != PlayState::PAUSE) {
					audio->setPlayState(PlayState::PAUSE);
				} else if (!is_paused && audio->getPlayState() == PlayState::PAUSE) {
					audio->setPlayState(PlayState::PLAY);
				}
			}
		}*/

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
		FMOD::ChannelGroup* group = nullptr;

		if(audio->getType() == AudioType::SFX) {
			group = m_sfxgroup;
		} else if (audio->getType() == AudioType::BGM) {
			// Handle BGM group if needed
			playEvent(id, audio->getGUID());
			return;
		}

		if (m_coresystem && m_coresystem->playSound(sound, group, false, &channel) == FMOD_OK) {
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

		if (!m_studiosystem) {
			LM.writeLog("AudioSystem::playEvent() - Studio system not initialized");
			return false;
		}

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
		if (m_studiosystem->loadBankFile(getAssetFilePath(path).c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank) != FMOD_OK) {
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
		if (m_coresystem->createSound(getAssetFilePath(path).c_str(), mode, nullptr, &sound) != FMOD_OK) {
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
			bool is_paused = false;

			pair.second->getPaused(&is_paused);
			pair.second->isPlaying(&is_playing); //does not factor if it is paused.

			if(!is_playing && !is_paused) {
				to_remove.push_back(pair.first);
			}

			AudioComponent* audio = EM.getComponent<AudioComponent>(pair.first);
			if (audio && audio->getPlayState() != PlayState::STOP) {
				audio->setPlayState(PlayState::STOP);
			}
			else {
				LM.writeLog("AudioSystem::cleanupInactiveChannels() fail to retrieve audio comp from %u", pair.first);
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