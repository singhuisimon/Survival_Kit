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
#include <string>

namespace gam300 {
	enum class AudioType {
		SFX,
		BGM
	};

	enum class PlayState {
		PLAY,
		PAUSE,
		STOP
	};

	class AudioComponent : public Component {
	public:
		AudioComponent(const std::string& guid = "",
			//int64_t audioID = -1,
			AudioType type = AudioType::SFX,
			float volume = 1.0f,
			float pitch = 1.0f,
			bool loop = false,
			PlayState playstate = PlayState::STOP,
			bool is3D = true,
			Vector3D position = Vector3D());

		void init(EntityID entity_id) override;
		void update(float dt) override;

		// Getters
		const std::string& getGUID() const { return m_guid; }
		//int64_t getAudioID() const { return m_audioID; }
		AudioType getType() const { return m_type; }
		float getVolume() const { return m_volume; }
		float getPitch() const { return m_pitch; }
		bool isLooping() const { return m_loop; }
		PlayState getPlayState() const { return m_playState; }
		bool is3D() const { return m_is3D; }
		Vector3D getPosition() const { return m_position; }

		// Setters
		void setGUID(const std::string& guid) { m_guid = guid; }
		//void setAudioID(int64_t audioID) { m_audioID = audioID; }
		void setType(AudioType type) { m_type = type; }
		void setVolume(float volume) { m_volume = volume; }
		void setPitch(float pitch) { m_pitch = pitch; }
		void setLooping(bool loop) { m_loop = loop; }
		void setPlayState(PlayState state) { m_playState = state; }
		void setIs3D(bool threeD) { m_is3D = threeD; }
		void setPosition(const Vector3D& pos) { m_position = pos; }

	private:
		std::string m_guid;		///< Unique identifier for the audio resource
		//int64_t m_audioID;	///< Audio engine specific ID (e.g., FMOD sound ID)
		AudioType m_type;		///< Type of audio (SFX or BGM)
		float m_volume;			///< Volume level (0.0 to 1.0)
		float m_pitch;			///< Pitch level (default 1.0)
		bool m_loop;			///< Whether the audio should loop
		PlayState m_playState;	///< Current playback state
		bool m_is3D;			///< Whether the audio is 3D or not
		Vector3D m_position;	///< Position in 3D space (if 3D audio)
	};
} // namespace gam300

#endif // __AUDIOCOMPONENT_H__