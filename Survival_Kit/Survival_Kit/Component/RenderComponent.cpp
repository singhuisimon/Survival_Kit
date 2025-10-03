#include "RenderComponent.h"
#include "../Manager/LogManager.h"

namespace gam300 {

	void RenderComponent::init(EntityID entity_id) {
		m_owner_id = entity_id;
		LM.writeLog("RenderComponent::init() - RenderComponent component initialized for entity %d", entity_id);
	}

	void RenderComponent::update(float dt) {
		(void)dt;
		return;
	}

}