#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	// Export these functions from the EXE
	__declspec(dllexport) void SetMovementDirection(int entityId, float x, float y, float z);
	__declspec(dllexport) void GetMovementDirection(int entityId, float* x, float* y, float* z);
	__declspec(dllexport) void SetMovementRotation(int entityId, float x, float y, float z);
	__declspec(dllexport) void GetMovementRotation(int entityId, float* x, float* y, float* z);
	__declspec(dllexport) void SetMovementMoveForce(int entityId, float force);
	__declspec(dllexport) float GetMovementMoveForce(int entityId);
	__declspec(dllexport) void SetMovementKinematicSpeed(int entityId, float speed);
	__declspec(dllexport) float GetMovementKinematicSpeed(int entityId);
	__declspec(dllexport) void SetMovementRotationTorque(int entityId, float torque);
	__declspec(dllexport) float GetMovementRotationTorque(int entityId);

#ifdef __cplusplus
}
#endif