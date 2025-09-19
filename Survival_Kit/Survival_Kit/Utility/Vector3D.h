#pragma once
/**
 * @file Vector3D.h
 * @brief Declaration of the Vector3D class for the game engine.
 * @details Provides 3D vector mathematics functionality for positions, velocities, and directions.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__

#include <cmath>
#include <iostream>

#include "Vector2D.h" // Include for Vector2D interoperability

namespace gam300 {

    class Vector3D {
    public:
        // Components
        float x;
        float y;
        float z;

        // Operator to perform a cast to a glm vector
        explicit operator glm::vec3() const noexcept { return glm::vec3(x, y, z); }

        // Constructors
        Vector3D();                              // Default constructor (0,0,0)
        Vector3D(float x, float y, float z);     // Constructor with components
        Vector3D(const Vector3D& other);         // Copy constructor
        Vector3D(const Vector2D& vec2, float z); // Construct from Vector2D + z component

        // Conversion to Vector2D (drops z component)
        Vector2D toVector2D() const;

        // Assignment
        Vector3D& operator=(const Vector3D& other);

        // Basic arithmetic operations
        Vector3D operator+(const Vector3D& other) const;
        Vector3D operator-(const Vector3D& other) const;
        Vector3D operator*(float scalar) const;
        Vector3D operator/(float scalar) const;
        Vector3D& operator+=(const Vector3D& other);
        Vector3D& operator-=(const Vector3D& other);
        Vector3D& operator*=(float scalar);
        Vector3D& operator/=(float scalar);

        // Negation
        Vector3D operator-() const;

        // Comparison
        bool operator==(const Vector3D& other) const;
        bool operator!=(const Vector3D& other) const;

        // Vector operations
        float magnitude() const;               // Length of the vector
        float magnitudeSquared() const;        // Squared length (faster when only comparing)
        Vector3D normalize() const;            // Returns a normalized (unit) vector
        void normalizeInPlace();               // Normalizes this vector in place

        // Static vector operations
        static float dot(const Vector3D& a, const Vector3D& b);           // Dot product
        static Vector3D cross(const Vector3D& a, const Vector3D& b);      // Cross product
        static float distance(const Vector3D& a, const Vector3D& b);      // Distance between two vectors
        static float distanceSquared(const Vector3D& a, const Vector3D& b); // Squared distance
        static Vector3D lerp(const Vector3D& a, const Vector3D& b, float t); // Linear interpolation
        static Vector3D project(const Vector3D& v, const Vector3D& onto); // Project v onto 'onto'
        static Vector3D reflect(const Vector3D& v, const Vector3D& normal); // Reflect v about normal

        // Common vectors
        static const Vector3D ZERO;
        static const Vector3D ONE;
        static const Vector3D UNIT_X;
        static const Vector3D UNIT_Y;
        static const Vector3D UNIT_Z;
        static const Vector3D UP;      // Positive Y
        static const Vector3D DOWN;    // Negative Y
        static const Vector3D RIGHT;   // Positive X
        static const Vector3D LEFT;    // Negative X
        static const Vector3D FORWARD; // Negative Z (OpenGL convention)
        static const Vector3D BACK;    // Positive Z
    };

    // Stream operators for easy printing
    std::ostream& operator<<(std::ostream& os, const Vector3D& vec);

    // Global scalar multiplication
    Vector3D operator*(float scalar, const Vector3D& vec);

} // end of namespace gam300

#endif // __VECTOR3D_H__