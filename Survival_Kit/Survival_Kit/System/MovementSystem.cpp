#include "../System/MovementSystem.h"
#include "../Manager/ComponentManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/InputManager.h"

namespace gam300 {

	MovementSystem::MovementSystem() : ComponentSystem<Transform3D, RigidBody>("MovementSystem") {
		set_priority(100);
	}

	bool MovementSystem::init(SystemManager& system_manager) {

		//Find a way to register the system
		//system_manager.register_system("MovementSystem");

		LM.writeLog("MovementSystem::init() - Movement System Initialized");
		return true;
	}

	void MovementSystem::update(float dt) {

		(void)dt;
		m_dt = dt;
		for (EntityID entity_id : m_entities) {
			process_entity(entity_id);
		}
	}

	void MovementSystem::shutdown() {
		LM.writeLog("MovementSystem::shutdown() - MovementSystem shut down");
	}

	void MovementSystem::process_entity(EntityID entity_id) {
		
		auto transform = CM.get_component<Transform3D>(entity_id);
		auto rigidBody = CM.get_component<RigidBody>(entity_id);
		//float dt = 0.0f;
		if (!transform || !rigidBody) {
			return;
		}
		switch (rigidBody->getRigidBodyType())
		{
		case BodyType::STATIC:
			break;
		case BodyType::KINEMATIC:
			
			// transform->setPosition(transform->getPosition() + rigidBody->getLinearVelocity() * m_dt); // use it after update rigidBody component
			// use this system to test input for now

			// Move left
			//if (IM.isKeyPressed(GLFW_KEY_A))  { 
			//	//std::cout << IM.getMouseDeltaX() << std::endl;
			//	transform->setPosition(transform->getPosition() + Vector3D(-2.0f, 0.0f, 0.0f) * m_dt);
			//}
			//if (IM.isKeyPressed(GLFW_KEY_D))
			//{
			//	//std::cout << "is this work for input?" << "\n";
			//	transform->setPosition(transform->getPosition() + Vector3D(2.0f, 0.0f, 0.0f) * m_dt);
			//}
			//if (IM.isKeyPressed(GLFW_KEY_W))
			//{
			//	transform->setPosition(transform->getPosition() + Vector3D(0.0f, 2.0f, 0.0f) * m_dt);
			//}
			//if (IM.isKeyPressed(GLFW_KEY_S))
			//{
			//	transform->setPosition(transform->getPosition() + Vector3D(0.0f, -2.0f, 0.0f) * m_dt);
			//}
			handleKinematic(transform, rigidBody);
			break;
		case BodyType::DYNAMIC:
			//rigidBody->applyForce(Vector3D(5.0f, 0.0f, 0.0f)); // for now testing
			//transform->setPosition(transform->getPosition() + Vector3D(2.0f, 0.0f, 0.0f) * m_dt); // for testing
			//transform->setPosition(transform->getPosition() + rigidBody->getLinearVelocitys() * m_dt);
			handleDynamic(transform, rigidBody);

			break;
		}
		
		
		//std::cout << "Position x of the entity: " << entity_id <<  "is " << transform->getPosition() << "\n";
	}
	void MovementSystem::handleDynamic(Transform3D* transform, RigidBody* rigidBody)
	{
		Vector3D inputDir = getMovementInput();
		
		if (inputDir.magnitude() > 0.1f) {
			//inputDir = inputDir.normalize();


			float forceStrength = 50.0f;
			Vector3D force = inputDir * forceStrength;
			rigidBody->applyForce(force);

			// Optional: Add rotation to face movement direction (like before)
			float targetYaw = atan2f(inputDir.x, inputDir.y) * (180.0f / 3.14159f);

			Vector3D currentRot = transform->getRotation();
			float currentYaw = currentRot.y;

			float rotationSpeed = 5.0f;
			float yawDiff = targetYaw - currentYaw;

			if (yawDiff > 180.0f) yawDiff -= 360.0f;
			if (yawDiff < -180.0f) yawDiff += 360.0f;

			float newYaw = currentYaw + (yawDiff * rotationSpeed * m_dt);
			transform->setRotation(Vector3D(0.0f, newYaw, 0.0f));
		}


	}

	void MovementSystem::handleKinematic(Transform3D* transform, RigidBody* rigidBody)
	{
		Vector3D inputDir = getMovementInput();

		inputDir = inputDir.normalize();

		float movementSpeed = 5.0f;

		Vector3D movement = inputDir * movementSpeed * m_dt;

		transform->setPosition(transform->getPosition() + movement);
		rigidBody->setLinearVelocity(inputDir + movement);
	}

	Vector3D MovementSystem::getMovementInput()
	{
		Vector3D movementInput;

		if (IM.isKeyPressed(GLFW_KEY_A)) // left
		{
			movementInput.x -= 2.0f;
		}
		if (IM.isKeyPressed(GLFW_KEY_D)) // right
		{
			movementInput.x += 2.0f;
		}
		if (IM.isKeyPressed(GLFW_KEY_W)) // top
		{
			movementInput.y += 2.0f;

		}
		if (IM.isKeyPressed(GLFW_KEY_S)) // down
		{
			movementInput.y -= 2.0f;
		}
		return movementInput;
	}
}