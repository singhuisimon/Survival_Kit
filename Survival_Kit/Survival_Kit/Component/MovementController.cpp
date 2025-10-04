/******************************************************************************/
/*!
\file       MovementController.cpp
\author     Simon Chan
\date       Oct 04 2025
\brief      Implementation of scriptable movement controller component
/******************************************************************************/

#include "../Component/MovementController.h"
#include "../Manager/LogManager.h"

namespace gam300
{
    MovementController::MovementController(
        MovementMode mode,
        float moveForce,
        float kinematicSpeed
    )
        : m_mode(mode)
        , m_moveForce(moveForce)
        , m_kinematicSpeed(kinematicSpeed)
        , m_rotationTorque(20.0f)
        , m_desiredDirection(0.0f, 0.0f, 0.0f)
        , m_desiredRotation(0.0f, 0.0f, 0.0f)
    {
    }

    void MovementController::init(EntityID entity_id)
    {
        m_owner_id = entity_id;
        LM.writeLog("MovementController::init() - Initialized for entity %d (mode: %d)",
            entity_id, static_cast<int>(m_mode));
    }

    void MovementController::update(float dt)
    {
        (void)dt; // Component update handled by system
    }
}