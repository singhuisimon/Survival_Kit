using ScriptAPI;
using System;

public class TestScript : Script
{
    // Movement parameters
    private float moveForce = 40.0f;
    private float kinematicSpeed = 5.0f;
    private float rotationTorque = 20.0f;
    private bool enableRotation = true;
    private bool circularMovement = true;

    private float time = 0.0f;
    private bool initialized = false;

    public override void Update()
    {
        // Get the MovementController component
        MovementControllerComponent movement = GetMovementController();
        
        // Initialize once
        if (!initialized)
        {
            Console.WriteLine("=== MovementTest Script Started ===");
            
            // Apply initial settings
            movement.SetMoveForce(moveForce);
            movement.SetKinematicSpeed(kinematicSpeed);
            movement.SetRotationTorque(rotationTorque);
            
            Console.WriteLine($"MoveForce: {movement.GetMoveForce()}");
            Console.WriteLine($"KinematicSpeed: {movement.GetKinematicSpeed()}");
            Console.WriteLine($"RotationTorque: {movement.GetRotationTorque()}");
            
            Console.WriteLine("===================================\n");
            initialized = true;
        }
        
        time += 0.016f; // Approximate delta time
        
        Vector3 direction;
        
        if (circularMovement)
        {
            // Move in a circle pattern
            float x = (float)Math.Cos(time);
            float z = (float)Math.Sin(time);
            direction = new Vector3(x, 0.0f, z);
        }
        else
        {
            // Move straight forward
            direction = new Vector3(0.0f, 0.0f, 1.0f);
        }
        
        movement.SetDesiredDirection(direction);
        
        // Apply rotation if enabled
        if (enableRotation)
        {
            Vector3 rotation = new Vector3(0.0f, 1.0f, 0.0f);
            movement.SetDesiredRotation(rotation);
        }
        else
        {
            movement.SetDesiredRotation(new Vector3(0.0f, 0.0f, 0.0f));
        }
        
        // Log every 2 seconds
        if ((int)(time * 60) % 120 == 0)
        {
            Vector3 dir = movement.GetDesiredDirection();
            Vector3 rot = movement.GetDesiredRotation();
            
            Console.WriteLine($"[{time:F2}s] Dir: ({dir.X:F2}, {dir.Y:F2}, {dir.Z:F2}) | Rot: ({rot.X:F2}, {rot.Y:F2}, {rot.Z:F2})");
        }
    }
}