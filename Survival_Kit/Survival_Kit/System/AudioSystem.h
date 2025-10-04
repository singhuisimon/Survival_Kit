/**
 * @file AudioSystem.h
 * @brief Declaration of the AudioSystem class — wrapper for FMOD Core audio system.
 * @details
 * Provides initialization, playback, and 3D sound handling for entities containing AudioComponents.
 * Supports channel grouping (Master, SFX, BGM) and editor-level sound previewing.
 * 
 * @author Amanda Leow Boon Suan (100%)
 * @date 21/9/2025
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
#include "../Component/RigidBody.h"

#include <fmod.hpp>
#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include <fmod_common.h>

#include <unordered_map>
#include <string>

#include "../Manager/AssetManager.h"
#include "../Manager/ResourceManager.h"

namespace gam300 {

	/**
	 * @class AudioSystem
	 * @brief ECS System that handles audio playback and 3D sound using FMOD Core.
	 * @details
	 * Responsible for initializing FMOD, loading and unloading sounds,
	 * managing active channels, handling spatial updates, and managing
	 * master/group volume controls.
	 */
	class AudioSystem : public ComponentSystem<AudioComponent, Transform3D, RigidBody> {
	public:

		/**************************************************************************
		* @brief Constructs the AudioSystem and assigns its system priority.
		**************************************************************************/
		AudioSystem();

		/**************************************************************************
		* @brief Destroys the AudioSystem and releases FMOD resources.
		**************************************************************************/
		~AudioSystem();

		/**************************************************************************
		* @brief
		* Initializes the FMOD Core audio system and creates master and group channels.
		* @param system_manager
		* Reference to the system manager managing this system.
		* @return
		* True if initialization succeeds, false otherwise.
		**************************************************************************/
		bool init(SystemManager& system_manager) override;

		/**************************************************************************
		* @brief Updates all active audio components every frame.
		* @param dt
		* Delta time in seconds.
		**************************************************************************/
		void update(float dt) override;

		/**************************************************************************
		* @brief Shuts down the audio system, releasing all FMOD objects and channels.
		**************************************************************************/
		void shutdown() override;

		/**************************************************************************
		* @brief Processes a single entity's AudioComponent each frame.
		* @param entity_id
		* The ID of the entity to process.
		**************************************************************************/
		void process_entity(EntityID entity_id) override;

		// ------------------------Core Audio Control -------------------------- //

		 /**
		 * @brief Plays an audio clip associated with a given entity.
		 * @param id Entity ID owning the AudioComponent.
		 * @param audio Pointer to the AudioComponent instance.
		 */
		void playSound(EntityID id, AudioComponent* audio);

		/**
		 * @brief Stops playback of an audio clip on a given entity.
		 * @param id Entity ID whose sound should be stopped.
		 */
		void stopSound(EntityID id);

		/**
		 * @brief Pauses or resumes playback for a given entity.
		 * @param id Entity ID owning the AudioComponent.
		 * @param pause True to pause, false to resume.
		 */
		void pauseSound(EntityID id, bool pause);

		//---------------------- Editor Playback ----------------------//

		/**
		 * @brief Plays a sound for preview in the level editor.
		 * @param handle Asset handle of the sound to preview.
		 */
		void playeditor(AssetId handle);

		/**
		 * @brief Stops playback of a sound preview in the editor.
		 * @param handle Asset handle of the sound to stop.
		 */
		void stopeditor(AssetId handle);

		//---------------------- Sound Management ----------------------//

		/**
		 * @brief Loads a sound into memory using its asset handle.
		 * @param handle Asset handle of the sound file.
		 * @param loop Whether the sound should loop.
		 * @return True if loaded successfully, false otherwise.
		 */
		bool loadSoundTemp(AssetId handle, bool loop = false);

		/**
		 * @brief Unloads a sound and frees FMOD resources.
		 * @param handle Asset handle of the sound file to unload.
		 */
		void unloadSound(AssetId handle);

		//---------------------- Listener Attributes ----------------------//

		/**
		 * @brief Sets 3D listener attributes for spatial audio.
		 * @param position Listener position in world space.
		 * @param forward Listener forward direction vector.
		 * @param up Listener up vector.
		 * @param velocity Listener velocity vector.
		 */
		void setListenerAttributes(const Vector3D& position, const Vector3D& forward, const Vector3D& up, const Vector3D& velocity);

		//---------------------- Volume Controls ----------------------//

		/**
		 * @brief Sets the master volume applied to all groups.
		 * @param volume Master volume multiplier (0.0f–1.0f).
		 */
		void setMasterVolume(float volume);

		/**
		 * @brief Sets the volume for the SFX channel group.
		 * @param volume Volume multiplier (0.0f–1.0f).
		 */
		void setSFXGroupVolume(float volume);

		/**
		 * @brief Retrieves the current master volume.
		 * @param volume Reference to store master volume value.
		 */
		void getMasterVolume(float& volume) const;

		/**
		 * @brief Retrieves the current SFX group volume.
		 * @param volume Reference to store SFX group volume value.
		 */
		void getSFXGroupVolume(float& volume) const;

		//---------------------- Editor State ----------------------//

		/**
		 * @brief Retrieves the playback state of a sound in the editor preview.
		 * @param handle Asset handle of the sound.
		 * @return PlayState indicating PLAY, PAUSE, or STOP.
		 */
		PlayState editorchannel_status(AssetId handle);

		//---------------------- Debug ----------------------//

		/**
		 * @brief Checks if a channel for a given entity is virtual (culled by FMOD).
		 * @param id Entity ID owning the AudioComponent.
		 * @return True if the channel is virtual, false otherwise.
		 */
		bool isChannelVirtual(EntityID id);

	private:
		//---------------------- Internal Helpers ----------------------//

		/**
		 * @brief Removes inactive channels from the tracking map.
		 */
		void cleanupInactiveChannels();

		/**
		 * @brief Updates the master and group volume multipliers.
		 */
		void updateVolumes();

		/**
		 * @brief Updates 3D positional attributes for a sound channel.
		 * @param id Entity ID.
		 * @param audio Pointer to the AudioComponent.
		 * @param transform Pointer to the entity’s Transform3D.
		 */
		void update3DAttributes(EntityID id, AudioComponent* audio, Transform3D* transform);

		//---------------------- FMOD Objects ----------------------//

		FMOD::System* m_coresystem = nullptr;							///< Pointer to FMOD Core System.
		std::unordered_map<AssetId, FMOD::Sound*> m_loadedsounds;		///< Loaded sound cache.
		std::unordered_map<EntityID, FMOD::Channel*> m_activechannels;	///< Active sound channels.
		std::unordered_map<EntityID, AssetId> m_previousHandles;		///< Track last played handles.
		std::unordered_map<AssetId, FMOD::Channel*> m_editorchannel;	///< Editor preview channels.

		//---------------------- Channel Groups ----------------------//

		FMOD::ChannelGroup* m_sfxgroup = nullptr;	///< SFX channel group.
		FMOD::ChannelGroup* m_bgmgroup = nullptr;	///< BGM channel group.
		FMOD::ChannelGroup* m_mastergroup = nullptr;///< Master channel group.

		//---------------------- Volume Levels ----------------------//

		float m_mastervolume = 1.0f;	///< Master volume multiplier.
		float m_bgmvolume = 1.0f;		///< BGM volume multiplier.
		float m_sfxgroupvolume = 1.0f;	///< SFX group volume multiplier.


	};

}

#endif // __AUDIO_SYSTEM_H__