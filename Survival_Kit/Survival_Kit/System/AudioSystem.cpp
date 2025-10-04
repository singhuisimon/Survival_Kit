/**
 * @file AudioSystem.h
 * @brief Wrappers for FMOD audio system.
 * @details Contains the prototype of wrappers for FMOD audio system functionalities.
 * @author Amanda Leow Boon Suan (100%)
 * @date 21/9/2025
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
#include "../Tracy/tracy/Tracy.hpp"

namespace gam300 {

	AudioSystem::AudioSystem() : ComponentSystem<AudioComponent, Transform3D, RigidBody>("AudioSystem") {
		set_priority(150); //set priority above graphics but above others
	}

	AudioSystem::~AudioSystem() {
		shutdown();
	}

	bool AudioSystem::init(SystemManager& /*system_manager*/) {

		if (FMOD::System_Create(&m_coresystem) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - FMOD::System_Create failed");
			return false;
		}

		if (m_coresystem->init(512, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - m_coresystem->init failed");
			return false;
		}

		if (m_coresystem->getMasterChannelGroup(&m_mastergroup) != FMOD_OK) {
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

		m_mastergroup->addGroup(m_sfxgroup);
		m_mastergroup->addGroup(m_bgmgroup);

		if (m_coresystem->set3DSettings(1.0f, 1.0f, 1.0f) != FMOD_OK) {
			LM.writeLog("AudioSystem::init() - m_coresystem->set3DSettings failed");
			return false;
		}

		//setListenerAttributes(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(0.0f, 0.0f, 1.0f), Vector3D(0.0f, 1.0f, 0.0f), Vector3D(0.0f, 0.0f, 0.0f));

		LM.writeLog("AudioSystem::init() - Audio System Initialized");
		return true;
	}

	void AudioSystem::update(float dt) {
		(void)dt;

		//LM.writeLog("AudioSystem::update() - Updating Audio System");

		/*if (IM.isKeyPressed(GLFW_KEY_P)) {
			LM.writeLog("AudioSystem::update() - Play sound on Cube pressed");
			Entity* retrievecube = EM.getEntityByName("New Entity_1");
			if (retrievecube) {
				if (retrievecube->has_component(get_component_type_id<AudioComponent>())) {
					AudioComponent* audio = EM.getComponent<AudioComponent>(retrievecube->get_id());
					if (audio) {
						audio->setPlayState(PlayState::PLAY);
					}
				}
			}

		}*/

		//Iterate through all entities with AudioComponent
		auto entities = EM.getEntitiesWithComponent<AudioComponent>();
		for (EntityID id : entities) {

			process_entity(id);

		}

		cleanupInactiveChannels();
		updateVolumes();

		//for testing itself but rememebr to set listener attributes to camera or smth
		Entity* camera = EM.getEntityByName("Camera");
		if (camera && camera->has_component(get_component_type_id<Transform3D>())) {
			Transform3D* camtransform = EM.getComponent<Transform3D>(camera->get_id());
			if (camtransform) {
				Vector3D campos = camtransform->getPosition();
				Vector3D camforward = camtransform->getForward();
				Vector3D camup = camtransform->getUp();

				Vector3D camvelocity(0.0f, 0.0f, 0.0f);
				if (camera->has_component(get_component_type_id<RigidBody>())) {
					RigidBody* camrigid = EM.getComponent<RigidBody>(camera->get_id());
					if (camrigid) {
						//camvelocity = camrigid->getVelocity();
					}
				}

				// Assuming velocity is zero for simplicity; can be calculated if needed
				setListenerAttributes(campos, camforward, camup, camvelocity);
			}
		}

		if (m_coresystem) {
			m_coresystem->update();
		}
	}

	void AudioSystem::shutdown() {

		// Stop and release all active channels
		for (auto& pair : m_activechannels) {
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
			if (pair.second) {
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

		if (m_mastergroup) {
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

		//**DEBUG: Log what state we're seeing**
			LM.writeLog("AudioSystem::process_entity() - Entity %u state: %d",
				entity_id, static_cast<int>(audio->getPlayState()));

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

		if (audio->is3D() && transform) {
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
		}

		m_previousguids[id] = audio->getGUID();

		//check if it has been played b4 / recorded in the active channels map
		auto channel_it = m_activechannels.find(id);
		if (channel_it != m_activechannels.end() && channel_it->second) {
			bool is_playing = false;
			channel_it->second->isPlaying(&is_playing);
			bool is_paused = false;
			channel_it->second->getPaused(&is_paused);
			if (is_playing && !is_paused) {
				// Already playing, do not restart

				//update the channel itself and check if its virtual
				//i add most while half awake if anything goes wrong find me - amanda
				float volume = 0.0f;
				float pitch = 0.0f;
				channel_it->second->getVolume(&volume);
				channel_it->second->getPitch(&pitch);
				if (volume != audio->getVolume() ||
					pitch != audio->getPitch()) {
					channel_it->second->setVolume(volume);
					channel_it->second->setPitch(pitch);
				}

				bool muted = false;
				channel_it->second->getMute(&muted);
				if (audio->isMute() != muted) {
					channel_it->second->setMute(audio->isMute());
				}

				FMOD_MODE current_mode;
				channel_it->second->getMode(&current_mode);

				if (audio->isLooping()) {
					// Should be looping
					if (!(current_mode & FMOD_LOOP_NORMAL)) {
						channel_it->second->setMode((current_mode & ~FMOD_LOOP_OFF) | FMOD_LOOP_NORMAL);
						channel_it->second->setLoopCount(-1);
						LM.writeLog("AudioSystem::playSound() - Changed entity %u to LOOP mode", id);
					}
				}
				else {
					// Should NOT be looping
					if (current_mode & FMOD_LOOP_NORMAL) {
						channel_it->second->setMode((current_mode & ~FMOD_LOOP_NORMAL) | FMOD_LOOP_OFF);
						channel_it->second->setLoopCount(0);
						LM.writeLog("AudioSystem::playSound() - Changed entity %u to NO LOOP mode", id);
					}
				}

				// Update 3D attributes if needed
				if (audio->is3D()) {
					Transform3D* transform = EM.getComponent<Transform3D>(id);
					if (transform) {
						update3DAttributes(id, audio, transform);
					}
				}

				isChannelVirtual(id);

				LM.writeLog("AudioSystem::playSound() - Sound %s on entity %u is already playing", audio->getGUID().c_str(), id);
				return;
			}
			else if(!is_playing && !is_paused){
				LM.writeLog("AudioSystem::playSound() - Sound %s on entity % u stop playing already", audio->getGUID().c_str(), id);
				/*if (audio->getPlayState() != PlayState::STOP) {
					audio->setPlayState(PlayState::STOP);
				}*/

				stopSound(id);
				return;
				// Channel is not playing, remove it from active channels
				//m_activechannels.erase(channel_it);
			}
		}

		FMOD::Channel* channel = nullptr;
		FMOD::ChannelGroup* group = nullptr;

		if (audio->getType() == AudioType::SFX) {
			group = m_sfxgroup;
		}
		else if (audio->getType() == AudioType::BGM) {
			group = m_bgmgroup;
		}

		if (m_coresystem->playSound(sound, group, true, &channel) == FMOD_OK) {
			if (channel) {
				channel->setVolume(audio->getVolume());
				channel->setPitch(audio->getPitch());
				channel->setMute(audio->isMute());

				if (audio->isLooping()) {
					LM.writeLog("SET LOOP NORMAL");
					channel->setMode(FMOD_LOOP_NORMAL);
					channel->setLoopCount(-1); // Infinite loop
				}
				else {
					LM.writeLog("SET NO LOOP");
					channel->setMode(FMOD_LOOP_OFF);
					channel->setLoopCount(0);
				}

				if (audio->is3D()) {
					channel->setMode(FMOD_3D | FMOD_3D_LINEARROLLOFF);

					Transform3D* transform = EM.getComponent<Transform3D>(id);
					if (transform) {
						Vector3D pos = transform->getPosition();
						FMOD_VECTOR fmod_pos = { pos.x, pos.y, pos.z };
						FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
						RigidBody* rigid = EM.getComponent<RigidBody>(id);
						if (rigid) {
							Vector3D velocity = rigid->getVelocity();
							vel.x = velocity.x;
							vel.y = velocity.y;
							vel.z = velocity.z;
						}

						LM.writeLog("AudioSystem::playSound() - Entity %u sound position: (%.2f, %.2f, %.2f)",
							id, pos.x, pos.y, pos.z);

						// Get listener position for distance calculation <for debug>
						/*Entity* camera = EM.getEntityByName("Camera");
						if (camera && camera->has_component(get_component_type_id<Transform3D>())) {
							Transform3D* camtransform = EM.getComponent<Transform3D>(camera->get_id());
							if (camtransform) {
								Vector3D campos = camtransform->getPosition();
								float distance = (pos - campos).magnitude();
								LM.writeLog("AudioSystem::playSound() - Camera position: (%.2f, %.2f, %.2f), Distance: %.2f",
									campos.x, campos.y, campos.z, distance);
							}
						}*/

						channel->set3DAttributes(&fmod_pos, &vel);
					}
					else {
						FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
						//FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
						channel->set3DAttributes(&pos, nullptr);
					}

					channel->set3DMinMaxDistance(audio->getMinDistance(), audio->getMaxDistance());
					LM.writeLog("AudioSystem::playSound() - Playing 3D sound on entity %u", id);
				}
				else {
					// Add 2D mode to existing mode flags
					FMOD_MODE current_mode;
					channel->getMode(&current_mode);
					channel->setMode(current_mode | FMOD_2D);
					LM.writeLog("AudioSystem::playSound() - Playing 2D sound on entity %u", id);
				}

				channel->setPaused(false); // Start playing

				m_activechannels[id] = channel;
				LM.writeLog("AudioSystem::playSound() - Playing sound %s on entity %u", audio->getGUID().c_str(), id);
			}
		}

	}

	void AudioSystem::stopSound(EntityID id) {
		auto it = m_activechannels.find(id);
		if (it != m_activechannels.end() && it->second) {
			
			// Quick volume ramp to prevent clicking
			float current_volume;
			it->second->getVolume(&current_volume);

			// Ramp down volume over a few frames
			const int ramp_steps = 5;
			for (int i = ramp_steps; i > 0; --i) {
				it->second->setVolume(current_volume * (i / (float)ramp_steps));
			}
			
			it->second->setVolume(0.0f);
			it->second->stop();
			AudioComponent* audio = EM.getComponent<AudioComponent>(id);
			if (audio) {
				audio->setPlayState(PlayState::STOP);
			}
			LM.writeLog("AudioSystem::stopSound() - Stopped sound on entity %u", id);
		}
	}

	void AudioSystem::pauseSound(EntityID id, bool pause) {
		auto it = m_activechannels.find(id);
		if (it != m_activechannels.end() && it->second) {
			it->second->setPaused(pause);
			AudioComponent* audio = EM.getComponent<AudioComponent>(id);
			if (audio) {
				audio->setPlayState(PlayState::PAUSE);
			}
			LM.writeLog("AudioSystem::pauseSound() - %s sound on entity %u", pause ? "Paused" : "Resumed", id);
		}
	}

	bool AudioSystem::loadSoundTemp(const std::string& path, bool loop) {
		(void)loop;
		if (!m_coresystem) {
			return false;
		}

		if (m_loadedsounds.find(path) != m_loadedsounds.end()) {
			return true; // already loaded
		}

		FMOD_MODE mode = FMOD_DEFAULT;

		//always load as 3D sound for this project
		mode |= FMOD_3D;
		mode |= FMOD_LOOP_OFF;

		/*if (loop) {
			mode |= FMOD_LOOP_NORMAL;
		}*/

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

			if (!is_playing && !is_paused) {
				to_remove.push_back(pair.first);
				AudioComponent* audio = EM.getComponent<AudioComponent>(pair.first);
				if (audio && audio->getPlayState() != PlayState::STOP) {
					audio->setPlayState(PlayState::STOP);
				}
			}
		}

		std::vector<std::string> editorguids_to_remove;
		for (const auto& pair : m_editorchannel) {
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

		for (const std::string& guid : editorguids_to_remove) {
			m_editorchannel.erase(guid);
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
			LM.writeLog("AudioSystem::update3DAttributes() - Invalid parameters");
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
				channel->setMode(FMOD_2D);
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

	PlayState AudioSystem::editorchannel_status(const std::string& path) {
		auto it = m_editorchannel.find(path);
		if (it != m_editorchannel.end() && it->second) {
			bool is_playing = false;
			bool is_paused = false;
			it->second->getPaused(&is_paused);
			it->second->isPlaying(&is_playing);
			if (is_playing && !is_paused) {
				return PlayState::PLAY;
			}
			else if (!is_playing && is_paused) {
				return PlayState::PAUSE;
			}
			else {
				return PlayState::STOP;
			}
		}
		return PlayState::STOP;
	}

	bool AudioSystem::isChannelVirtual(EntityID id) {
		auto it = m_activechannels.find(id);
		if (it != m_activechannels.end() && it->second) {
			bool is_virtual = false;
			if (it->second->isVirtual(&is_virtual) == FMOD_OK) {
				LM.getInstance().writeLog("AudioSystem::isChannelVirtual() - Channel for entity %u is %s", id, is_virtual ? "virtual" : "not virtual");
				return is_virtual;
			}
		}
		LM.getInstance().writeLog("AudioSystem::isChannelVirtual() - No active channel for entity %u", id);
		return false;
	}

}