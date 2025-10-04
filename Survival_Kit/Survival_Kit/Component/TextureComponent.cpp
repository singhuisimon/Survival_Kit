#include "../Component/TextureComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {
	
	TextureComponent::TextureComponent(const std::string& guid, u16 texture_handle) : m_guid(guid), m_texture_handle(texture_handle) { }

	void TextureComponent::init(EntityID entity_id) {
		m_owner_id = entity_id;
		LM.writeLog("TextureComponent::init() - TextureComponent initialized for entity %d", entity_id);
	}

	void TextureComponent::update(float dt) {
		(void)dt;
	}

}