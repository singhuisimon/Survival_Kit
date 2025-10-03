#pragma once
#ifndef __TEXTURE_COMPONENT_H__
#define __TEXTURE_COMPONENT_H__

#include "../Component/Component.h"
#include "../Graphics/SharedGraphics.h"

namespace gam300 {

	class TextureComponent : public Component {
	public:
		TextureComponent(const std::string& guid = "", u16 texture_handle = 0);

		void init(EntityID entity_id) override;
		void update(float dt) override;

		inline const std::string& getGUID() const { return m_guid; }
		inline const uint16_t& getTextureHandle() const { return m_texture_handle; }

		inline void setGUID(const std::string& guid) { m_guid = guid; }
		inline void setTextureHandle(u16& tex_hdl) { m_texture_handle = tex_hdl; }

	private:

		std::string m_guid;
		u16         m_texture_handle;
	};

}

#endif // !__TEXTURE_COMPONENT_H__
