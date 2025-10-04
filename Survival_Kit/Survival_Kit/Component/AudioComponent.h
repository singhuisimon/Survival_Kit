/**
 * @file AudioComponent.h
 * @brief Declaration of the AudioComponent for the Entity Component System.
 * @details Handles the audio properties and playback for entities.
 * @author	Amanda Leow Boon Suan (100%)
 * @date	21/9/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __AUDIOCOMPONENT_H__
#define __AUDIOCOMPONENT_H__

#include "../Component/Component.h"
#include "../Utility/Vector3D.h"
#include "../Manager/LogManager.h"
#include "../Manager/AssetManager.h"
#include <string>

namespace gam300 {

	/**
	* @enum AudioType
	* @brief Defines the type of audio clip
	*/
	enum class AudioType {
		SFX,
		BGM
	};

	/**
	* @enum PlayState
	* @brief Represents the playback state of the audio
	*/
	enum class PlayState {
		PLAY,
		PAUSE,
		STOP
	};

	/**
	* @class AudioComponent
	* @brief ECS component that stores audio playback data and configuration.
	* @details
	* Each entity that can emit sound has one AudioComponent. This class
	* stores parameters such as volume, pitch, 3D attributes, and play state.
	*/
	class AudioComponent : public Component {
	public:
		/**
		* @brief Constructs an AudioComponent with configurable playback parameters.
        * @param audioHandle
        * Asset handle ID referencing the audio resource managed by AssetManager.
        * @param type
        * Type of the audio (SFX or BGM).
        * @param playstate
        * Initial playback state of the audio.
        * @param volume
        * Audio volume (range: 0.0f to 1.0f).
        * @param pitch
        * Pitch modifier (1.0f is normal pitch).
        * @param loop
        * Whether the audio should loop continuously.
        * @param mute
        * Whether the audio is initially muted.
        * @param is3D
        * Whether this sound is spatial (3D) or non-spatial (2D).
        * @param minDistance
        * Minimum distance for 3D audio attenuation.
        * @param maxDistance
        * Maximum distance for 3D audio attenuation.
		*/
		AudioComponent(AssetId audioHandle = 0,
			//int64_t audioID = -1,
			AudioType type = AudioType::SFX,
			PlayState playstate = PlayState::STOP,
			float volume = 1.0f,
			float pitch = 1.0f,
			bool loop = false,
			bool mute = false,
			bool is3D = true,
			float minDistance = 1.0f,
			float maxDistance = 100.0f);

		/**************************************************************************
		* @brief Initializes the AudioComponent when attached to an entity.
		* @param entity_id
		* ID of the entity this component belongs to.
		**************************************************************************/
		void init(EntityID entity_id) override;

		/**************************************************************************
		* @brief Updates the component (usually passive, data-only container).
		* @param dt
		* Delta time in seconds.
		**************************************************************************/
		void update(float dt) override;

		// ----------------------- Getters ----------------------//
		/**
		 * @brief Retrieves the audio handle assigned to this component.
		 * @return AssetId of the associated audio.
		 */
		AssetId getHandle() const { return m_audioHandle; }

		/**
		 * @brief Retrieves the audio type (SFX or BGM).
		 * @return AudioType enumeration value.
		 */
		AudioType getType() const { return m_type; }

		/**
		 * @brief Retrieves the current playback state.
		 * @return Current PlayState of the audio.
		 */
		PlayState getPlayState() const { return m_playState; }

		/**
		* @brief Gets the current volume of the audio.
		* @return Volume level (0.0f to 1.0f).
		*/
		float getVolume() const { return m_volume; }

		/**
		 * @brief Gets the current pitch value.
		 * @return Pitch multiplier.
		 */
		float getPitch() const { return m_pitch; }

		/**
		 * @brief Checks if the sound is looping.
		 * @return True if looping, false otherwise.
		 */
		bool isLooping() const { return m_loop; }

		/**
		 * @brief Checks if the sound is muted.
		 * @return True if muted, false otherwise.
		 */
		bool isMute() const { return m_mute; }

		/**
		 * @brief Checks if the sound is a 3D spatial sound.
		 * @return True if 3D, false if 2D.
		 */
		bool is3D() const {	return m_is3D;}

		/**
		 * @brief Retrieves the minimum distance for 3D sound attenuation.
		 * @return Minimum distance value.
		 */
		float getMinDistance() const { return m_minDistance; }

		/**
		 * @brief Retrieves the maximum distance for 3D sound attenuation.
		 * @return Maximum distance value.
		 */
		float getMaxDistance() const { return m_maxDistance; }

		// -------------------- Setters ---------------------------//
		/**
		 * @brief Assigns a new audio handle.
		 * @param handle Asset handle of the audio.
		 */
		void setHandle(AssetId handle) { m_audioHandle = handle; }

		/**
		 * @brief Sets the audio type.
		 * @param type Type of audio (SFX or BGM).
		 */
		void setType(AudioType type) { m_type = type; }

		/**
		 * @brief Sets the playback state of the audio.
		 * @param state New playback state.
		 */
		void setPlayState(PlayState state) { m_playState = state; }

		/**
		 * @brief Adjusts the audio volume.
		 * @param volume Volume value (0.0f to 1.0f).
		 */
		void setVolume(float volume) { m_volume = volume; }

		/**
		 * @brief Adjusts the audio pitch.
		 * @param pitch Pitch multiplier.
		 */
		void setPitch(float pitch) { m_pitch = pitch; }

		/**
		 * @brief Sets whether the audio loops.
		 * @param loop True to enable looping.
		 */
		void setLooping(bool loop) { m_loop = loop; }

		/**
		 * @brief Mutes or unmutes the audio.
		 * @param mute True to mute, false to unmute.
		 */
		void setMute(bool mute) { m_mute = mute; }

		/**
		 * @brief Enables or disables 3D spatialization.
		 * @param threeD True for 3D sound, false for 2D.
		 */
		void setIs3D(bool threeD) { m_is3D = threeD; }

		/**
		 * @brief Sets the minimum distance for 3D sound.
		 * @param minDistance Minimum attenuation distance.
		 */
		void setMinDistance(float minDistance) { m_minDistance = minDistance; }

		/**
		 * @brief Sets the maximum distance for 3D sound.
		 * @param maxDistance Maximum attenuation distance.
		 */
		void setMaxDistance(float maxDistance) { m_maxDistance = maxDistance; }

	private:
		AssetId m_audioHandle;	///< Audio handle for audio resource.
		AudioType m_type;		///< Type of audio (SFX or BGM)
		PlayState m_playState;	///< Current playback state
		float m_volume;			///< Volume level (0.0 to 1.0)
		float m_pitch;			///< Pitch level (default 1.0)
		bool m_loop;			///< Whether the audio should loop
		bool m_mute;			///< Whether the audio is muted
		bool m_is3D;			///< Whether the audio is 3D or not
		float m_minDistance;	///< Minimum distance for 3D audio attenuation
		float m_maxDistance;	///< Maximum distance for 3D audio attenuation
	};
} // namespace gam300

#endif // __AUDIOCOMPONENT_H__